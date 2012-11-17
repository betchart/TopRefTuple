import webpage_SCBooks,sqlite3,os,sys,getpass,time
db_version = 1
icfpath = "/afs/cern.ch/user/b/bbetchar/www/web/ICF_Database"

databases = {"ICF": {"db_location": icfpath,
                     "db_file": 'sqlite%d.db' % db_version,
                     "db_lock": 'sqlite%d.lock' % db_version,
                     "webpath": '%s/status%d.html'%(icfpath,db_version)},

             "TEST": {"db_location": "./",
                      "db_file": 'sqlite.db',
                      "db_lock": 'sqlite.lock',
                      "webpath": "./status.html"}
             }

def create_db(path) :
    conn = sqlite3.connect(path)
    conn.execute('''CREATE TABLE dset( dataset TEXT NOT NULL,
                                       isData  BOOLEAN DEFAULT 0 NOT NULL
                                       )''')
    conn.execute('''CREATE TABLE job( recipe    TEXT NOT NULL,
                                      dsetid    INT NOT NULL,
                                      globalTag TEXT NOT NULL,
                                      nonDefault TEXT,
                                      jsonls TEXT,
                                      state  TEXT DEFAULT \'Unclaimed\',
                                      user   TEXT,
                                      node   TEXT,
                                      path   TEXT,
                                      rpath  TEXT,
                                      dash   TEXT
                                      )''')
    conn.commit()
    print path, 'successfully created with tables.'

def valid_keys(connection) :
    valid_dsetid = [row[0] for row in connection.execute('select rowid from dset')]
    for row in connection.execute('select dsetid from job') :
        if row['dsetid'] not in valid_dsetid: return False
    return True

def duplicates(connection, table) :
    if len(['' for row in connection.execute('select * from '+table)]) >  len(['' for row in connection.execute('select distinct * from '+table)]) :
        print table,'contains duplicate rows'
        return True
    return False

def valid_json(connection) :
    rows = connection.execute('select jsonls from job').fetchall()
    for row in rows:
        if not row[0] : continue
        try:
            jsonls = eval(row[0])
            for run in jsonls.keys() :
                assert(eval(run)>0)
                for pair in jsonls[run] :
                    assert(pair[0]<=pair[1])
        except:
            print 'A jsonls field is improperly formatted'
            return False
    return True

def valid_db(connection) :
    if not valid_keys(connection) :
        print 'Some jobs have invalid dsetid'
        return False
    if not valid_json(connection) :    return False
    if duplicates(connection,'dset') : return False
    if duplicates(connection,'job') :  return False
    return True


def create_db_lock(path) :
    lockfile = open(path, "w")
    print>>lockfile, getpass.getuser()
    lockfile.close()

class lockedDB:
    def __init__(self):
        self.conn = 0
        self.locktime = 0
        for key in databases : print key
        self.name = raw_input('Database: ')
        self.config = databases[self.name]
        self.db_location = self.config["db_location"]
        self.db_lock = self.config["db_lock"]
        self.db_file = self.config["db_file"]
        self.webpath = self.config["webpath"]
        
    def connect(self):
        if not os.path.exists(self.db_location) :
            print 'Cannot find database:', self.db_location, 'is not a valid directory'
            sys.exit()
            
        lockpath = self.db_location+'/'+self.db_lock
        if os.path.exists(lockpath) :
            user = open(lockpath).readline()[:-1]
            print "\nDatabase locked by %s %0.1f minutes ago." % (user, (time.time()-os.path.getmtime(lockpath))/60)
            input = raw_input("Break lock? [n] ")
            if not input or input[0] != 'y' :
                sys.exit()
        create_db_lock(lockpath)
        self.locktime = os.path.getmtime(lockpath)

        path = self.db_location+'/'+self.db_file
        if not os.path.exists(path) :
            input = raw_input('Cannot find database in '+self.db_location+'.  Create it now? [y/n] ')
            if input and input[0] in ['y','Y']:
                create_db(path)
            else :
                print 'No action taken'
                self.disconnect()
                sys.exit()
        self.conn = sqlite3.connect(path)
        self.conn.row_factory = sqlite3.Row

    def secure(self):
        lockpath = self.db_location+'/'+self.db_lock
        if os.path.exists(lockpath) :
            user = open(lockpath).readline()[:-1]
            locktime = os.path.getmtime(lockpath)
            if locktime == self.locktime and user == getpass.getuser() : return True
        return False

    def save(self):
        secure = self.secure()
        valid = valid_db(self.conn)
        if not secure :
            print 'Database not locked : not saved'
        else :
            if not valid :
                print 'Invalid database : not saved'
            else :
                self.conn.commit()
                print 'Saved'
                webpage_SCBooks.write_webpage(self.conn,self.webpath,self.name)
                self.git()

    def disconnect(self):
        input = raw_input("Save before disconnecting? [y]")
        if input=='' or input[0]=='y' or input[0]=='Y' :
            self.save()
        else :
            print 'Not saved'
        if self.secure() : os.remove(self.db_location+'/'+self.db_lock)
        self.conn=0
    
    def execute(self,one): return self.conn.execute(one)
    def cursor(self): return self.conn.cursor()

    def git(self) :
        cmd = '\n'.join(['#!/usr/bin/env bash',
                         'which git &> /dev/null && (',
                         'cd %(path)s',
                         '(ls .git &> /dev/null && true || git init)',
                         'git add %(sqlite)s %(html)s',
                         'git commit -m "%(script)s" &> /dev/null',
                         'true',
                         ') || echo "git not found"',
                         ])%{'path': self.db_location,
                             'sqlite':self.db_file,
                             'html':self.webpath,
                             'script':' '.join(sys.argv)}
        print
        os.system(cmd)
