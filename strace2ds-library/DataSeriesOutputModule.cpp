/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the DataSeriesOutputModule.hpp
 * header file.
 *
 * Read the DataSeriesOutputModule.hpp file for more information about this
 * class.
 */

#include "DataSeriesOutputModule.hpp"

#define INVALID_FIELD_LENGTH -1
#define GET_INT_LENGTH(len, args_map, field_enum)                 \
  {                                                               \
    if (args_map[field_enum] != NULL) {                           \
      len = *(reinterpret_cast<int *>(args_map[field_enum]));     \
    } else {                                                      \
      len = INVALID_FIELD_LENGTH;                                 \
      args_map[SYSCALL_FIELD_BUFFER_NOT_CAPTURED] = (void *)true; \
    }                                                             \
  }

bool DataSeriesOutputModule::true_ = true;
bool DataSeriesOutputModule::false_ = false;

// Constructor to set up all extents and fields
DataSeriesOutputModule::DataSeriesOutputModule(std::ifstream &table_stream,
                                               const std::string xml_dir,
                                               const char *output_file)
    : /*
       * Create a new DataSeriesSink(filename, compression_modes,
       * compression_level), and open
       * filename. output_file is the name of the file to write to.
       * compression_modes and
       * compression_level should both be 0 to disable compression (can always
       * use ds-repack to
       * compress a ds file).
       */
      ds_sink_(output_file, 0, 0),
      record_num_(0) {
  /*
   * Provide a hint to the library to set the number of buckets to be the most
   * appropriate for
   * the number of elements
   */
  modules_.reserve(nsyscalls);
  extents_.reserve(nsyscalls);
  config_table_.reserve(nsyscalls);

  // Initialize field enum - name cache
  field_enum_cache = new std::unordered_map<std::string, int>();

  // Initialize config table
  initConfigTable(table_stream);

  // Registering extent types to the library
  ExtentTypeLibrary extent_type_library;

  uint32_t extent_size = DEFAULT_EXTENT_SIZE;

  // Loop through each extent and create its fields from xmls
  for (auto const &extent : config_table_) {
    const std::string &extent_name = extent.first;

    // Loading extent XML descriptions from outside file
    std::ifstream extent_xml_file((xml_dir + extent_name + ".xml").c_str());
    if (!extent_xml_file.is_open()) {
      std::cout << extent_name << ": Could not open xml file!\n";
      exit(1);
    }
    std::string extent_xml_description, str;
    while (getline(extent_xml_file, str)) extent_xml_description += str + "\n";

    // Register the ExtentXMLDescription
    const ExtentType::Ptr extent_type =
        extent_type_library.registerTypePtr(extent_xml_description);

    // Create ExtentSeries, OutPutModule, and fields
    ExtentSeries *extent_series = new ExtentSeries();
    modules_[extent_name] =
        new OutputModule(ds_sink_, *extent_series, extent_type, extent_size);
    addExtent(extent_name, *extent_series);
  }

  // We do not need field enum cache anymore
  delete field_enum_cache;

  // Write out the extent type extent.
  ds_sink_.writeExtentLibrary(extent_type_library);

  // Initialize function pointer map
  initArgsMapFuncPtr();

  // Initialize Cache
  initCache();

  // Initialize Syscall Name Number Map
  initSyscallNameNumberMap();
}

// Initializes all the caches with NULL values
void DataSeriesOutputModule::initCache() {
  modules_cache_ = new OutputModule *[nsyscalls];
  extents_cache_ = new FieldMap *[nsyscalls];
  config_table_cache_ = new config_table_entry_pair **[nsyscalls];
  func_ptr_map_cache_ = new SysCallArgsMapFuncPtr[nsyscalls];
  for (int i = 0; i < nsyscalls; i++) {
    modules_cache_[i] = NULL;
    extents_cache_[i] = NULL;
    config_table_cache_[i] = NULL;
    func_ptr_map_cache_[i] = NULL;
  }
}

/*
 * Inserts <syscall_name, address of syscall_args_map_func> pair
 * into func_ptr_map_.
 */
void DataSeriesOutputModule::initArgsMapFuncPtr() {
  // accept system call
  func_ptr_map_["accept"] = &DataSeriesOutputModule::makeAcceptArgsMap;
  // accept4 system call
  func_ptr_map_["accept4"] = &DataSeriesOutputModule::makeAccept4ArgsMap;
  // access system call
  func_ptr_map_["access"] = &DataSeriesOutputModule::makeAccessArgsMap;
  // bind system call
  func_ptr_map_["bind"] = &DataSeriesOutputModule::makeBindArgsMap;
  // chdir system call
  func_ptr_map_["chdir"] = &DataSeriesOutputModule::makeChdirArgsMap;
  // chdir system call
  func_ptr_map_["chroot"] = &DataSeriesOutputModule::makeChrootArgsMap;
  // chmod system call
  func_ptr_map_["chmod"] = &DataSeriesOutputModule::makeChmodArgsMap;
  // chown system call
  func_ptr_map_["chown"] = &DataSeriesOutputModule::makeChownArgsMap;
  // clone system call
  func_ptr_map_["clone"] = &DataSeriesOutputModule::makeCloneArgsMap;
  // close system call
  func_ptr_map_["close"] = &DataSeriesOutputModule::makeCloseArgsMap;
  // connect system call
  func_ptr_map_["connect"] = &DataSeriesOutputModule::makeConnectArgsMap;
  // creat system call
  func_ptr_map_["creat"] = &DataSeriesOutputModule::makeCreatArgsMap;
  // dup system call
  func_ptr_map_["dup"] = &DataSeriesOutputModule::makeDupArgsMap;
  // dup2 system call
  func_ptr_map_["dup2"] = &DataSeriesOutputModule::makeDup2ArgsMap;
  // dup3 system call
  func_ptr_map_["dup3"] = &DataSeriesOutputModule::makeDup3ArgsMap;
  // execve system call
  func_ptr_map_["execve"] = &DataSeriesOutputModule::makeExecveArgsMap;
  // _exit system call
  func_ptr_map_["exit"] = &DataSeriesOutputModule::makeExitArgsMap;
  // faccessat system call
  func_ptr_map_["faccessat"] = &DataSeriesOutputModule::makeFAccessatArgsMap;
  // fchmod system call
  func_ptr_map_["fchmod"] = &DataSeriesOutputModule::makeFChmodArgsMap;
  // fchmodat system call
  func_ptr_map_["fchmodat"] = &DataSeriesOutputModule::makeFChmodatArgsMap;
  // fchdir system call
  func_ptr_map_["fchdir"] = &DataSeriesOutputModule::makeFChdirArgsMap;
  // fcntl system call
  func_ptr_map_["fcntl"] = &DataSeriesOutputModule::makeFcntlArgsMap;
  // fgetxattr system call
  func_ptr_map_["fgetxattr"] = &DataSeriesOutputModule::makeFGetxattrArgsMap;
  // flistxattr system call
  func_ptr_map_["flistxattr"] = &DataSeriesOutputModule::makeFListxattrArgsMap;
  // flock system call
  func_ptr_map_["flock"] = &DataSeriesOutputModule::makeFLockArgsMap;
  // fremovexattr system call
  func_ptr_map_["fremovexattr"] =
      &DataSeriesOutputModule::makeFRemovexattrArgsMap;
  // fsetxattr system call
  func_ptr_map_["fsetxattr"] = &DataSeriesOutputModule::makeFSetxattrArgsMap;
  // fstat system call
  func_ptr_map_["fstat"] = &DataSeriesOutputModule::makeFStatArgsMap;
  // fstatat system call
  func_ptr_map_["fstatat"] = &DataSeriesOutputModule::makeFStatatArgsMap;
  // fstatfs system call
  func_ptr_map_["fstatfs"] = &DataSeriesOutputModule::makeFStatfsArgsMap;
  // fsync system call
  func_ptr_map_["fsync"] = &DataSeriesOutputModule::makeFsyncArgsMap;
  // fdatasync system call
  func_ptr_map_["fdatasync"] = &DataSeriesOutputModule::makeFdatasyncArgsMap;
  // fallocate system call
  func_ptr_map_["fallocate"] = &DataSeriesOutputModule::makeFallocateArgsMap;
  // readahead system call
  func_ptr_map_["readahead"] = &DataSeriesOutputModule::makeReadaheadArgsMap;
  // ftruncate system call
  func_ptr_map_["ftruncate"] = &DataSeriesOutputModule::makeFTruncateArgsMap;
  // getdents system call
  func_ptr_map_["getdents"] = &DataSeriesOutputModule::makeGetdentsArgsMap;
  // getpeername system call
  func_ptr_map_["getpeername"] =
      &DataSeriesOutputModule::makeGetpeernameArgsMap;
  // getrlimit system call
  func_ptr_map_["getrlimit"] = &DataSeriesOutputModule::makeGetrlimitArgsMap;
  // getsockname system call
  func_ptr_map_["getsockname"] =
      &DataSeriesOutputModule::makeGetsocknameArgsMap;
  // getsockopt system call
  func_ptr_map_["getsockopt"] = &DataSeriesOutputModule::makeGetsockoptArgsMap;
  // getxattr system call
  func_ptr_map_["getxattr"] = &DataSeriesOutputModule::makeGetxattrArgsMap;
  // ioctl system call
  func_ptr_map_["ioctl"] = &DataSeriesOutputModule::makeIoctlArgsMap;
  // lgetxattr system call
  func_ptr_map_["lgetxattr"] = &DataSeriesOutputModule::makeLGetxattrArgsMap;
  // link system call
  func_ptr_map_["link"] = &DataSeriesOutputModule::makeLinkArgsMap;
  // linkat system call
  func_ptr_map_["linkat"] = &DataSeriesOutputModule::makeLinkatArgsMap;
  // listxattr system call
  func_ptr_map_["listxattr"] = &DataSeriesOutputModule::makeListxattrArgsMap;
  // listen system call
  func_ptr_map_["listen"] = &DataSeriesOutputModule::makeListenArgsMap;
  // llistxattr system call
  func_ptr_map_["llistxattr"] = &DataSeriesOutputModule::makeLListxattrArgsMap;
  // lremovexattr system call
  func_ptr_map_["lremovexattr"] =
      &DataSeriesOutputModule::makeLRemovexattrArgsMap;
  // lseek system call
  func_ptr_map_["lseek"] = &DataSeriesOutputModule::makeLSeekArgsMap;
  // lsetxattr system call
  func_ptr_map_["lsetxattr"] = &DataSeriesOutputModule::makeLSetxattrArgsMap;
  // lstat system call
  func_ptr_map_["lstat"] = &DataSeriesOutputModule::makeLStatArgsMap;
  // mkdir system call
  func_ptr_map_["mkdir"] = &DataSeriesOutputModule::makeMkdirArgsMap;
  // mkdirat system call
  func_ptr_map_["mkdirat"] = &DataSeriesOutputModule::makeMkdiratArgsMap;
  // mknod system call
  func_ptr_map_["mknod"] = &DataSeriesOutputModule::makeMknodArgsMap;
  // mknodat system call
  func_ptr_map_["mknodat"] = &DataSeriesOutputModule::makeMknodatArgsMap;
  // mmap system call
  func_ptr_map_["mmap"] = &DataSeriesOutputModule::makeMmapArgsMap;
  // munmap system call
  func_ptr_map_["munmap"] = &DataSeriesOutputModule::makeMunmapArgsMap;
  // open system call
  func_ptr_map_["open"] = &DataSeriesOutputModule::makeOpenArgsMap;
  // openat system call
  func_ptr_map_["openat"] = &DataSeriesOutputModule::makeOpenatArgsMap;
  // pipe system call
  func_ptr_map_["pipe"] = &DataSeriesOutputModule::makePipeArgsMap;
  // pread system call
  func_ptr_map_["pread"] = &DataSeriesOutputModule::makePReadArgsMap;
  // pwrite system call
  func_ptr_map_["pwrite"] = &DataSeriesOutputModule::makePWriteArgsMap;
  // read system call
  func_ptr_map_["read"] = &DataSeriesOutputModule::makeReadArgsMap;
  // readlink system call
  func_ptr_map_["readlink"] = &DataSeriesOutputModule::makeReadlinkArgsMap;
  // readv system call
  func_ptr_map_["readv"] = &DataSeriesOutputModule::makeReadvArgsMap;
  // mmappread system call
  func_ptr_map_["mmappread"] = &DataSeriesOutputModule::makeMmapPreadArgsMap;
  // mmappread system call
  func_ptr_map_["mmappwrite"] = &DataSeriesOutputModule::makeMmapPwriteArgsMap;
  // recv system call
  func_ptr_map_["recv"] = &DataSeriesOutputModule::makeRecvArgsMap;
  // recvfrom system call
  func_ptr_map_["recvfrom"] = &DataSeriesOutputModule::makeRecvfromArgsMap;
  // recvmsg system call
  func_ptr_map_["recvmsg"] = &DataSeriesOutputModule::makeRecvmsgArgsMap;
  // removexattr system call
  func_ptr_map_["removexattr"] =
      &DataSeriesOutputModule::makeRemovexattrArgsMap;
  // rename system call
  func_ptr_map_["rename"] = &DataSeriesOutputModule::makeRenameArgsMap;
  // rmdir system call
  func_ptr_map_["rmdir"] = &DataSeriesOutputModule::makeRmdirArgsMap;
  // send system call
  func_ptr_map_["send"] = &DataSeriesOutputModule::makeSendArgsMap;
  // sendto system call
  func_ptr_map_["sendto"] = &DataSeriesOutputModule::makeSendtoArgsMap;
  // sendmsg system call
  func_ptr_map_["sendmsg"] = &DataSeriesOutputModule::makeSendmsgArgsMap;
  // setxattr system call
  func_ptr_map_["setxattr"] = &DataSeriesOutputModule::makeSetxattrArgsMap;
  // setpgid system call
  func_ptr_map_["setpgid"] = &DataSeriesOutputModule::makeSetpgidArgsMap;
  // setrlimit system call
  func_ptr_map_["setrlimit"] = &DataSeriesOutputModule::makeSetrlimitArgsMap;
  // setsid system call
  func_ptr_map_["setsid"] = &DataSeriesOutputModule::makeSetsidArgsMap;
  // setsockopt system call
  func_ptr_map_["setsockopt"] = &DataSeriesOutputModule::makeSetsockoptArgsMap;
  // shutdown system call
  func_ptr_map_["shutdown"] = &DataSeriesOutputModule::makeShutdownArgsMap;
  // socket system call
  func_ptr_map_["socket"] = &DataSeriesOutputModule::makeSocketArgsMap;
  // socket system call
  func_ptr_map_["epoll_create"] =
      &DataSeriesOutputModule::makeEpollCreateArgsMap;
  // socket system call
  func_ptr_map_["epoll_create1"] =
      &DataSeriesOutputModule::makeEpollCreate1ArgsMap;
  // socketpair system call
  func_ptr_map_["socketpair"] = &DataSeriesOutputModule::makeSocketpairArgsMap;
  // stat system call
  func_ptr_map_["stat"] = &DataSeriesOutputModule::makeStatArgsMap;
  // statfs system call
  func_ptr_map_["statfs"] = &DataSeriesOutputModule::makeStatfsArgsMap;
  // symlink system call
  func_ptr_map_["symlink"] = &DataSeriesOutputModule::makeSymlinkArgsMap;
  // symlinkat system call
  func_ptr_map_["symlinkat"] = &DataSeriesOutputModule::makeSymlinkatArgsMap;
  // truncate system call
  func_ptr_map_["truncate"] = &DataSeriesOutputModule::makeTruncateArgsMap;
  // umask system call
  func_ptr_map_["umask"] = &DataSeriesOutputModule::makeUmaskArgsMap;
  // unlink system call
  func_ptr_map_["unlink"] = &DataSeriesOutputModule::makeUnlinkArgsMap;
  // unlinkat system call
  func_ptr_map_["unlinkat"] = &DataSeriesOutputModule::makeUnlinkatArgsMap;
  // utime system call
  func_ptr_map_["utime"] = &DataSeriesOutputModule::makeUtimeArgsMap;
  // utimensat system call
  func_ptr_map_["utimensat"] = &DataSeriesOutputModule::makeUtimensatArgsMap;
  // utimes system call
  func_ptr_map_["utimes"] = &DataSeriesOutputModule::makeUtimesArgsMap;
  // vfork system call
  func_ptr_map_["vfork"] = &DataSeriesOutputModule::makeVForkArgsMap;
  // write system call
  func_ptr_map_["write"] = &DataSeriesOutputModule::makeWriteArgsMap;
  // writev system call
  func_ptr_map_["writev"] = &DataSeriesOutputModule::makeWritevArgsMap;
}

/*
* Creates mapping of syscall name with the syscall
* number
*/
void DataSeriesOutputModule::initSyscallNameNumberMap() {
  // TODO: Find a way to find the libraries relative to the executable.
  // The current way searches a path relative to the user's current directory.
  const char *env_path = "../lib/strace2ds";
  struct stat relative_lib_info;
  int lib_search_return = stat(env_path, &relative_lib_info);
  if (lib_search_return != 0 || !(S_ISDIR(relative_lib_info.st_mode)))
    env_path = "/usr/local/strace2ds";
  std::string file_path =
      std::string(env_path) + "/" + "tables/syscalls_name_number.table";

  std::string input_path(file_path);
  std::ifstream in_file(input_path.c_str());

  std::string line = " ", name, number;
  while (getline(in_file, line)) {
    std::stringstream fss(line);
    getline(fss, name, ' ');
    getline(fss, number, ' ');
    if (name.size() > 0 && number.size() > 0) {
      syscall_name_num_map[name] = stoi(number);
    }
  }
}

/*
 * conversions for system calls that is named differently
 */
void DataSeriesOutputModule::syscall_name_conversion(std::string *extent_name) {
  if (*extent_name == "newfstat") {
    *extent_name = "fstat";
  }

  if (*extent_name == "newlstat") {
    *extent_name = "lstat";
  }

  if (*extent_name == "newfstatat") {
    *extent_name = "fstatat";
  }

  if (*extent_name == "pread64") {
    *extent_name = "pread";
  }

  if (*extent_name == "pwrite64") {
    *extent_name = "pwrite";
  }

  if (*extent_name == "newstat") {
    *extent_name = "stat";
  }
}

/*
 * Register the record and field values into DS fields.
 *
 * @param extent_name: represents the name of a system call being recorded.
 *
 * @param args: represent the array of const arguments passed to a system call.
 *
 * @param common_fields: represents the array of common fields values stored by
 *                       strace
 *
 * @param v_args: represent the helper arguments obtained from strace which are
 *                copied from the address space of actual process being traced.
 */
bool DataSeriesOutputModule::writeRecord(
    const char *extent_name_arg, long *args,
    void *common_fields[DS_NUM_COMMON_FIELDS], void **v_args) {
  void *sys_call_args_map[MAX_SYSCALL_FIELDS];
  struct timeval tv_time_recorded;
  int var32_len;
  uint64_t time_called_Tfrac, time_returned_Tfrac;
  SysCallArgsMapFuncPtr fxn = NULL;
  OutputModule *output_module = NULL;
  FieldMap *field_map = NULL;
  config_table_entry_pair **extent_config_table_ = NULL;
  int scno = -1;
  std::string extent_name(extent_name_arg);

  memset(sys_call_args_map, 0, sizeof(void *) * MAX_SYSCALL_FIELDS);
  /*
   * Create a map from field names to field values.
   * Iterate through every possible fields (via table_).
   * If the field is in the map, then set value of the
   * field.  Otherwise set it to null.
   */

  /* set unique id field */
  sys_call_args_map[SYSCALL_FIELD_UNIQUE_ID] =
      common_fields[DS_COMMON_FIELD_UNIQUE_ID];

  /*
   * Add common field values to the map.
   * NOTE: Some system calls such as _exit(2) do not have
   * time_returned, errno and return values. So we do not
   * set these values into the map.
   */

  if (common_fields[DS_COMMON_FIELD_SYSCALL_NUM] != NULL)
    scno = *static_cast<int *>(common_fields[DS_COMMON_FIELD_SYSCALL_NUM]);

  /* set time called field */
  if (common_fields[DS_COMMON_FIELD_TIME_CALLED] != NULL) {
    if (scno == LTTNG_DEFAULT_SYSCALL_NUM) {
      time_called_Tfrac =
          *((uint64_t *)common_fields[DS_COMMON_FIELD_TIME_CALLED]);
    } else {
      // Convert tv_time_called to Tfracs
      time_called_Tfrac = timespec_to_Tfrac(
          *(struct timespec *)common_fields[DS_COMMON_FIELD_TIME_CALLED]);
    }
    sys_call_args_map[SYSCALL_FIELD_TIME_CALLED] = &time_called_Tfrac;
  }

  /* set time returned field */
  if (common_fields[DS_COMMON_FIELD_TIME_RETURNED] != NULL) {
    if (scno == LTTNG_DEFAULT_SYSCALL_NUM) {
      time_returned_Tfrac =
          *((uint64_t *)common_fields[DS_COMMON_FIELD_TIME_RETURNED]);
    } else {
      // Convert tv_time_returned to Tfracs
      time_returned_Tfrac = timespec_to_Tfrac(
          *(struct timespec *)common_fields[DS_COMMON_FIELD_TIME_RETURNED]);
    }
    sys_call_args_map[SYSCALL_FIELD_TIME_RETURNED] = &time_returned_Tfrac;
  }

  /* set executing pid field */
  if (common_fields[DS_COMMON_FIELD_EXECUTING_PID] != NULL) {
    sys_call_args_map[SYSCALL_FIELD_EXECUTING_PID] =
        common_fields[DS_COMMON_FIELD_EXECUTING_PID];
  }

  /* set executing tid field */
  if (common_fields[DS_COMMON_FIELD_EXECUTING_TID] != NULL) {
    sys_call_args_map[SYSCALL_FIELD_EXECUTING_TID] =
        common_fields[DS_COMMON_FIELD_EXECUTING_TID];
  }

  /* set return value field */
  if (common_fields[DS_COMMON_FIELD_RETURN_VALUE] != NULL) {
    sys_call_args_map[SYSCALL_FIELD_RETURN_VALUE] =
        common_fields[DS_COMMON_FIELD_RETURN_VALUE];
  }

  /* set errno number field */
  if (common_fields[DS_COMMON_FIELD_ERRNO_NUMBER] != NULL) {
    sys_call_args_map[SYSCALL_FIELD_ERRNO_NUMBER] =
        common_fields[DS_COMMON_FIELD_ERRNO_NUMBER];
  }

  /* set buffer not captured field */
  sys_call_args_map[SYSCALL_FIELD_BUFFER_NOT_CAPTURED] =
      common_fields[DS_COMMON_FIELD_BUFFER_NOT_CAPTURED];

  if (scno == LTTNG_DEFAULT_SYSCALL_NUM) {
    scno = syscall_name_num_map[extent_name];
    syscall_name_conversion(&extent_name);
  }

  if (scno >= 0) {
    // lookup func_ptr_map_cache_ here, if cached directly use
    if (func_ptr_map_cache_[scno] != NULL) {
      fxn = func_ptr_map_cache_[scno];
    } else {
      FuncPtrMap::iterator iter = func_ptr_map_.find(extent_name);
      if (iter != func_ptr_map_.end()) {
        fxn = iter->second;
        func_ptr_map_cache_[scno] = fxn;
      }
    }
    // lookup modules_cache_ here, if cached directly use
    if (modules_cache_[scno] != NULL) {
      output_module = modules_cache_[scno];
    } else {
      output_module = modules_[extent_name];
      modules_cache_[scno] = output_module;
    }
    // lookup extents_cache_ here, if cached directly use
    if (extents_cache_[scno] != NULL) {
      field_map = extents_cache_[scno];
    } else {
      field_map = &extents_[extent_name];
      extents_cache_[scno] = field_map;
    }
    // lookup config_table_cache_ here, if cached directly use
    if (config_table_cache_[scno] != NULL) {
      extent_config_table_ = config_table_cache_[scno];
    } else {
      extent_config_table_ = config_table_[extent_name];
      config_table_cache_[scno] = extent_config_table_;
    }

  } else {
    std::cerr << "Error! Negative scno occured:" << extent_name << ":" << scno
              << std::endl;
  }

  // set system call specific field
  if (fxn != NULL)
    (this->*fxn)(sys_call_args_map, args, v_args);
  else
    return false;

  // Create a new record to write
  output_module->newRecord();

  /*
   * Get the time the record was written as late as possible
   * before we actually write the record.
   */
  gettimeofday(&tv_time_recorded, NULL);
  // Convert time_recorded_timeval to Tfracs and add it to the map
  uint64_t time_recorded_Tfrac = timeval_to_Tfrac(tv_time_recorded);
  sys_call_args_map[SYSCALL_FIELD_TIME_RECORDED] = &time_recorded_Tfrac;

  // Write values to the new record
  unsigned int field_enum;
  for (field_enum = 0; field_enum < MAX_SYSCALL_FIELDS; field_enum++) {
    if (extent_config_table_[field_enum] == NULL) continue;

    const std::string &field_name = field_names[field_enum];
    const bool nullable = extent_config_table_[field_enum]->first;
    const ExtentFieldTypePair &extent_field_value_ = (*field_map)[field_enum];
    var32_len = 0;
    if (sys_call_args_map[field_enum] != NULL) {
      void *field_value = sys_call_args_map[field_enum];
      /*
       * If field is of type Variable32, then retrieve the length of the
       * field that needs to be set.
       */
      if (extent_field_value_.second == ExtentType::ft_variable32) {
        var32_len = getVariable32FieldLength(sys_call_args_map, field_enum);
      }

      if (var32_len < 0) {
        common_fields[DS_COMMON_FIELD_BUFFER_NOT_CAPTURED] = (void *)true;
        setField((*field_map)[SYSCALL_FIELD_BUFFER_NOT_CAPTURED],
                 sys_call_args_map[SYSCALL_FIELD_BUFFER_NOT_CAPTURED], 0);
      }

      setField(extent_field_value_, field_value, var32_len);
      continue;
    } else {
      if (nullable) {
        setFieldNull(extent_field_value_);
      } else {
        // Print error message only if there is a field that is missing
        if (!field_name.empty()) {
          std::cerr << extent_name << ":" << field_name << " ";
          std::cerr
              << "WARNING: Attempting to setNull to a non-nullable field. ";
          std::cerr << "This field will take on default value instead."
                    << std::endl;
        }
      }
    }
  }

  return true;
}

void DataSeriesOutputModule::setIoctlSize(uint64_t size) { ioctl_size_ = size; }

uint64_t DataSeriesOutputModule::getIoctlSize() { return ioctl_size_; }

uint64_t DataSeriesOutputModule::getNextID() { return record_num_++; }

void DataSeriesOutputModule::setCloneCTIDIndex(u_int ctid_index) {
  clone_ctid_index_ = ctid_index;
}

u_int DataSeriesOutputModule::getCloneCTIDIndex() { return clone_ctid_index_; }

// Destructor to delete the module
DataSeriesOutputModule::~DataSeriesOutputModule() {
  int i;
  config_table_entry_pair **extent_config_table;

  /*
   * Need to delete dynamically-allocated fields before we can delete
   * ExtentSeries objects
   */
  for (auto const &extent_map_iter : extents_) {
    for (auto const &field_map_iter : extent_map_iter.second) {
      delete (Field *)field_map_iter.first;
    }
  }

  for (auto const &module_map_iter : modules_) {
    // module_map_iter.second is an OutputModule
    if (module_map_iter.second != NULL) {
      module_map_iter.second->flushExtent();
      module_map_iter.second->close();
      delete (ExtentSeries *)&module_map_iter.second->getSeries();
      delete module_map_iter.second;
    }
  }

  /*Pointers to common fields are shared and shouldn't be deleted more than
   * once!*/
  extent_config_table = config_table_["read"];
  delete extent_config_table[SYSCALL_FIELD_TIME_CALLED];
  delete extent_config_table[SYSCALL_FIELD_TIME_RECORDED];
  delete extent_config_table[SYSCALL_FIELD_TIME_RETURNED];
  delete extent_config_table[SYSCALL_FIELD_ERRNO_NUMBER];
  delete extent_config_table[SYSCALL_FIELD_ERRNO_STRING];
  delete extent_config_table[SYSCALL_FIELD_EXECUTING_PGID];
  delete extent_config_table[SYSCALL_FIELD_EXECUTING_PID];
  delete extent_config_table[SYSCALL_FIELD_EXECUTING_PPID];
  delete extent_config_table[SYSCALL_FIELD_EXECUTING_SID];
  delete extent_config_table[SYSCALL_FIELD_EXECUTING_TID];
  delete extent_config_table[SYSCALL_FIELD_EXECUTING_UID];
  delete extent_config_table[SYSCALL_FIELD_RETURN_VALUE];
  delete extent_config_table[SYSCALL_FIELD_UNIQUE_ID];
  delete extent_config_table[SYSCALL_FIELD_BUFFER_NOT_CAPTURED];

  for (auto const &config_table_iter : config_table_) {
    extent_config_table = config_table_iter.second;

    if (!extent_config_table) continue;
    /*delete std::pair objects*/
    for (i = 0; i < MAX_SYSCALL_FIELDS; i++) {
      /*don't delete common fields*/
      switch (i) {
        case SYSCALL_FIELD_TIME_CALLED:
        case SYSCALL_FIELD_TIME_RECORDED:
        case SYSCALL_FIELD_TIME_RETURNED:
        case SYSCALL_FIELD_ERRNO_NUMBER:
        case SYSCALL_FIELD_ERRNO_STRING:
        case SYSCALL_FIELD_EXECUTING_PGID:
        case SYSCALL_FIELD_EXECUTING_PID:
        case SYSCALL_FIELD_EXECUTING_PPID:
        case SYSCALL_FIELD_EXECUTING_SID:
        case SYSCALL_FIELD_EXECUTING_TID:
        case SYSCALL_FIELD_EXECUTING_UID:
        case SYSCALL_FIELD_RETURN_VALUE:
        case SYSCALL_FIELD_UNIQUE_ID:
        case SYSCALL_FIELD_BUFFER_NOT_CAPTURED:
          continue;
        default:
          if (extent_config_table[i] != NULL) delete extent_config_table[i];
      }
    }
    /*now delete the array of pointers*/
    delete[] extent_config_table;
  }

  delete[] modules_cache_;
  delete[] extents_cache_;
  delete[] config_table_cache_;
  delete[] func_ptr_map_cache_;
}

// Initialize config table
void DataSeriesOutputModule::initConfigTable(std::ifstream &table_stream) {
  std::string line;

  /* Special case for Common fields */
  config_table_entry_pair **common_field_map;

  common_field_map = new config_table_entry_pair *[MAX_SYSCALL_FIELDS];
  memset(common_field_map, 0x00,
         (MAX_SYSCALL_FIELDS * sizeof(config_table_entry_pair *)));

  while (getline(table_stream, line)) {
    /* Skipping Comment lines */
    if (line.find_first_of('#', 0) != std::string::npos) {
      continue;
    }
    std::istringstream iss(line);
    std::vector<std::string> split_data{std::istream_iterator<std::string>{iss},
                                        std::istream_iterator<std::string>{}};

    if (split_data.size() != 6 && split_data.size() != 3) {
      std::cout << "Illegal field table file" << std::endl;
      exit(1);
    }
    /* Initializing with default values for system calls without arguments
     * (Default Constructor initializes a string as empty string )*/
    std::string extent_name = split_data[0];
    std::string field_name;
    std::string nullable_str;
    std::string field_type;
    /*
     * We are initializing field_num with the max value
     * Because there are syscalls are taking zero parameter
     */
    unsigned int field_enum = MAX_SYSCALL_FIELDS;
    /* We are ignoring  split_data[1]: syscall_id, split_data[2]: field_id for
     * now */
    if (split_data.size() == 6) {
      field_name = split_data[3];
      nullable_str = split_data[4];
      field_type = split_data[5];
      field_enum = static_cast<int>(std::stoul(split_data[2]));
      if (field_enum > MAX_SYSCALL_FIELDS) {
        std::cout << "Illegal field table file : field id" << std::endl;
        exit(1);
      }
      field_names[field_enum] = field_name;
      (*field_enum_cache)[field_name] = field_enum;
    }

    bool nullable = false;
    if (nullable_str == "1") nullable = true;

    ExtentType::fieldType ftype = ExtentType::ft_unknown;
    if (field_type == "bool")
      ftype = ExtentType::ft_bool;
    else if (field_type == "byte")
      ftype = ExtentType::ft_byte;
    else if (field_type == "int32")
      ftype = ExtentType::ft_int32;
    else if (field_type == "int64" or field_type == "time")
      ftype = ExtentType::ft_int64;
    else if (field_type == "double")
      ftype = ExtentType::ft_double;
    else if (field_type == "variable32")
      ftype = ExtentType::ft_variable32;

    if (extent_name == "Common") {
      common_field_map[field_enum] =
          new config_table_entry_pair(nullable, ftype);
    } else if (config_table_.find(extent_name) != config_table_.end()) {
      // we have to check whether syscall has any arg. or not
      if (field_enum < MAX_SYSCALL_FIELDS) {
        config_table_[extent_name][field_enum] =
            new config_table_entry_pair(nullable, ftype);
      }
    } else { /* New extent detected */
      config_table_[extent_name] =
          new config_table_entry_pair *[MAX_SYSCALL_FIELDS];
      /*copy all the pointers from common_field_map into the new array*/
      memcpy(config_table_[extent_name], common_field_map,
             (MAX_SYSCALL_FIELDS * sizeof(config_table_entry_pair *)));
      // we have to check whether syscall has any arg. or not
      if (field_enum < MAX_SYSCALL_FIELDS) {
        config_table_[extent_name][field_enum] =
            new config_table_entry_pair(nullable, ftype);
      }
    }
  }
  delete[] common_field_map;
}

// Add an extent(system call)
void DataSeriesOutputModule::addExtent(const std::string &extent_name,
                                       ExtentSeries &series) {
  const ExtentType::Ptr extent_type = series.getTypePtr();
  for (uint32_t i = 0; i < extent_type->getNFields(); i++) {
    const std::string &field_name = extent_type->getFieldName(i);
    bool nullable = extent_type->getNullable(field_name);

    switch ((ExtentType::fieldType)extent_type->getFieldType(field_name)) {
      case ExtentType::ft_bool:
        addField(extent_name, field_name,
                 new BoolField(series, field_name, nullable),
                 ExtentType::ft_bool);
        break;
      case ExtentType::ft_byte:
        addField(extent_name, field_name,
                 new ByteField(series, field_name, nullable),
                 ExtentType::ft_byte);
        break;
      case ExtentType::ft_int32:
        addField(extent_name, field_name,
                 new Int32Field(series, field_name, nullable),
                 ExtentType::ft_int32);
        break;
      case ExtentType::ft_int64:
        addField(extent_name, field_name,
                 new Int64Field(series, field_name, nullable),
                 ExtentType::ft_int64);
        break;
      case ExtentType::ft_double:
        addField(extent_name, field_name,
                 new DoubleField(series, field_name, nullable),
                 ExtentType::ft_double);
        break;
      case ExtentType::ft_variable32:
        addField(extent_name, field_name,
                 new Variable32Field(series, field_name, nullable),
                 ExtentType::ft_variable32);
        break;
      default:
        std::stringstream error_msg;
        error_msg << "Unsupported field type: "
                  << extent_type->getFieldType(field_name) << std::endl;
        throw std::runtime_error(error_msg.str());
    }
  }
}

// Add a field(system call arg) to the desired extent
void DataSeriesOutputModule::addField(const std::string &extent_name,
                                      const std::string &field_name,
                                      void *field,
                                      const ExtentType::fieldType field_type) {
  auto field_enum = (*field_enum_cache)[field_name];
  extents_[extent_name][field_enum] = std::make_pair(field, field_type);
}

/*
 * Set corresponding DS field to the given value
 */
void DataSeriesOutputModule::setField(
    const ExtentFieldTypePair &extent_field_value_, void *field_value,
    int var32_len) {
  bool buffer;
  switch (extent_field_value_.second) {
    case ExtentType::ft_bool:
      buffer = (field_value != 0);
      doSetField<BoolField, bool>(extent_field_value_, &buffer);
      break;
    case ExtentType::ft_byte:
      doSetField<ByteField, ExtentType::byte>(extent_field_value_, field_value);
      break;
    case ExtentType::ft_int32:
      doSetField<Int32Field, ExtentType::int32>(extent_field_value_,
                                                field_value);
      break;
    case ExtentType::ft_int64:
      doSetField<Int64Field, ExtentType::int64>(extent_field_value_,
                                                field_value);
      break;
    case ExtentType::ft_double:
      doSetField<DoubleField, double>(extent_field_value_, field_value);
      break;
    case ExtentType::ft_variable32:
      if (var32_len < 0) {
        /*
         * var32_len may be negative in the cases, where we use the return value
         * of a failed system call as the length.
         * In those cases, we set the Variable32Field to NULL.
         */
        ((Variable32Field *)(extent_field_value_.first))->setNull();
      } else {
        ((Variable32Field *)(extent_field_value_.first))
            ->set((*(char **)field_value), var32_len);
      }
      break;
    default:
      std::stringstream error_msg;
      error_msg << "Unsupported field type: " << extent_field_value_.second
                << std::endl;
      throw std::runtime_error(error_msg.str());
  }
}

/*
 * Set corresponding DS field to null
 */
void DataSeriesOutputModule::setFieldNull(
    const ExtentFieldTypePair &extent_field_value_) {
  switch (extent_field_value_.second) {
    case ExtentType::ft_bool:
      ((BoolField *)(extent_field_value_.first))->setNull();
      break;
    case ExtentType::ft_byte:
      ((ByteField *)(extent_field_value_.first))->setNull();
      break;
    case ExtentType::ft_int32:
      ((Int32Field *)(extent_field_value_.first))->setNull();
      break;
    case ExtentType::ft_int64:
      ((Int64Field *)(extent_field_value_.first))->setNull();
      break;
    case ExtentType::ft_double:
      ((DoubleField *)(extent_field_value_.first))->setNull();
      break;
    case ExtentType::ft_variable32:
      ((Variable32Field *)(extent_field_value_.first))->setNull();
      break;
    default:
      std::stringstream error_msg;
      error_msg << "Unsupported field type: " << extent_field_value_.second
                << std::endl;
      throw std::runtime_error(error_msg.str());
  }
}

template <typename FieldType, typename ValueType>
void DataSeriesOutputModule::doSetField(
    const ExtentFieldTypePair &extent_field_value_, void *field_value) {
  ((FieldType *)(extent_field_value_.first))->set(*(ValueType *)field_value);
}

/*
 * Standard string functions does not work for buffer data that is read
 * or written.  Hence we cannot use strlen() in setField() function.
 * This function returns the length of variable32 type field.
 * NOTE: This function should be extended according to the field name of
 * system call as described in SNIA document.
 */
int DataSeriesOutputModule::getVariable32FieldLength(void **args_map,
                                                     const int field_enum) {
  int length = 0;
  if (args_map[field_enum] != NULL) {
    /*
     * If field_name refers to the pathname passed as an argument to
     * the system call, string length function can be used to determine
     * the length.  Strlen does not count the terminating null character,
     * so we add 1 to its return value to get the full length of the pathname.
     */
    switch (field_enum) {
      case SYSCALL_FIELD_GIVEN_PATHNAME:
      case SYSCALL_FIELD_GIVEN_OLDPATHNAME:
      case SYSCALL_FIELD_GIVEN_NEWPATHNAME:
      case SYSCALL_FIELD_TARGET_PATHNAME:
      case SYSCALL_FIELD_GIVEN_OLDNAME:
      case SYSCALL_FIELD_GIVEN_NEWNAME:
      case SYSCALL_FIELD_ARGUMENT:
      case SYSCALL_FIELD_ENVIRONMENT: {
        void *field_value = args_map[field_enum];
        length = strlen(*(char **)field_value) + 1;
        break;
      }
      /*
       * If field_name refers to the actual data read or written, then length
       * of buffer must be the return value of that corresponding system call.
       */
      case SYSCALL_FIELD_DATA_READ:
      case SYSCALL_FIELD_DATA_WRITTEN:
      case SYSCALL_FIELD_LINK_VALUE:
      case SYSCALL_FIELD_DIRENT_BUFFER:
        GET_INT_LENGTH(length, args_map, SYSCALL_FIELD_RETURN_VALUE)
        break;
      case SYSCALL_FIELD_IOCTL_BUFFER:
        length = ioctl_size_;
        break;
      case SYSCALL_FIELD_SOCKADDR_BUFFER:
        GET_INT_LENGTH(length, args_map, SYSCALL_FIELD_SOCKADDR_LENGTH)
        break;
      case SYSCALL_FIELD_OPTION_VALUE:
        GET_INT_LENGTH(length, args_map, SYSCALL_FIELD_BUFFER_SIZE)
        break;
      case SYSCALL_FIELD_IOV_DATA_READ:
      case SYSCALL_FIELD_IOV_DATA_WRITTEN:
        GET_INT_LENGTH(length, args_map, SYSCALL_FIELD_BYTES_REQUESTED)
        break;
      default:
        length = 0;
        break;
    }
  } else {
    std::cerr << "WARNING: field_enum = " << field_enum << " ";
    std::cerr << "is not set in the arguments map";
  }
  return length;
}

// Initialize all non-nullable boolean fields as False of given extent_name.
void DataSeriesOutputModule::initArgsMap(void **args_map,
                                         const char *extent_name) {
  config_table_entry_pair **extent_config_table_ = config_table_[extent_name];
  FieldMap &extent_field_map_ = extents_[extent_name];

  unsigned int field_enum;
  for (field_enum = 0; field_enum < MAX_SYSCALL_FIELDS; field_enum++) {
    if (extent_config_table_[field_enum] == NULL) continue;

    const std::string &field_name = field_names[field_enum];
    const bool nullable = extent_config_table_[field_enum]->first;
    if (!nullable &&
        extent_field_map_[field_enum].second == ExtentType::ft_bool)
      args_map[field_enum] = &false_;
  }
}

/*
 * This function processes the flag and mode values passed as an arguments
 * to system calls.  It checks each individual flag/mode bits and sets the
 * corresponding bits as True in the argument map.
 *
 * @param args_map: stores mapping of <field, value> pairs.
 *
 * @param num: the actual flag or mode value.
 *
 * @param value: specifies flag or mode bit value. Ex: O_RDONLY or S_ISUID.
 *
 * @param field_name: denotes the field name for individual flag/mode bits.
 *                    Ex: "flag_read_only", "mode_R_user".
 */
void DataSeriesOutputModule::process_Flag_and_Mode_Args(void **args_map,
                                                        u_int &num, int value,
                                                        int field_enum) {
  if (num & value) {
    args_map[field_enum] = (void *)1;
    num &= ~value;
  }
}

uint64_t DataSeriesOutputModule::timeval_to_Tfrac(struct timeval tv) {
  double time_seconds = (double)tv.tv_sec + pow(10.0, -6) * tv.tv_usec;
  uint64_t time_Tfracs = (uint64_t)(time_seconds * (((uint64_t)1) << 32));
  return time_Tfracs;
}

uint64_t DataSeriesOutputModule::sec_to_Tfrac(time_t time) {
  uint64_t time_Tfracs = (uint64_t)(time * (((uint64_t)1) << 32));
  return time_Tfracs;
}
