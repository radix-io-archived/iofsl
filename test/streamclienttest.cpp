#include "test/streamclienttest.hh"


namespace po = boost::program_options;
iofwdclient::IOFWDClient * iofslSetup( char * address, std::string opt_config)
{
   // Needed to autoregister services
   registerIofwdFactoryClients ();
   registerIofwdutilFactoryClients (); 
   iofwd::service::ServiceManager & man = iofwd::service::ServiceManager::instance ();
   man.setParam ("config.configfile", opt_config);
   boost::shared_ptr<iofwd::Net> netservice (man.loadService<iofwd::Net>("net"));
   iofwdevent::SingleCompletion block;
   net::Net * net = netservice->getNet ();
   net::AddressPtr addr;
   net->lookup (address, &addr, block);
   block.wait ();
   iofwdclient::IOFWDClient * x  =  new iofwdclient::IOFWDClient (*(new iofwdclient::CommStream()),
                                                                  addr,
                                                                  (bool)true);
   return x;
}

int check_system (const char * cmd)
{
   if (system (cmd) == -1)
   {
      std::cerr << "Error calling system '" << cmd << "'!\n";
      exit (1);
   }
}

int linkTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  char * fname = new char[dir.length() + 50];
  char * slinkName = new char[dir.length() + 50];
  char * systemCommand = new char[500];  
  strcpy(fname, dir.c_str());
  strcat(fname, "linktest");
  strcpy(systemCommand, "mkdir ");
  strcat(systemCommand, fname);
  check_system(systemCommand);
  
  strcpy(systemCommand, "touch ");
  strcat(systemCommand, fname);

  strcat(systemCommand, "/slinktest");
  check_system(systemCommand);
  
  strcpy(fname, dir.c_str());
  strcat(fname, "linktest/slinktest");
  
  strcpy(slinkName, dir.c_str());
  strcat(slinkName, "linktest/linkedfile");
  zoidfs::zoidfs_cache_hint_t from_parent_hint;
  zoidfs::zoidfs_cache_hint_t to_parent_hint;
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  zoidfs::zoidfs_sattr_t attr;
  struct stat info;
  lstat(fname, &info);
  attr.uid = info.st_uid;
  attr.gid = info.st_gid;
  client->link(NULL,NULL,fname, NULL,NULL,slinkName, &from_parent_hint,
                  &to_parent_hint, &op_hint);


  if (stat(slinkName, &info ) != 0)
  {
    strcpy(fname, dir.c_str());
    strcat(fname, "linktest");
    strcpy(systemCommand, "rm -r ");
    strcat(systemCommand, fname);
    check_system(systemCommand);
    std::cout << "symlink : Could not create symlink with full path" << std::endl;
    return -1;
  }
  strcpy(systemCommand, "rm ");
  strcat(systemCommand, slinkName);
  check_system(systemCommand);
  
  strcpy(fname, dir.c_str());
  strcat(fname, "linktest");

  zoidfs::zoidfs_handle_t pathHandle;
  client->lookup (NULL, NULL, fname, &pathHandle, &op_hint);
  
  strcpy(fname, "slinktest");
  strcpy(slinkName, "linkedfile");
  lstat(fname, &info);
  attr.uid = info.st_uid;
  attr.gid = info.st_gid;
  client->link(&pathHandle,fname, NULL, &pathHandle, slinkName, NULL, &from_parent_hint,
                  &to_parent_hint, &op_hint);

  strcpy(slinkName, dir.c_str());
  strcat(slinkName, "linktest/linkedfile");
  if (stat(slinkName, &info ) != 0)
  {
    strcpy(fname, dir.c_str());
    strcat(fname, "linktest");
    strcpy(systemCommand, "rm -r ");
    strcat(systemCommand, fname);
    check_system(systemCommand);
    std::cout << "link : Could not create link with handle/component_name" << std::endl;
    return -1;
  }
  strcpy(fname, dir.c_str());
  strcat(fname, "linktest");
  strcpy(systemCommand, "rm -r ");
  strcat(systemCommand, fname);
  check_system(systemCommand);
  return 0;
}
int setattrTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  std::ofstream testFile;
  char * fname = new char[dir.length() + 18];
  strcpy(fname, dir.c_str());
  strcat(fname, "setAttrTest.txt");
  testFile.open (fname);
  testFile.close();

  /* Get Handle */
  zoidfs::zoidfs_handle_t fullFileHandle;
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  client->lookup (NULL, NULL, fname, &fullFileHandle, &op_hint);
  zoidfs::zoidfs_sattr_t sattr;
  zoidfs::zoidfs_attr_t attr;
  struct stat info;
  sattr.uid = 1000;
  sattr.gid = 1000;  
  sattr.mask = 65535;
  client->setattr (&fullFileHandle, &sattr, &attr, &op_hint);

  lstat(fname, &info);
  if (info.st_uid == 55 && info.st_gid == 55)
      return 0;
  else
    std::cout << "SetAttr: ERROR - File attributes do not match Set Attr" << std::endl;
  return -1;
}

int readlinkTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  int  _N = 1;
  size_t _BSIZE = 4096;
  size_t mem_sizes[_N]; 
  size_t _foff = 0; 
  size_t mem_count, file_count; 
  uint64_t file_sizes[_N], file_starts[_N]; 
  void *mem_starts_write[_N]; 
  size_t _i = 0; 
  mem_count = _N; 
  file_count = _N; 

  for(_i = 0 ; _i < mem_count ; _i++) 
  { 
     mem_starts_write[_i] = malloc(_BSIZE); 
     memset(mem_starts_write[_i], 'z', _BSIZE); 
     file_sizes[_i] = mem_sizes[_i] = _BSIZE; 
     file_starts[_i] = _foff; 
     _foff += _BSIZE; 
  } 

  /* Create Test File */
  std::ofstream testFile;
  char * fname = new char[dir.length() + 18];
  strcpy(fname, dir.c_str());
  strcat(fname, "readlink.txt");
  testFile.open (fname);
  testFile << ((char**)mem_starts_write)[0];
  testFile.close();

  char * sfname = new char[dir.length() + 18];
  strcpy(sfname, dir.c_str());
  strcat(sfname, "sreadl.txt");
  char * systemCommand = new char[500];  
  strcpy(systemCommand, "ln -s ");
  strcat(systemCommand, fname);
  strcat(systemCommand, " ");
  strcat(systemCommand, sfname);
  char * buffer = new char[4096];
  size_t buffer_length = 4096;
  zoidfs::zoidfs_handle_t pathHandle;
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  client->lookup (NULL, NULL, sfname, &pathHandle, &op_hint);
  client->readlink(&pathHandle, buffer, buffer_length, &op_hint);  
  return 0;
}
int symlinkTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  char * fname = new char[dir.length() + 50];
  char * slinkName = new char[dir.length() + 50];
  char * systemCommand = new char[500];  
  strcpy(fname, dir.c_str());
  strcat(fname, "symlinktest");
  strcpy(systemCommand, "mkdir ");
  strcat(systemCommand, fname);
  check_system(systemCommand);
  
  strcpy(systemCommand, "touch ");
  strcat(systemCommand, fname);

  strcat(systemCommand, "/slinktest");
  check_system(systemCommand);
  
  strcpy(fname, dir.c_str());
  strcat(fname, "symlinktest/slinktest");
  
  strcpy(slinkName, dir.c_str());
  strcat(slinkName, "symlinktest/linkedfile");
  zoidfs::zoidfs_cache_hint_t from_parent_hint;
  zoidfs::zoidfs_cache_hint_t to_parent_hint;
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  zoidfs::zoidfs_sattr_t attr;
  struct stat info;
  lstat(fname, &info);
  attr.uid = info.st_uid;
  attr.gid = info.st_gid;
  client->symlink(NULL,NULL,fname, NULL,NULL,slinkName, &attr, &from_parent_hint,
                  &to_parent_hint, &op_hint);


  if (stat(slinkName, &info ) != 0)
  {
    strcpy(fname, dir.c_str());
    strcat(fname, "symlinktest");
    strcpy(systemCommand, "rm -r ");
    strcat(systemCommand, fname);
    check_system(systemCommand);
    std::cout << "symlink : Could not create symlink with full path" << std::endl;
    return -1;
  }
  strcpy(systemCommand, "rm ");
  strcat(systemCommand, slinkName);
  check_system(systemCommand);
  
  strcpy(fname, dir.c_str());
  strcat(fname, "symlinktest");

  zoidfs::zoidfs_handle_t pathHandle;
  client->lookup (NULL, NULL, fname, &pathHandle, &op_hint);
  
  strcpy(fname, "slinktest");
  strcpy(slinkName, "linkedfile");
  lstat(fname, &info);
  attr.uid = info.st_uid;
  attr.gid = info.st_gid;
  client->symlink(&pathHandle,fname, NULL, &pathHandle, slinkName, NULL, &attr, &from_parent_hint,
                  &to_parent_hint, &op_hint);

  strcpy(slinkName, dir.c_str());
  strcat(slinkName, "symlinktest/linkedfile");
  if (stat(slinkName, &info ) != 0)
  {
    strcpy(fname, dir.c_str());
    strcat(fname, "symlinktest");
    strcpy(systemCommand, "rm -r ");
    strcat(systemCommand, fname);
    check_system(systemCommand);
    std::cout << "symlink : Could not create symlink with full path" << std::endl;
    return -1;
  }
  strcpy(fname, dir.c_str());
  strcat(fname, "symlinktest");
  strcpy(systemCommand, "rm -r ");
  strcat(systemCommand, fname);
  check_system(systemCommand);
  return 0;
}


int renameTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  char * fname = new char[dir.length() + 50];
  char * slinkName = new char[dir.length() + 50];
  char * systemCommand = new char[500];  
  strcpy(fname, dir.c_str());
  strcat(fname, "renametest");
  strcpy(systemCommand, "mkdir ");
  strcat(systemCommand, fname);
  check_system(systemCommand);
  
  strcpy(systemCommand, "touch ");
  strcat(systemCommand, fname);
  strcat(systemCommand, "/renamefile");
  check_system(systemCommand);
  
  strcpy(fname, dir.c_str());
  strcat(fname, "renametest/renamefile");
  
  strcpy(slinkName, dir.c_str());
  strcat(slinkName, "renametest/copiedfile");
  zoidfs::zoidfs_cache_hint_t from_parent_hint;
  zoidfs::zoidfs_cache_hint_t to_parent_hint;
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  struct stat info;
  client->rename (NULL,NULL,fname, NULL,NULL,slinkName, &from_parent_hint,
                  &to_parent_hint, &op_hint);


  if (stat(slinkName, &info ) != 0)
  {
    strcpy(fname, dir.c_str());
    strcat(fname, "renametest");
    strcpy(systemCommand, "rm -r ");
    strcat(systemCommand, fname);
    check_system(systemCommand);
    std::cout << "rename : Could not create rename with full path" << std::endl;
    return -1;
  }
  strcpy(systemCommand, "rm ");
  strcat(systemCommand, slinkName);
  check_system(systemCommand);
  
  strcpy(fname, dir.c_str());
  strcat(fname, "renametest");
  
  strcpy(systemCommand, "touch ");
  strcat(systemCommand, fname);
  strcat(systemCommand, "/renamefile");
  check_system(systemCommand);  

  strcpy(fname, dir.c_str());
  strcat(fname, "renametest");

  zoidfs::zoidfs_handle_t pathHandle;
  client->lookup (NULL, NULL, fname, &pathHandle, &op_hint);
  
  strcpy(fname, "renamefile");
  strcpy(slinkName, "copiedfile");
  client->rename(&pathHandle,fname, NULL, &pathHandle, slinkName, NULL, &from_parent_hint,
                  &to_parent_hint, &op_hint);

  strcpy(slinkName, dir.c_str());
  strcat(slinkName, "renametest/copiedfile");
  if (stat(slinkName, &info ) != 0)
  {
    strcpy(fname, dir.c_str());
    strcat(fname, "renametest");
    strcpy(systemCommand, "rm -r ");
    strcat(systemCommand, fname);
    check_system(systemCommand);
    std::cout << "rename : Could not create rename with parent_handle/component_name" << std::endl;
    return -1;
  }
  strcpy(fname, dir.c_str());
  strcat(fname, "renametest");
  strcpy(systemCommand, "rm -r ");
  strcat(systemCommand, fname);
  check_system(systemCommand);
  return 0;
}




int commitTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  int ret;  
  zoidfs::zoidfs_handle_t pathHandle;
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  std::ofstream testFile;
  char * fname = new char[dir.length() + 25];
  strcpy(fname, dir.c_str());
  strcat(fname, "commitTest.txt");
  testFile.open (fname);
  testFile.close();  
  client->lookup (NULL, NULL, fname, &pathHandle, &op_hint);
  ret = client->commit(&pathHandle, &op_hint);
  return ret; 
}

int createTest (iofwdclient::IOFWDClient * client, std::string dir)
{
  int created;
  zoidfs::zoidfs_sattr_t sattr;
  zoidfs::zoidfs_handle_t fullFileHandle;
  std::ofstream testFile;
  char * fname = new char[dir.length() + 18];
  strcpy(fname, dir.c_str());
  strcat(fname, "createtest.txt");
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  client->create(NULL, NULL, fname, &sattr, &fullFileHandle, &created, &op_hint);
  std::ifstream ifile(fname);
  if (!ifile || created != 1) {
    std::cout << "File was not created!" << std::endl;
    return -1;
  }
  return 0;
}

int readdirTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  zoidfs::zoidfs_handle_t fullFileHandle;
  char * fname = new char[dir.length() + 50];
  char * systemCommand = new char[500];  
  strcpy(fname, dir.c_str());
  strcat(fname, "dirTest");
  strcpy(systemCommand, "mkdir ");
  strcat(systemCommand, fname);
  check_system(systemCommand);
  
  strcpy(systemCommand, "touch ");
  strcat(systemCommand, fname);
  strcat(systemCommand, "/testfile1");
  check_system(systemCommand);
  
  strcpy(systemCommand, "touch ");
  strcat(systemCommand, fname);
  strcat(systemCommand, "/testfile2");
  check_system(systemCommand);  

  strcpy(systemCommand, "touch ");
  strcat(systemCommand, fname);
  strcat(systemCommand, "/testfile3");
  check_system(systemCommand);  

  /* Lookup handle */
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  client->lookup (NULL, NULL, fname, &fullFileHandle, &op_hint);

  /* Read Directory */
  zoidfs::zoidfs_dirent_cookie_t cookie = 0;
  size_t entry_count = 10;
  zoidfs::zoidfs_dirent_t * entries = new zoidfs::zoidfs_dirent_t[10];
  uint32_t flags = 0;
  zoidfs::zoidfs_cache_hint_t parent_hint;
  client->readdir(&fullFileHandle, cookie, &entry_count, entries, flags,
                  &parent_hint, &op_hint);

  int count = 0;
  for (int x = 0; x < 4; x++)
    if (strcmp(entries[x].name,"testfile1") == 0 || strcmp(entries[x].name,"testfile2") == 0||
        strcmp(entries[x].name,"testfile3") == 0)
        count++; 

  strcpy(systemCommand, "rm -r ");
  strcat(systemCommand, fname);
  check_system(systemCommand);
  if (count < 3)
  {
    std::cout << "readdir : error -  Some files are missing from readdir entries list!" << std::endl;
    return -1;
  }
  return 0;
}

int removeTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  zoidfs::zoidfs_handle_t fullFileHandle;
  struct stat info;
  char * fname = new char[dir.length() + 50];
  char * systemCommand = new char[500];  
  strcpy(fname, dir.c_str());
  strcat(fname, "removeTest");
  strcpy(systemCommand, "mkdir ");
  strcat(systemCommand, fname);
  check_system(systemCommand);
  
  strcpy(systemCommand, "touch ");
  strcat(systemCommand, fname);
  strcat(systemCommand, "/testfile1");
  check_system(systemCommand);
  
  strcat(fname, "/testfile1");
  zoidfs::zoidfs_cache_hint_t hint;
  client->remove (NULL, NULL, fname, &hint, &op_hint);

  strcpy(fname, dir.c_str());
  strcat(fname, "removeTest/testfile1");
  if (stat(fname, &info ) == 0)
  {
    strcpy(fname, dir.c_str());
    strcat(fname, "removeTest");
    strcpy(systemCommand, "rm -r ");
    strcat(systemCommand, fname);
    check_system(systemCommand);
    std::cout << "remove : Unable to remove file using full path" << std::endl;
    return -1;
  }  

  strcpy(fname, dir.c_str());
  strcat(fname, "removeTest");
  strcpy(systemCommand, "touch ");
  strcat(systemCommand, fname);
  strcat(systemCommand, "/testfile1");
  check_system(systemCommand);
  client->lookup (NULL, NULL, fname, &fullFileHandle, &op_hint);
  client->remove (&fullFileHandle, "testfile1",NULL, &hint, &op_hint);
  strcpy(fname, dir.c_str());
  strcat(fname, "removeTest/testfile1");
  if (stat(fname, &info ) == 0)
  {
    strcpy(fname, dir.c_str());
    strcat(fname, "removeTest");
    strcpy(systemCommand, "rm -r ");
    strcat(systemCommand, fname);
    check_system(systemCommand);

    std::cout << "remove : Unable to remove file using handle/component_name" << std::endl;
    return -1;
  }
  strcpy(fname, dir.c_str());
  strcat(fname, "removeTest");
  strcpy(systemCommand, "rm -r ");
  strcat(systemCommand, fname);
  check_system(systemCommand);
  return 0;
}


int lookupTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  zoidfs::zoidfs_handle_t pathHandle;
  zoidfs::zoidfs_handle_t fullFileHandle;
  zoidfs::zoidfs_handle_t tmpHandle;
  std::ofstream testFile;
  char * fname = new char[dir.length() + 18];
  strcpy(fname, dir.c_str());
  strcat(fname, "lookuptest.txt");
  testFile.open (fname);
  testFile.close();
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  client->lookup (NULL, NULL, fname, &fullFileHandle, &op_hint);
  client->lookup (NULL, NULL, dir.c_str(), &pathHandle, &op_hint);
  client->lookup (&pathHandle, "lookuptest.txt",NULL, &tmpHandle, &op_hint);
  for (int x = 0; x < 32; x++)
    if (tmpHandle.data[x] != fullFileHandle.data[x])
    {
      std::cout << "Error with lookup, Full path lookup and handle lookup do not match at position " << x << std::endl;
      return -1;
    }
  return 0;
}

int mkdirTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  char * systemCommand = new char[500];  
  zoidfs::zoidfs_handle_t fullFileHandle;
  zoidfs::zoidfs_sattr_t sattr;
  zoidfs::zoidfs_cache_hint_t parent_hint;
  zoidfs::zoidfs_op_hint_t op_hint;

  sattr.uid = 1000;
  sattr.gid = 1000;

  /* Directory to make */
  char * fname = new char[dir.length() + 18];
  strcpy(fname, dir.c_str());
  strcat(fname, "dirTest");

  /* full path mkdir test */
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  client->mkdir(NULL, NULL, fname, &sattr, &parent_hint, &op_hint);

  struct stat info;
  if (stat( fname, &info ) != 0)
  {
    std::cout << "mkdir : Could not create directory with full path" << std::endl;
    return -1;
  }
  strcpy(systemCommand, "rm -r ");
  strcat(systemCommand, fname);
  check_system(systemCommand);

  /* Directory to make */
  strcpy(fname, "dirTest2");
  client->lookup (NULL, NULL, dir.c_str(), &fullFileHandle, &op_hint);
  client->mkdir(&fullFileHandle, fname, (const char *) NULL, &sattr, &parent_hint, &op_hint);  

  strcpy(fname, dir.c_str());
  strcat(fname, "dirTest2");
  struct stat info2;
  if (stat( fname, &info2 ) != 0)
  {
    std::cout << "mkdir : Could not create directory with full path" << std::endl;
    return -1;
  }

  strcpy(systemCommand, "rm -r ");
  strcat(systemCommand, fname);
  check_system(systemCommand);

  return 0;  
}

int getattrTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  /* Create Test File */
  std::ofstream testFile;
  char * fname = new char[dir.length() + 18];
  strcpy(fname, dir.c_str());
  strcat(fname, "getAttrTest.txt");
  testFile.open (fname);
  testFile.close();

  /* Get Handle */
  zoidfs::zoidfs_handle_t fullFileHandle;
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  client->lookup (NULL, NULL, fname, &fullFileHandle, &op_hint);

  /* Get Attributes */  
  zoidfs::zoidfs_attr_t attr;
  client->getattr (&fullFileHandle,&attr, &op_hint);

  /* Check againts actual file attributes */
  struct stat info;
  lstat(fname, &info);
  if (info.st_nlink == attr.nlink && info.st_uid == attr.uid &&
      info.st_gid == attr.gid)
      return 0;
  else
    std::cout << "GetAttr: ERROR - File attributes do not match IOFSL Returned Attributes" << std::endl;

  return -1;
}
int writeTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  /* Create Test File */
  std::ofstream testFile;
  char * fname = new char[dir.length() + 18];
  strcpy(fname, dir.c_str());
  strcat(fname, "writetest.txt");
  testFile.open (fname);
  testFile.close();

  zoidfs::zoidfs_handle_t fullFileHandle;
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  client->lookup (NULL, NULL, fname, &fullFileHandle, &op_hint);
             
  int  _N = 1;
  size_t _BSIZE = 5000000;
  size_t mem_sizes[_N]; 
  size_t _foff = 0; 
  size_t mem_count, file_count; 
  uint64_t file_sizes[_N], file_starts[_N]; 
  void *mem_starts_write[_N]; 
  size_t _i = 0; 
  mem_count = _N; 
  file_count = _N; 

  for(_i = 0 ; _i < mem_count ; _i++) 
  { 
     mem_starts_write[_i] = malloc(_BSIZE); 
     memset(mem_starts_write[_i], 'q', _BSIZE); 
     file_sizes[_i] = mem_sizes[_i] = _BSIZE; 
     file_starts[_i] = _foff; 
     _foff += _BSIZE; 
  } 

  client->write (&fullFileHandle, mem_count, (const void **)mem_starts_write, 
                 mem_sizes, file_count, file_starts, file_sizes, 
                 ZOIDFS_NO_OP_HINT); 

  for(_i = 0 ; _i < mem_count ; _i++) 
  { 
     free(mem_starts_write[_i]); 
  } 

  std::ifstream checkFile (fname);
  std::string line;
  if (checkFile.is_open())
  {
    while ( checkFile.good() )
    {
      std::getline (checkFile, line);
      if (line.length() != _BSIZE)
      {
        std::cout << "Write did not write out full data size" << std::endl;
        return -1; 
      }
      for (size_t x = 0; x < _BSIZE; x++)
        if (line.c_str()[x] != 'q')
        {
          std::cout << "INVALID CHARACTOR AT " << x << std::endl;
          return -1;
        }
    }
    checkFile.close();
  }
  else
  {
    std::cout << "Unable to open file"; 
    return -1;
  }
  return 0;
}

int readTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  int  _N = 1;
  size_t _BSIZE = 5000000;
  size_t mem_sizes[_N]; 
  size_t _foff = 0; 
  size_t mem_count, file_count; 
  uint64_t file_sizes[_N], file_starts[_N]; 
  void *mem_starts_write[_N]; 
  size_t _i = 0; 
  mem_count = _N; 
  file_count = _N; 

  for(_i = 0 ; _i < mem_count ; _i++) 
  { 
     mem_starts_write[_i] = malloc(_BSIZE); 
     memset(mem_starts_write[_i], 'z', _BSIZE); 
     file_sizes[_i] = mem_sizes[_i] = _BSIZE; 
     file_starts[_i] = _foff; 
     _foff += _BSIZE; 
  } 

  /* Create Test File */
  std::ofstream testFile;
  char * fname = new char[dir.length() + 18];
  strcpy(fname, dir.c_str());
  strcat(fname, "lookuptest.txt");
  testFile.open (fname);
  testFile << ((char**)mem_starts_write)[0];
  testFile.close();

  zoidfs::zoidfs_handle_t fullFileHandle;
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  client->lookup (NULL, NULL, fname, &fullFileHandle, &op_hint);

  for(_i = 0 ; _i < mem_count ; _i++) 
  { 
     free(mem_starts_write[_i]); 
  } 

  _foff = 0;  
  for(_i = 0 ; _i < mem_count ; _i++) 
  { 
     mem_starts_write[_i] = malloc(_BSIZE); 
     file_sizes[_i] = mem_sizes[_i] = _BSIZE; 
     file_starts[_i] = _foff; 
     _foff += _BSIZE; 
  } 
  client->read (&fullFileHandle, mem_count, (void **)mem_starts_write, mem_sizes, 
                file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT); 

  for (size_t x = 0; x < _BSIZE; x++)
    if (((char**)mem_starts_write)[0][x] != 'z')
    {
      std::cout << "INVALID CHARACTOR AT " << x << std::endl;
      return -1;
    }
  return 0;
}

int runTest(std::string r, std::string s, std::string c, std::string d)
{
  /* Setup client */
  iofwdclient::IOFWDClient * client = iofslSetup((char*)s.c_str(), c);
  if (strcmp(r.c_str(), "lookup") == 0)
    return lookupTest(client, d);
  else if (strcmp(r.c_str(), "write") == 0)
    return writeTest(client, d);
  else if (strcmp(r.c_str(), "read") == 0)
    return readTest(client, d);
  else if (strcmp(r.c_str(), "commit") == 0)
    return commitTest(client, d);
  else if (strcmp(r.c_str(), "create") == 0)
    return createTest(client, d);
  else if (strcmp(r.c_str(), "getattr") == 0)
    return getattrTest(client, d);
  else if (strcmp(r.c_str(), "mkdir") == 0)
    return mkdirTest(client, d);
  else if (strcmp(r.c_str(), "readdir") == 0)
    return readdirTest(client, d);
  else if (strcmp(r.c_str(), "symlink") == 0)
    return symlinkTest(client, d);
  else if (strcmp(r.c_str(), "rename") == 0)
    return renameTest(client, d);
  else if (strcmp(r.c_str(), "remove") == 0)
    return removeTest(client, d);
  else if (strcmp(r.c_str(), "readlink") == 0)
    return readlinkTest(client, d);
  else if (strcmp(r.c_str(), "setattr") == 0)
    return setattrTest(client, d);
  else if (strcmp(r.c_str(), "link") == 0)
    return linkTest(client, d);
  std::cout << "No valid request selected for testing" << std::endl;
  return -1;

}

int main (int argc, char *argv[])
{
   // Declare the supported options.
   po::options_description desc("Allowed options");
   desc.add_options()
      ("help", "produce help message")
      ("list", "List registered transforms")
      ("request", po::value<std::string>(), "Name of request to test (read,write,lookup,ect)")
      ("verbose", "Verbose operation")
      ("server", po::value<std::string>(), "IOFSL Server Address (ex: tcp://127.0.0.1:9001)")
      ("config", po::value<std::string>(), "Client Configuration File")
      ("dir", po::value<std::string>(), "Test Directory")
      ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);
   if (vm.count("help"))
   {
      std::cout << desc << std::endl;
      return 0;
   }  
   if (vm.count("request") == 0)
   {
      std::cout << "Missing request flag, please see --help or --list for parameter information"
                << std::endl;
      return -1;
   }
   if (vm.count("server") == 0)
   {
      std::cout << "Missing server flag, please see --help or --list for parameter information"
                << std::endl;
      return -1;
   }
   if (vm.count("config") == 0)
   {
      std::cout << "Missing config flag, please see --help or --list for parameter information"
                << std::endl;
      return -1;
   }
   if (vm.count("dir") == 0)
   {
      std::cout << "Missing dir flag, please see --help or --list for parameter information" 
                << std::endl;
      return -1;
   }

   std::string request = vm["request"].as<std::string>();
   std::string server = vm["server"].as<std::string>();
   std::string config = vm["config"].as<std::string>();
   std::string dir = vm["dir"].as<std::string>();
   return runTest(request, server, config, dir);
}
