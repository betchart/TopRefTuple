#!/usr/bin/env python
import configuration_SCBooks as conf,sqlite3,readline,sys

def menu(actions) :
    s = '\n\t'
    for key in actions:
        s += '['+key+']'+actions[key][0]+'\n\t'
    s += '--> '
    actions.get(raw_input(s),('', lambda: menu(actions)))[1]()
    menu(actions)

class planner:
    def __init__(self):
        self.db = conf.lockedDB()
        self.db.connect()
        
        self.mainM = {
            "q": ("uit", self.quit),
            "s": ("ave", self.db.save),
            "d": ("atasets", lambda : menu(self.dsetM)),
            "j": ("obs", lambda : menu(self.jobM))
            }
        self.dsetM = {
            "m": ("ain", lambda : menu(self.mainM)),
            "l": ("ist datasets", lambda : self.list_entries('select rowid,dataset,isData from dset')),
            "n": ("ew dataset", lambda : self.new_entry('dset')),
            "d": ("elete", lambda : self.delete_row('dset'))
            }
        self.jobM = {
            "m": ("ain", lambda : menu(self.mainM)),
            "l": ("ist jobs", lambda : self.list_entries('select rowid,recipe,dsetid,globalTag,jsonls from job')),
            "L": ("ist jobs (resolve keys)",
                  lambda : self.list_entries('''select job.rowid,job.recipe,job.globalTag,job.nonDefault,dset.dataset
                                                       from job join dset on dset.rowid=job.dsetid''')),
            "d": ("elete", lambda : self.delete_row('job')),
            "n": ("ew job", lambda : self.new_entry('job','dsetid,recipe,globalTag,nonDefault,jsonls'))
            }

    def run(self) :
        menu(self.mainM)
        
    def quit(self) :
        self.db.disconnect()
        sys.exit()        

    def list_entries(self,select) :
        rows = self.db.execute(select).fetchall()
        if len(rows) :   print rows[0].keys()
        for row in rows :  print ('\t'.join([str(item) for item in row]))[:170]

    def new_entry(self,table,cols='*') :
        c = self.db.cursor()
        c.execute("select %s from %s"%(cols,table))
        pars = dict((item[0], raw_input("%s: "%item[0])) for item in c.description)

        # Sanitize
        if 'jsonls' in pars and pars['jsonls'][-4:]=='.txt' :
            pars['jsonls'] = open(pars['jsonls']).readline()
        if 'isData' in pars :
            pars['isData'] = pars['isData'] in [True,'True',1,'1','y','Y','yes','Yes']
            if pars['isData'] :
                if 'nonDefault' not in pars : pars['nonDefault'] = ''
                #if not pars['nonDefault'].count('jetCorrections') :
                #    pars['nonDefault'] += " jetCorrections=L2Relative,L3Absolute,L2L3Residual"
        pars = dict(filter(lambda i: i[1]!='',pars.iteritems()))
        print '-'*10
        for key,val in pars.iteritems() : print key.ljust(10), " : ", val
        if raw_input("Enter values into table %s? [n] : "%table) != 'y' :  print "Did not enter values"
        else : c.execute("insert into %s (%s) values (%s)" % (table, ','.join(pars.keys()), ','.join('?'*len(pars))), tuple(pars.values()))
        c.close()

    def delete_row(self,table) :
        rowid = raw_input("delete rowid: ")
        if (not rowid) and rowid!=0 : return
        self.db.execute("delete from "+table+" where rowid="+rowid)
        if table!='job' :
            self.db.execute("delete from job where "+table+"id="+rowid)

planner().run()
