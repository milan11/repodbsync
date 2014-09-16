#  RepoDbSync

Helps to maintain \(synchronize\):

* local development databases
* with one main database storage in a shared source code repository \(in form of database creation SQL scripts\)

Each run does these 2 steps:

* applying scripts from the repository to your local database
* helping with creating scripts in the repository which reflect changes in your local database

Applying of these 2 steps \(generally in more iterations\) makes the local database synchronized with the repository\.

__Version:__ 0\.4

__License:__ Public domain\. Use at your own risk\. There is absolutely no warranty of any kind\.

##  Build

* __implementation language:__ C\+\+11
* __platform:__ independent
* __latest source code:__ [https://github\.com/milan11/repodbsync](https://github\.com/milan11/repodbsync)
* __source code distribution:__ as CMake project – uses \(and includes\) cotire CMake module: [https://github\.com/sakra/cotire](https://github\.com/sakra/cotire)
* __build dependencies:__
 * boost
* __build steps:__
 * git clone git@github\.com:milan11/repodbsync\.git
 * cd repodbsync
 * cmake
 * make
* __possible CMake arguments:__
 * __debug build:__ \-DCMAKE\_BUILD\_TYPE=Debug
 * __force using gcc compiler:__ \-DCMAKE\_CXX\_COMPILER=g\+\+
 * __force using clang compiler:__ \-DCMAKE\_CXX\_COMPILER=clang\+\+ 
 * __build executable running tests instead of the normal executable:__ \-DTESTMODE=1

##  Supported Environment

* __databases:__ MySQL, PostgreSQL, SQLite
* __OS:__ Linux tested only \(but should be multiplatform\)
* __needs these database tools \(must be in path\):__
 * __for MySQL:__ mysql, mysqldump
 * __for PostgreSQL:__ psql, pgdump
 * __for SQLite:__ sqlite3
* __needs these libraries \(is dynamically linked with\):__
 * boost \(system, filesystem\)

##  Short Usage Description

Just run it in some source code repository directory where you want the sql scripts and some configuration to be placed\. It will guide you through the configuration and synchronization process\. The executable does not process any arguments\.

After following each instruction printed by RepoDbSync, you will have to run it again\. It will continue in the process of setup and synchronization\. After everything is correctly set up and after the databases are synchronized, it should print nothing and quit immediately\.

Please note that the following descriptions refer to the state when everyone already has their development database in some state\. However, the instructions can be applied to the cases of creating new development databases, too\. Just imagine that you already have a development database, but it is empty\.

### A\. I have a local development database and want to put its state to a shared repository \(e\.g\. a GIT repository\) to share/synchronize the database with others \(who maybe have their own development databases already\)\.

#### 1\. Create an empty directory in your repository working copy\.

* this directory will serve as a main source for the database state
* if you have more development databases \(e\.g\. one for a core system and one for a web interface\), you have to deal with each database separately, thus creating such directory in your repository for each database

#### 2\. Set up \(fill the configuration file\)\.

* run RepoDbSync in that empty directory for the first time – it should generate an empty settings file
* fill the settings file; see: A Settings file

#### 3\. Set up database versioning\.

* run RepoDbSync with settings filled in
* see: B Database versioning

#### 4\. Add database scripts to the repository\.

* see: D Adding database scripts to the repository

#### 5\. Commit\.

* commit the directory dedicated to the database to the shared repository
* the directory now contains scripts \(in the "scripts" directory\) which can be used to create new database in the desired state or to synchronize an existing database

### B\. Someone has already put their repository state to our shared repository\. I want to synchronize my own development database to reflect the state in the repository\.

#### 1\. Checkout the repository and find the database directory\.

* the database directory should contain a directory called "scripts" and maybe \.gitignore, ignore\_data\.txt and ignore\_tables\.txt files

#### 2\. Set up \(fill the configuration file\)\.

* run RepoDbSync in that directory for the first time – it should generate an empty settings file
* fill the settings file; see: A Settings file

#### 3\. Set up database versioning\.

* run RepoDbSync with settings filled in
* see: B Database versioning

#### 4\. Apply scripts to your local database\.

* see: C Applying scripts to local database

#### 5\. Add database scripts to the repository\.

* see: D Adding database scripts to the repository

#### 6\. Commit\.

* commit new \.sql scripts to the shared repository

##  Manual

### A\. Settings file

* you have to set up two databases:
* the __"local"__ database is your development database
* the __"temp"__ database is a \(empty\) temporary database – it will be used by RepoDbSync for simulating a database reflecting the repository state and will be cleaned before each run \(so be careful to set it to an empty database; not to some database you are using for something\)

### B\. Database versioning

* if your local development database does not contain version information, RepoDbSync will ask you if it can add the version information – that is adding one table named VersionInfo to your database where the version info will be stored
* if the local database has higher version than the scripts in the repository can provide, RepoDbSync can repair this automatically – however, that should never happen and you should check e\.g\. if you have the right repository revisision checked out before allowing RepoDbSync to repair the version

### C\. Applying scripts to local database

* review the scripts you checked out from the repository
* RepoDbSync can apply scripts which have higher target version than current version of your local database
* RepoDbSync will ask you if it can apply each script automatically; but maybe you will have to make some changes in your local development database manually and let RepoDbSync increment the database version only
* in general, for each script, you can select one of these options:
 * __apply script__ – applies the script and increments database version to the target version of the script
 * __increment version only__ – if you have changed your local database manually to reflect the changes in this script or if you already had your database in the target state \(e\.g\. when you just created this new script to reflect your local database\)
 * __apply all scripts__ – automatically applies all scripts and increments database version to the target version of the last script
 * __increment version to the target version of the last script__ – increments the database version to the target version of the last script – if you know that you have applied all changes from the scripts in your database

### D\. Adding database scripts to the repository

* RepoDbSync will tell you what add to the repository so that the repository will exactly reflect your local development database
* note the contents of the "outs" directory after each run, there are some created scripts that you can use when creating the scripts in the repository:
 * __&lt;num&gt;\_delete\_&lt;table&gt;\.sql__ – for tables which are in the repository only
 * __&lt;num&gt;\_create\_&lt;table&gt;\.sql__ – for tables which are in the local database only
 * __&lt;table&gt;\_local\.sql__ and __&lt;table&gt;\_repository\.sql__ – for tables with different structure
 * __&lt;table&gt;\_local\_data\.sql__ and __&lt;table&gt;\_repository\_data\.sql__ – for tables with different data
 * __&lt;num&gt;\_routine\_delete\_&lt;routine&gt;\.sql__ – for routines which are in the repository only
 * __&lt;num&gt;\_routine\_create\_&lt;routine&gt;\.sql__ – for routines which are in the local database only
 * __&lt;routine&gt;\_local\_routine\.sql__ and __&lt;routine&gt;\_repository\_routine\.sql__ – for routines which are different
* the scripts must be named in the following way: version\_description\.sql
 * __version__ is the version which will the database have after applying this script \(a number must be aligned to six digits, first version is 000001, no version can be left out\)
 * __description__ is your own description \(e\.g\. create\_table\_user, fill\_users, delete\_table\_user\)
* one \.sql file in the repository has not to contain exactly one table or table data – you can even put whole database structure and data to one \.sql file \(this can be the case of the first file added to the repository\)
* remember that once committed, the scripts should not be changed \(because somebody else should already use the scripts to change their local database\) – you can only add a following script reverting the change in the database \(e\.g\. DROP TABLE reverting CREATE TABLE\)
* note that after adding scripts to the repository and running RepoDbSync again, you will have a version mismatch \(the repository version will be higher that your local development database version\)
 * RepoDbSync will ask you, if you want to apply the repository scripts to upgrade your local development database to the repository state
 * because you already have the desired state provided by the scripts in your local development database \(because you created the scripts from the state of the database\), you can ignore this mismatch and answer with "nall" \(this only sets the version of the local development database to the version provided by the scripts\)
* of course, you will have some specific tables or data in your local development database, which you do not want to share \(which do not form the base database state\), this can be e\.g\. data about users which you have added while testing etc\.
 * list tables you want to ignore in the __ignore\_tabes\.txt__ file \(one table name on one line\)
 * list tables which data have to be ignored in the __ignore\_data\.txt__ file \(one table name on one line\)
 * additionally, you can list conditions for ignored data \(e\.g\. "user id = 1 OR id &gt; 4"\) will ignore records in the table user which have ID 1 or ID greater than 4
 * list routines \(stored functions or procedures\) to ignore in the __ignore\_routines\.txt__ file
 * the \*local\.txt files are there for the ignore lists which you do not want to share in the repository

##  TODO

* provide downloadable binaries
* test on Windows
* real\-world examples \(e\.g\. handling a standard Drupal database\)

