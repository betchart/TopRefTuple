#!/usr/bin/env python

import configuration_SCBooks as conf,sys,os,readline,getpass,string,fileinput,socket,datetime,re

def remove_multislash(inString) :  return re.sub(r'//*','/',inString)

def print_and_execute(c) : print c; os.system(c)

def get_jobrow(db) :
    rows = db.execute('''select job.rowid,recipe,globalTag,dataset
                       from job join dset on dset.rowid=job.dsetid
                       where state="Unclaimed"''').fetchall()
    for i,row in enumerate(rows):
        print '\n'.join(['', str(row.keys()),
                         '\t'.join([str(item) for item in row])[0:120] + "..."][2 if i%10 else None:])
    jobnumber = raw_input("\n\n\tWhich job?  ")
    if not jobnumber in [str(row['rowid']) for row in rows] :
        print jobnumber+' is not an `Unclaimed` job'
        db.disconnect()
        sys.exit()
    
    return db.execute('''select job.rowid,jsonls,recipe,dataset,isData,globalTag,nonDefault
                         from job join dset on dset.rowid=job.dsetid
                         where state="Unclaimed" AND job.rowid='''+jobnumber).fetchone()

def setup_cmssw(job,path) :
    print_and_execute('\n'.join(["#!/usr/bin/env bash",
                                 "mkdir -p %s"%path,
                                 "cd %s"%path,
                                 "wget --no-check-certificate https://raw.github.com/betchart/TopRefTuple/%s/scripts/recipe.sh"%job['recipe'],
                                 "cat recipe.sh",
                                 ". recipe.sh",
                                 "scram b -j 8",
                                 'echo "\n\n\nCheck that everything built:"',
                                 'scram b']))

def setup_output_dirs(option) :
    dirs = option["FULL_RPATH"].strip('/').split('/')
    print_and_execute('\n'.join(['#!/usr/bin/env bash',
                                 'rfmkdir -p %(FULL_RPATH)s'%option,
                                 'rfchmod 775 %(FULL_RPATH)s'%option]+
                                ['rfchmod 755 /'+'/'.join(dirs[:-i]) for i in range(1,len(dirs)-dirs.index(option["USER"]))])
                      )

def setup_crab(job,option) :

    SITE = { "CASTOR" : {"SE":"srm-cms.cern.ch",
                         "FULL_RPATH":"/castor/cern.ch/user/%(INITIAL)s/%(USER)s/%(RPATH)s",
                         "USER_REMOTE": "user/%(INITIAL)s/%(USER)s/%(RPATH)s",
                         "SCHEDULER":"glite",
                         "DBS_URL": option["DBS_URL"],
                         "EXTRA": "\n[USER]\nstorage_path=/srm/managerv2?SFN=/castor/cern.ch"
                         },
             "CAF"    : {"SE":"T2_CH_CAF",
                         "FULL_RPATH":"/castor/cern.ch/cms/store/caf/user/%(USER)s/%(RPATH)s" % option,
                         "USER_REMOTE":"%(RPATH)s",
                         "SCHEDULER":"caf",
                         "DBS_URL": (option["DBS_URL"] if option["DBS_URL"] \
                                     else None if job['dataset'].find('ExpressPhysics')<0 \
                                     else "http://cmsdbsprod.cern.ch/cms_dbs_caf_analysis_01/servlet/DBSServlet"),
                         "EXTRA":"\n[CAF]\nqueue=cmscaf1nd\n"
                         },
             "LONDON" : {"SE":"T2_UK_London_IC",
                         "FULL_RPATH":"/pnfs/hep.ph.ic.ac.uk/data/cms/store/user/%(USER)s/%(RPATH)s" % option,
                         "USER_REMOTE":"%(RPATH)s",
                         "SCHEDULER":"remoteGlidein",
                         "DBS_URL": option["DBS_URL"],
                         "EXTRA": ""},
             "OSETHACK" : {"SE":"T2_UK_London_IC",
                           "FULL_RPATH":"/pnfs/hep.ph.ic.ac.uk/data/cms/store/user/%(USER)s/%(RPATH)s" % option,
                           "USER_REMOTE":"%(RPATH)s",
                           "SCHEDULER":"remoteGlidein",
                           "DBS_URL": "http://cmsdbsprod.cern.ch/cms_dbs_ph_analysis_02/servlet/DBSServlet",
                           "EXTRA": "[CRAB]\nserver_name=slc5ucsd\n[GRID]\nse_white_list=T1,T2\nce_white_list=T1,T2\n"},
              "FNAL" : {"SE":"cmseos.fnal.gov",
              		"FULL_RPATH":"/eos/uscms/store/user/%(USER)s/%(RPATH)s" % option,
              		"USER_REMOTE":"/store/user/%(USER)s/%(RPATH)s",
              		"SCHEDULER":"remoteGlidein",
              		#"SCHEDULER":"glite",
                        "DBS_URL": option["DBS_URL"],
                        "EXTRA":""}
             }
             
    option["INITIAL"] = option["USER"][0]
    for key,val in SITE[option["SITE"]].items() :
        if val is None : continue
        option[key] = eval("'''"+val+"'''%option")

    option["EVENTS"] = '\n'.join(['lumis_per_job=150',
                                  'total_number_of_lumis=-1',
                                  'lumi_mask=%(PATH)s/jsonls.txt'%option if job['jsonls'] else ''
                                  ] if job['isData'] else
                                 ['total_number_of_events=-1',
                                  'events_per_job=100000'
                                  ]
                                 )
    
    option["DBS_URL"] = ("dbs_url="+option["DBS_URL"]) if option["DBS_URL"] else ""

    if option["SITE"] not in ["LONDON","OSETHACK","FNAL"]: setup_output_dirs(option)
    if job['jsonls'] :
        with open("%(PATH)s/jsonls.txt"%option,"w") as jsonfile:
            print>>jsonfile,str(job['jsonls']).replace("'",'"')

#    option["FNAL_SPECIFIC"] = "" if option['SITE'] != "FNAL" else '\n'.join(['check_user_remote_dir = 0',
#                                                                             'storage_path = /srm/managerv2?SFN=11'])
    with open("%(PATH)s/crab.cfg"%option,"w") as crabfile :
        print>>crabfile,'''
[CMSSW]
get_edm_output = 1
%(EVENTS)s
pset=topTuple_cfg.py
datasetpath=%(DATASET)s
%(DBS_URL)s

[USER]
copy_data=1
user_remote_dir=%(USER_REMOTE)s
storage_element=%(SE)s
%(FNAL_SPECIFIC)s
additional_input_files=*.txt,AutoDict*

[CRAB]
cfg=crab.cfg
scheduler=%(SCHEDULER)s
use_server=0
jobtype=cmssw

%(EXTRA)s
'''% option
    return


def setup_multi(job,path) :
    with open(path+"/multicrab.cfg","w") as mcrabfile :
        print>>mcrabfile, '\n'.join(['[MULTICRAB]',
                                     'cfg=crab.cfg'] +
                                    [ '\n'.join(['',
                                                 '[%s]'% ds.strip('/').replace(" ","_").replace("/","."),
                                                 'CMSSW.datasetpath=%s'%ds])
                                      for ds in job['DATASET'].split(',')])

def run_crab(job,path,MULTI,onlyTest) :
    print_and_execute('''
#!/usr/bin/env bash
    
source /afs/cern.ch/cms/LCG/LCG-2/UI/cms_ui_env.sh
cd %(path)s/CMSSW_*/src/
eval `scram runtime -sh`
source %(crab_setup)s
cd %(path)s
python %(path)s/CMSSW_*/src/TopQuarkAnalysis/TopRefTuple/test/config.py isData=%(isData)d globalTag=%(gt)s %(other)s
%(crab)s -create
%(crab)s %(doit)s
%(crab)s -status &> crab.status
'''%{ "path": path,
      "isData" : job['isData'],
      "gt" : job['globalTag'],
      "other" : job['nonDefault'] if job['nonDefault'] else '',
      "crab" : "multicrab" if MULTI else "crab",
      #"crab_setup" : "/afs/cern.ch/cms/ccs/wm/scripts/Crab/crab.sh",
      "crab_setup" : "/bin/true",
      "doit" : "" if onlyTest else "-submit"
      })
    return

def get_dashboard(path) :
    dash = []
    for line in fileinput.input(path+'/crab.status') :
        if line.find("Dashboard:")>0 :
            dash.append((line[line.find(":")+1:]).strip())
    return '"'+', '.join(dash)+'"'

def get_options(name) :
    option = {}
    timestamp = '_'.join(['%02d'% i for i in datetime.datetime.now().timetuple()[:6]])
    print "Choose output site:"
    for site in ["CASTOR","CAF","LONDON","OSETHACK", "FNAL"] : print '\t'+site
    option["SITE"] = raw_input("\t> ")
    option["USER"] = getpass.getuser()
    option["NODE"] = socket.gethostname()
    option["PATH"] = '/'.join(['','tmp',option['USER'],name,timestamp,''])
    option["RPATH"] = '/'.join(['',name,'automated',timestamp,''])
    option["JOBID"] = job['rowid']
    option['DATASET'] = job['dataset']
    option["MULTI"] = len(job['dataset'].split(','))>1
    option["DBS_URL"] = None

    print 'You have specified:'
    for key in ["SITE"]:  print "%s : %s"%( key, str(option[key]))
    return option

db = conf.lockedDB()
db.connect()
job = get_jobrow( db )
option = get_options(db.name)

doit = raw_input('\nDo it? [y/n/f]')
if doit in ['Y','y',1,'f','F'] :

    setup_cmssw( job, option["PATH"] )
    setup_crab( job, option)
    if option["MULTI"] : setup_multi( job, option['PATH'] )
    run_crab( job, option["PATH"], option['MULTI'], onlyTest = doit in ['f','F'])
    option["DASH"] = get_dashboard(option["PATH"])
    
    db.execute('''update job set
    state="Processing",
    user="%(USER)s",
    node="%(NODE)s",
    path="%(PATH)s",
    rpath="%(FULL_RPATH)s",
    dash=%(DASH)s where rowid=%(JOBID)d''' % option )

db.disconnect()
