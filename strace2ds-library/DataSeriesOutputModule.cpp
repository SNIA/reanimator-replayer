/*
 * Copyright (c) 2016-2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
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

bool DataSeriesOutputModule::true_ = true;
bool DataSeriesOutputModule::false_ = false;

// Constructor to set up all extents and fields
DataSeriesOutputModule::DataSeriesOutputModule(std::ifstream &table_stream,
					       const std::string xml_dir,
					       const char *output_file) :
  /*
   * Create a new DataSeriesSink(filename, compression_modes, compression_level), and open
   * filename. output_file is the name of the file to write to. compression_modes and
   * compression_level should both be 0 to disable compression (can always use ds-repack to
   * compress a ds file).
   */
  ds_sink_(output_file, 0, 0), record_num_(1) {

  /* 
   * Provide a hint to the library to set the number of buckets to be the most appropriate for 
   * the number of elements
   */
  modules_.reserve(nsyscalls);
  extents_.reserve(nsyscalls);
  config_table_.reserve(nsyscalls);

  // Initialize config table
  initConfigTable(table_stream);

  // Registering extent types to the library
  ExtentTypeLibrary extent_type_library;

  uint32_t extent_size = DEFAULT_EXTENT_SIZE;

  // Loop through each extent and create its fields from xmls
  for (auto const &extent : config_table_) {
    const std::string& extent_name = extent.first;

    // Loading extent XML descriptions from outside file
    std::ifstream extent_xml_file((xml_dir + extent_name + ".xml").c_str());
    if (!extent_xml_file.is_open()) {
      std::cout << extent_name << ": Could not open xml file!\n";
      exit(1);
    }
    std::string extent_xml_description, str;
    while (getline(extent_xml_file, str))
      extent_xml_description += str + "\n";

    // Register the ExtentXMLDescription
    const ExtentType::Ptr extent_type =
      extent_type_library.registerTypePtr(extent_xml_description);

    // Create ExtentSeries, OutPutModule, and fields
    ExtentSeries *extent_series = new ExtentSeries();
    modules_[extent_name] = new OutputModule(ds_sink_, *extent_series,
					     extent_type, extent_size);
    addExtent(extent_name, *extent_series);
  }

  // Write out the extent type extent.
  ds_sink_.writeExtentLibrary(extent_type_library);

  // Initialize function pointer map
  initArgsMapFuncPtr();

  // Initialize Cache
  initCache();
}

// Initializes all the caches with NULL values
void DataSeriesOutputModule::initCache() {
  modules_cache_ = new OutputModule*[nsyscalls];
  extents_cache_ = new FieldMap*[nsyscalls];
  config_table_cache_ = new config_table_entry_type*[nsyscalls];
  func_ptr_map_cache_ = new SysCallArgsMapFuncPtr[nsyscalls];
  for(int i = 0; i < nsyscalls; i++) {
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
  // access system call
  func_ptr_map_["access"] = &DataSeriesOutputModule::makeAccessArgsMap;
  // chdir system call
  func_ptr_map_["chdir"] = &DataSeriesOutputModule::makeChdirArgsMap;
  // chmod system call
  func_ptr_map_["chmod"] = &DataSeriesOutputModule::makeChmodArgsMap;
  // chown system call
  func_ptr_map_["chown"] = &DataSeriesOutputModule::makeChownArgsMap;
  // clone system call
  func_ptr_map_["clone"] = &DataSeriesOutputModule::makeCloneArgsMap;
  // close system call
  func_ptr_map_["close"] = &DataSeriesOutputModule::makeCloseArgsMap;
  // creat system call
  func_ptr_map_["creat"] = &DataSeriesOutputModule::makeCreatArgsMap;
  // dup system call
  func_ptr_map_["dup"] = &DataSeriesOutputModule::makeDupArgsMap;
  // dup2 system call
  func_ptr_map_["dup2"] = &DataSeriesOutputModule::makeDup2ArgsMap;
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
  // fcntl system call
  func_ptr_map_["fcntl"] = &DataSeriesOutputModule::makeFcntlArgsMap;
  // fgetxattr system call
  func_ptr_map_["fgetxattr"] = &DataSeriesOutputModule::makeFGetxattrArgsMap;
  // flistxattr system call
  func_ptr_map_["flistxattr"] = &DataSeriesOutputModule::makeFListxattrArgsMap;
  // flock system call
  func_ptr_map_["flock"] = &DataSeriesOutputModule::makeFLockArgsMap;
  // fremovexattr system call
  func_ptr_map_["fremovexattr"] = &DataSeriesOutputModule::makeFRemovexattrArgsMap;
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
  // ftruncate system call
  func_ptr_map_["ftruncate"] = &DataSeriesOutputModule::makeFTruncateArgsMap;
  // getdents system call
  func_ptr_map_["getdents"] = &DataSeriesOutputModule::makeGetdentsArgsMap;
  // getrlimit system call
  func_ptr_map_["getrlimit"] = &DataSeriesOutputModule::makeGetrlimitArgsMap;
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
  // llistxattr system call
  func_ptr_map_["llistxattr"] = &DataSeriesOutputModule::makeLListxattrArgsMap;
  // lremovexattr system call
  func_ptr_map_["lremovexattr"] = &DataSeriesOutputModule::makeLRemovexattrArgsMap;
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
  // removexattr system call
  func_ptr_map_["removexattr"] = &DataSeriesOutputModule::makeRemovexattrArgsMap;
  // rename system call
  func_ptr_map_["rename"] = &DataSeriesOutputModule::makeRenameArgsMap;
  // rmdir system call
  func_ptr_map_["rmdir"] = &DataSeriesOutputModule::makeRmdirArgsMap;
  // setxattr system call
  func_ptr_map_["setxattr"] = &DataSeriesOutputModule::makeSetxattrArgsMap;
  // setpgid system call
  func_ptr_map_["setpgid"] = &DataSeriesOutputModule::makeSetpgidArgsMap;
  // setrlimit system call
  func_ptr_map_["setrlimit"] = &DataSeriesOutputModule::makeSetrlimitArgsMap;
  // setsid system call
  func_ptr_map_["setsid"] = &DataSeriesOutputModule::makeSetsidArgsMap;
  // socket system call
  func_ptr_map_["socket"] = &DataSeriesOutputModule::makeSocketArgsMap;
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
bool DataSeriesOutputModule::writeRecord(const char *extent_name, long *args,
					 void
					 *common_fields[DS_NUM_COMMON_FIELDS],
					 void **v_args) {

  void *sys_call_args_map[MAX_SYSCALL_FIELDS] = { NULL };
  struct timeval tv_time_recorded;
  int var32_len;
  uint64_t time_called_Tfrac, time_returned_Tfrac;

  /*
   * Create a map from field names to field values.
   * Iterate through every possible fields (via table_).
   * If the field is in the map, then set value of the
   * field.  Otherwise set it to null.
   */

  /* set unique id field */
  sys_call_args_map[SYSCALL_FIELD_UNIQUE_ID] = common_fields[DS_COMMON_FIELD_UNIQUE_ID];

  /*
   * Add common field values to the map.
   * NOTE: Some system calls such as _exit(2) do not have
   * time_returned, errno and return values. So we do not
   * set these values into the map.
   */

  /* set time called field */
  if (common_fields[DS_COMMON_FIELD_TIME_CALLED] != NULL) {
    // Convert tv_time_called to Tfracs
    time_called_Tfrac = timeval_to_Tfrac(
      *(struct timeval *) common_fields[DS_COMMON_FIELD_TIME_CALLED]);
    sys_call_args_map[SYSCALL_FIELD_TIME_CALLED] = &time_called_Tfrac;
  }

  /* set time returned field */
  if (common_fields[DS_COMMON_FIELD_TIME_RETURNED] != NULL) {
    // Convert tv_time_returned to Tfracs
    time_returned_Tfrac = timeval_to_Tfrac(
      *(struct timeval *) common_fields[DS_COMMON_FIELD_TIME_RETURNED]);
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

  int scno = -1;
  if (common_fields[DS_COMMON_FIELD_SYSCALL_NUM] != NULL)
    scno = *static_cast<int*>(common_fields[DS_COMMON_FIELD_SYSCALL_NUM]);

  SysCallArgsMapFuncPtr fxn = NULL;
  OutputModule *output_module = NULL;
  FieldMap *field_map = NULL;
  config_table_entry_type *extent_config_table_ = NULL;

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
      extent_config_table_ = &config_table_[extent_name];
      config_table_cache_[scno] = extent_config_table_;
    }

  } else {
    std::cerr << "Error! Negative scno occured:" << extent_name
                                                 << ":" << scno
                                                 << std::endl;
  }

  // set system call specific field
  if (fxn != NULL)
    (this->*fxn)(sys_call_args_map, args, v_args);

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
  for (auto const &extent_config_table_entry : (*extent_config_table_)) {
    const int field_enum = extent_config_table_entry.first;
    const std::string& field_name = FieldNames[field_enum];
    const bool nullable = extent_config_table_entry.second.first;
    const ExtentFieldTypePair& extent_field_value_ = (*field_map)[field_name];
    var32_len = 0;

      if(sys_call_args_map[field_enum] != NULL) {
      void *field_value = sys_call_args_map[field_enum];
      /*
       * If field is of type Variable32, then retrieve the length of the
       * field that needs to be set.
       */
      if (extent_field_value_.second == ExtentType::ft_variable32) {
        var32_len = getVariable32FieldLength(sys_call_args_map, field_enum);
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
          std::cerr << "WARNING: Attempting to setNull to a non-nullable field. ";
          std::cerr << "This field will take on default value instead." << std::endl;
        }
      }
    }
  }
}

void DataSeriesOutputModule::setIoctlSize(uint64_t size) {
  ioctl_size_ = size;
}

uint64_t DataSeriesOutputModule::getIoctlSize() {
  return ioctl_size_;
}

int64_t DataSeriesOutputModule::getNextID() {
  return record_num_++;
}

void DataSeriesOutputModule::setCloneCTIDIndex(u_int ctid_index) {
  clone_ctid_index_ = ctid_index;
}

u_int DataSeriesOutputModule::getCloneCTIDIndex() {
  return clone_ctid_index_;
}

// Destructor to delete the module
DataSeriesOutputModule::~DataSeriesOutputModule() {
  /*
   * Need to delete dynamically-allocated fields before we can delete
   * ExtentSeries objects
   */
  for(auto const &extent_map_iter: extents_){
    for(auto const &field_map_iter: extent_map_iter.second){
      delete (Field *)field_map_iter.second.first;
    }
  }

  for (auto const &module_map_iter : modules_) {
    // module_map_iter.second is an OutputModule
    module_map_iter.second->flushExtent();
    module_map_iter.second->close();
    delete (ExtentSeries *)&module_map_iter.second->getSeries();
    delete module_map_iter.second;
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
  config_table_entry_type common_field_map;
  while (getline(table_stream, line)) {
    /* Skipping Comment lines */
    if (line.find_first_of('#', 0) != std::string::npos) {
      continue;
    }
    std::istringstream iss(line);
    std::vector<std::string> split_data {std::istream_iterator<std::string>{iss},
                                         std::istream_iterator<std::string>{}};

    if (split_data.size() != 6 && split_data.size() != 3) {
      std::cout << "Illegal field table file" << std::endl;
      exit(1);
    }
    /* Initializing with default values for system calls without arguments (Default Constructor initializes a string as empty string )*/
    std::string extent_name = split_data[0];
    std::string field_name;
    std::string nullable_str;
    std::string field_type;
    int field_enum;
    /* We are ignoring  split_data[1]: syscall_id, split_data[2]: field_id for now */
    if (split_data.size() == 6) {  
      field_name = split_data[3];
      nullable_str = split_data[4];
      field_type = split_data[5];
      field_enum = atoi(split_data[2].c_str());
      FieldNames[field_enum] = field_name;
    }
    bool nullable = false;
    if (nullable_str == "1")
      nullable = true;

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

    if (extent_name == "Common")
      common_field_map[field_enum] = std::make_pair(nullable, ftype);
    else if (config_table_.find(extent_name) != config_table_.end())
      config_table_[extent_name][field_enum] = std::make_pair(nullable, ftype);
    else { /* New extent detected */
      config_table_[extent_name] = common_field_map;
      config_table_[extent_name][field_enum] = std::make_pair(nullable, ftype);
    }
  }
}

// Add an extent(system call)
void DataSeriesOutputModule::addExtent(const std::string &extent_name,
				       ExtentSeries &series) {
  const ExtentType::Ptr extent_type = series.getTypePtr();
  for (uint32_t i = 0; i < extent_type->getNFields(); i++) {
    const std::string &field_name = extent_type->getFieldName(i);
    bool nullable = extent_type->getNullable(field_name);

    switch ((ExtentType::fieldType) extent_type->getFieldType(field_name)) {
    case ExtentType::ft_bool:
      addField(extent_name,
	       field_name,
	       new BoolField(series, field_name, nullable),
	       ExtentType::ft_bool);
      break;
    case ExtentType::ft_byte:
      addField(extent_name,
	       field_name,
	       new ByteField(series, field_name, nullable),
	       ExtentType::ft_byte);
      break;
    case ExtentType::ft_int32:
      addField(extent_name,
	       field_name,
	       new Int32Field(series, field_name, nullable),
	       ExtentType::ft_int32);
      break;
    case ExtentType::ft_int64:
      addField(extent_name,
	       field_name,
	       new Int64Field(series, field_name, nullable),
	       ExtentType::ft_int64);
      break;
    case ExtentType::ft_double:
      addField(extent_name,
	       field_name,
	       new DoubleField(series, field_name, nullable),
	       ExtentType::ft_double);
      break;
    case ExtentType::ft_variable32:
      addField(extent_name,
	       field_name,
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
  extents_[extent_name][field_name] = std::make_pair(field, field_type);
}

/*
 * Set corresponding DS field to the given value
 */
void DataSeriesOutputModule::setField(const ExtentFieldTypePair&
				      extent_field_value_,
				      void *field_value,
				      int var32_len) {
  bool buffer;
  switch (extent_field_value_.second) {
  case ExtentType::ft_bool:
    buffer = (field_value != 0);
    doSetField<BoolField, bool>(extent_field_value_, &buffer);
    break;
  case ExtentType::ft_byte:
    doSetField<ByteField, ExtentType::byte>(extent_field_value_,
					    field_value);
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
    doSetField<DoubleField, double>(extent_field_value_,
				    field_value);
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
      ((Variable32Field *)(extent_field_value_.first)) ->set(
        (*(char **)field_value), var32_len);
    }
    break;
  default:
    std::stringstream error_msg;
    error_msg << "Unsupported field type: "
	      << extent_field_value_.second << std::endl;
    throw std::runtime_error(error_msg.str());
  }
}

/*
 * Set corresponding DS field to null
 */
void DataSeriesOutputModule::setFieldNull(const
					  ExtentFieldTypePair&
					  extent_field_value_) {
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
    error_msg << "Unsupported field type: "
	      << extent_field_value_.second << std::endl;
    throw std::runtime_error(error_msg.str());
  }
}

template <typename FieldType, typename ValueType>
void DataSeriesOutputModule::doSetField(const
					ExtentFieldTypePair&
					extent_field_value_,
					void* field_value) {
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
						     const int &field_enum) {
  int length = 0;
  if (args_map[field_enum] != NULL) {
    /*
     * If field_name refers to the pathname passed as an argument to
     * the system call, string length function can be used to determine
     * the length.  Strlen does not count the terminating null character,
     * so we add 1 to its return value to get the full length of the pathname.
     */
    if ((field_enum ==SYSCALL_FIELD_GIVEN_PATHNAME) ||
      (field_enum == SYSCALL_FIELD_GIVEN_OLDPATHNAME) ||
      (field_enum == SYSCALL_FIELD_GIVEN_NEWPATHNAME) ||
      (field_enum == SYSCALL_FIELD_TARGET_PATHNAME) ||
      (field_enum == SYSCALL_FIELD_GIVEN_OLDNAME) ||
      (field_enum == SYSCALL_FIELD_GIVEN_NEWNAME) ||
      (field_enum == SYSCALL_FIELD_ARGUMENT) ||
      (field_enum == SYSCALL_FIELD_ENVIRONMENT)) {
      void *field_value = args_map[field_enum];
      length = strlen(*(char **) field_value) + 1;
      /*
       * If field_name refers to the actual data read or written, then length
       * of buffer must be the return value of that corresponding system call.
       */
    } else if ((field_enum == SYSCALL_FIELD_DATA_READ) ||
      (field_enum == SYSCALL_FIELD_DATA_WRITTEN) ||
      (field_enum == SYSCALL_FIELD_LINK_VALUE) ||
      (field_enum == SYSCALL_FIELD_DIRENT_BUFFER)) {
      length = *(int *)(args_map[SYSCALL_FIELD_RETURN_VALUE]);
    } else if (field_enum == SYSCALL_FIELD_IOCTL_BUFFER) {
      length = ioctl_size_;
    }
  } else {
    std::cerr << "WARNING: field_name = " << field_enum << " ";
    std::cerr << "is not set in the arguments map";
  }
  return length;
}

// Initialize all non-nullable boolean fields as False of given extent_name.
void DataSeriesOutputModule::initArgsMap(void **args_map,
					 const char *extent_name) {
  const config_table_entry_type& extent_config_table_ =
    config_table_[extent_name];
  FieldMap& extent_field_map_ = extents_[extent_name];
  for (auto const &extent_config_table_entry : extent_config_table_) {
    int field_enum = extent_config_table_entry.first;
    const std::string& field_name = FieldNames[field_enum];
    const bool nullable = extent_config_table_entry.second.first;
    if (!nullable && extent_field_map_[field_name].second == ExtentType::ft_bool)
      args_map[field_enum] = &false_;
  }
}

void DataSeriesOutputModule::makeCloseArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
}

void DataSeriesOutputModule::makeOpenArgsMap(void **args_map,
					     long *args,
					     void **v_args) {
  int offset = 0;

  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "open");

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Open: Pathname is set as NULL!!" << std::endl;
  }

  /* Setting flag values */
  args_map[SYSCALL_FIELD_OPEN_VALUE] = &args[offset + 1];
  u_int flag = processOpenFlags(args_map, args[offset + 1]);
  if (flag != 0) {
    std::cerr << "Open: These flag are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }

  /*
   * If only, open is called with 3 arguments, set the corresponding
   * mode value and mode bits as True.
   */
  if (args[offset + 1] & O_CREAT) {
    mode_t mode = processMode(args_map, args, offset + 2);
    if (mode != 0) {
      std::cerr << "Open: These modes are not processed/unknown->0";
      std::cerr << std::oct << mode << std::dec << std::endl;
    }
  }
}

void DataSeriesOutputModule::makeOpenatArgsMap(void **args_map,
                                              long *args,
                                              void **v_args) {
  int offset = 1;

  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "openat");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
  }

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Openat: Pathname is set as NULL!!" << std::endl;
  }

  /* Setting flag values */
  args_map[SYSCALL_FIELD_OPEN_VALUE] = &args[offset + 1];
  u_int flag = processOpenFlags(args_map, args[offset + 1]);
  if (flag != 0) {
    std::cerr << "Openat: These flags are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }

  /*
   * If openat is called with 4 arguments, set the corresponding
   * mode value and mode bits as True.
   */
  if (args[offset + 1] & O_CREAT) {
    mode_t mode = processMode(args_map, args, offset + 2);
    if (mode != 0) {
      std::cerr << "Openat: These modes are not processed/unknown->0";
      std::cerr << std::oct << mode << std::dec << std::endl;
    }
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
							u_int &num,
							int value,
							int field_enum) {
  if (num & value) {
    args_map[field_enum] = (void *) 1;
    num &= ~value;
  }
}

/*
 * This function unwraps the flag value passed as an argument to the
 * open system call and sets the corresponding flag values as True.
 *
 * @param args_map: stores mapping of <field, value> pairs.
 *
 * @param open_flag: represents the flag value passed as an argument
 *                   to the open system call.
 */
u_int DataSeriesOutputModule::processOpenFlags(void **args_map,
					       u_int open_flag) {

  /*
   * Process each individual flag bits that has been set
   * in the argument open_flag.
   */
  // set read only flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_RDONLY,
			    SYSCALL_FIELD_FLAG_READ_ONLY);
  // set write only flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_WRONLY,
			     SYSCALL_FIELD_FLAG_WRITE_ONLY);
  // set both read and write flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_RDWR,
			     SYSCALL_FIELD_FLAG_READ_AND_WRITE);
  // set append flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_APPEND,
			     SYSCALL_FIELD_FLAG_APPEND);
  // set async flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_ASYNC,
			     SYSCALL_FIELD_FLAG_ASYNC);
  // set close-on-exec flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_CLOEXEC,
			     SYSCALL_FIELD_FLAG_CLOSE_ON_EXEC);
  // set create flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_CREAT,
			     SYSCALL_FIELD_FLAG_CREATE);
  // set direct flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_DIRECT,
			     SYSCALL_FIELD_FLAG_DIRECT);
  // set directory flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_DIRECTORY,
			     SYSCALL_FIELD_FLAG_DIRECTORY);
  // set exclusive flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_EXCL,
			     SYSCALL_FIELD_FLAG_EXCLUSIVE);
  // set largefile flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_LARGEFILE,
			     SYSCALL_FIELD_FLAG_LARGEFILE);
  // set last access time flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NOATIME,
			     SYSCALL_FIELD_FLAG_NO_ACCESS_TIME);
  // set controlling terminal flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NOCTTY,
			     SYSCALL_FIELD_FLAG_NO_CONTROLLING_TERMINAL);
  // set no_follow flag (in case of symbolic link)
  process_Flag_and_Mode_Args(args_map, open_flag, O_NOFOLLOW,
			     SYSCALL_FIELD_FLAG_NO_FOLLOW);
  // set non blocking mode flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NONBLOCK,
			     SYSCALL_FIELD_FLAG_NO_BLOCKING_MODE);
  // set no delay flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NDELAY,
			    SYSCALL_FIELD_FLAG_NO_DELAY);
  // set synchronized IO flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_SYNC,
			     SYSCALL_FIELD_FLAG_SYNCHRONOUS);
  // set truncate mode flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_TRUNC,
			     SYSCALL_FIELD_FLAG_TRUNCATE);

  /*
   * Return remaining unprocessed flags so that caller can
   * warn of unknown flags if the open_flag value is not set
   * as zero.
   */
  return open_flag;
}

/*
 * This function unwraps the mode value passed as an argument to the
 * system call.
 *
 * @param args_map: stores mapping of <field, value> pairs.
 *
 * @param args: represents complete arguments of the actual system call.
 *
 * @param mode_offset: represents index of mode value in the actual
 *		       system call.
 */
mode_t DataSeriesOutputModule::processMode(void **args_map,
					   long *args,
					   u_int mode_offset) {
  // Save the mode argument with mode_value file in map
  args_map[SYSCALL_FIELD_MODE_VALUE] = &args[mode_offset];
  mode_t mode = args[mode_offset];

  // set user-ID bit
  process_Flag_and_Mode_Args(args_map, mode, S_ISUID, SYSCALL_FIELD_MODE_UID);
  // set group-ID bit
  process_Flag_and_Mode_Args(args_map, mode, S_ISGID,  SYSCALL_FIELD_MODE_GID);
  //set sticky bit
  process_Flag_and_Mode_Args(args_map, mode, S_ISVTX,  SYSCALL_FIELD_MODE_STICKY_BIT);
  // set user read permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IRUSR,  SYSCALL_FIELD_MODE_R_USER);
  // set user write permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IWUSR,  SYSCALL_FIELD_MODE_W_USER);
  // set user execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IXUSR,  SYSCALL_FIELD_MODE_X_USER);
  // set group read permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IRGRP,  SYSCALL_FIELD_MODE_R_GROUP);
  // set group write permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IWGRP,  SYSCALL_FIELD_MODE_W_GROUP);
  // set group execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IXGRP,  SYSCALL_FIELD_MODE_X_GROUP);
  // set others read permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IROTH,  SYSCALL_FIELD_MODE_R_OTHERS);
  // set others write permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IWOTH,  SYSCALL_FIELD_MODE_W_OTHERS);
  // set others execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IXOTH,  SYSCALL_FIELD_MODE_X_OTHERS);

  /*
   * Return remaining unprocessed modes so that caller can warn
   * of unknown modes if the mode value is not set as zero.
   */
  return mode;
}

uint64_t DataSeriesOutputModule::timeval_to_Tfrac(struct timeval tv) {
  double time_seconds = (double) tv.tv_sec + pow(10.0, -6) * tv.tv_usec;
  uint64_t time_Tfracs = (uint64_t) (time_seconds * (((uint64_t) 1)<<32));
  return time_Tfracs;
}

uint64_t DataSeriesOutputModule::sec_to_Tfrac(time_t time) {
  uint64_t time_Tfracs = (uint64_t) (time * (((uint64_t) 1)<<32));
  return time_Tfracs;
}

uint64_t DataSeriesOutputModule::timespec_to_Tfrac(struct timespec ts) {
  double time_seconds = (double) ts.tv_sec + pow(10.0, -9) * ts.tv_nsec;
  uint64_t time_Tfracs = (uint64_t)(time_seconds * (((uint64_t)1)<<32));
  return time_Tfracs;
}

void DataSeriesOutputModule::makeReadArgsMap(void **args_map,
					     long *args,
					     void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_DATA_READ] = &v_args[0];
  args_map[SYSCALL_FIELD_BYTES_REQUESTED] = &args[2];
}

void DataSeriesOutputModule::makeWriteArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_DATA_WRITTEN] = &v_args[0];
  args_map[SYSCALL_FIELD_BYTES_REQUESTED] = &args[2];
}

void DataSeriesOutputModule::makeChdirArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Chdir: Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeRmdirArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Rmdir: Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeUnlinkArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Unlink: Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeUnlinkatArgsMap(void **args_map,
						 long *args,
						 void **v_args) {
  initArgsMap(args_map, "unlinkat");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
  }
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Unlinkat: Pathname is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[2];
  u_int flag = args[2];
  process_Flag_and_Mode_Args(args_map, flag, AT_REMOVEDIR,
			     SYSCALL_FIELD_FLAG_REMOVE_DIRECTORY);
  if (flag != 0) {
    std::cerr << "Unlinkat: These flags are not processed/unknown->"
	      << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeMkdirArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  initArgsMap(args_map, "mkdir");
  int mode_offset = 1;
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Mkdir: Pathname is set as NULL!!" << std::endl;
  }
  mode_t mode = processMode(args_map, args, 1);
  if (mode != 0) {
    std::cerr << "Mkdir: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeMkdiratArgsMap(void **args_map,
						long *args,
						void **v_args) {
  int mode_offset = 2;

  // Initialize all non-nullable boolean fields
  initArgsMap(args_map, "mkdirat");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
  }

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Mkdirat: Pathname is set as NULL!!" << std::endl;
  }
  mode_t mode = processMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "Mkdirat: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeCreatArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  initArgsMap(args_map, "creat");
  int mode_offset = 1;
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Creat: Pathname is set as NULL!!" << std::endl;
  }
  mode_t mode = processMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "Creat: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeChmodArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  initArgsMap(args_map, "chmod");
  int mode_offset = 1;
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Chmod: Pathname is set as NULL!!" << std::endl;
  }
  mode_t mode = processMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "Chmod: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeUmaskArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  initArgsMap(args_map, "umask");
  int mode_offset = 0;
  mode_t mode = processMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "Umask: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeSetxattrArgsMap(void **args_map,
						 long *args,
						 void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "setxattr");

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Setxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_NAME] = &v_args[1];
  } else {
    std::cerr << "Setxattr: Attribute name is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_VALUE_WRITTEN] = &v_args[2];
  args_map[SYSCALL_FIELD_VALUE_SIZE] = &args[3];

  /* Setting flag values */
  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[4];
  u_int flag = args[4];
  process_Flag_and_Mode_Args(args_map, flag, XATTR_CREATE, SYSCALL_FIELD_FLAG_XATTR_CREATE);
  process_Flag_and_Mode_Args(args_map, flag, XATTR_REPLACE, SYSCALL_FIELD_FLAG_XATTR_REPLACE);
  if (flag != 0) {
    std::cerr << "Setxattr: These flags are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeLSetxattrArgsMap(void **args_map,
						  long *args,
						  void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "lsetxattr");

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "LSetxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_NAME] = &v_args[1];
  } else {
    std::cerr << "LSetxattr: Attribute name is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_VALUE_WRITTEN] = &v_args[2];
  args_map[SYSCALL_FIELD_VALUE_SIZE] = &args[3];

  /* Setting flag values */
  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[4];
  u_int flag = args[4];
  process_Flag_and_Mode_Args(args_map, flag, XATTR_CREATE, SYSCALL_FIELD_FLAG_XATTR_CREATE);
  process_Flag_and_Mode_Args(args_map, flag, XATTR_REPLACE, SYSCALL_FIELD_FLAG_XATTR_REPLACE);
  if (flag != 0) {
    std::cerr << "LSetxattr: These flags are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeGetxattrArgsMap(void **args_map,
                                                 long *args,
                                                 void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Getxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_NAME] = &v_args[1];
  } else {
    std::cerr << "Getxattr: Attribute name is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_VALUE_READ] = &v_args[2];
  args_map[SYSCALL_FIELD_VALUE_SIZE] = &args[3];
}

void DataSeriesOutputModule::makeLGetxattrArgsMap(void **args_map,
                                                  long *args,
                                                  void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "LGetxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_NAME] = &v_args[1];
  } else {
    std::cerr << "LGetxattr: Attribute name is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_VALUE_READ] = &v_args[2];
  args_map[SYSCALL_FIELD_VALUE_SIZE] = &args[3];
}

void DataSeriesOutputModule::makeFSetxattrArgsMap(void **args_map,
						  long *args,
						  void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "fsetxattr");
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_NAME] = &v_args[0];
  } else {
    std::cerr << "FSetxattr: Attribute name is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_VALUE_WRITTEN] = &v_args[1];
  args_map[SYSCALL_FIELD_VALUE_SIZE] = &args[3];

  /* Setting flag values */
  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[4];
  u_int flag = args[4];
  process_Flag_and_Mode_Args(args_map, flag, XATTR_CREATE, SYSCALL_FIELD_FLAG_XATTR_CREATE);
  process_Flag_and_Mode_Args(args_map, flag, XATTR_REPLACE, SYSCALL_FIELD_FLAG_XATTR_REPLACE);
  if (flag != 0) {
    std::cerr << "FSetxattr: These flag are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeFGetxattrArgsMap(void **args_map,
						  long *args,
						  void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_NAME] = &v_args[0];
  } else {
    std::cerr << "FGetxattr: Attribute name is set as NULL!!" << std::endl;
  }
  args_map[SYSCALL_FIELD_VALUE_READ] = &v_args[1];
  args_map[SYSCALL_FIELD_VALUE_SIZE] = &args[3];
}

void DataSeriesOutputModule::makeListxattrArgsMap(void **args_map,
						  long *args,
						  void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Listxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_LIST] = &v_args[1];
  } else {
    std::cerr << "Listxattr: Attribute list is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_LIST_SIZE] = &args[2];
}

void DataSeriesOutputModule::makeLListxattrArgsMap(void **args_map,
						   long *args,
						   void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "LListxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_LIST] = &v_args[1];
  } else {
    std::cerr << "LListxattr: Attribute list is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_LIST_SIZE] = &args[2];
}

void DataSeriesOutputModule::makeFListxattrArgsMap(void **args_map,
						   long *args,
						   void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_LIST] = &v_args[0];
  } else {
    std::cerr << "FListxattr: Attribute list is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_LIST_SIZE] = &args[2];
}

void DataSeriesOutputModule::makeFLockArgsMap(void **args_map,
                                              long *args,
                                              void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  args_map[SYSCALL_FIELD_OPERATION_VALUE] = &args[1];
  /*
   * TODO: The correct value of args_map["operation"] should be 0 if operation is
   * LOCK_SH, 1 if it is LOCK_EX, 2 if it is LOCK_UN, so and so forth.
   * Currently, we don't do this. We simply assume that resource is same
   * across different platforms.
   */
  args_map[SYSCALL_FIELD_OPERATION] = &args[1];
}

void DataSeriesOutputModule::makeRemovexattrArgsMap(void **args_map,
						    long *args,
						    void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Removexattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_NAME] = &v_args[1];
  } else {
    std::cerr << "Removexattr: Attribute name is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeLRemovexattrArgsMap(void **args_map,
						     long *args,
						     void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "LRemovexattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_NAME] = &v_args[1];
  } else {
    std::cerr << "LRemovexattr: Attribute name is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeFRemovexattrArgsMap(void **args_map,
						     long *args,
						     void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_XATTR_NAME] = &v_args[0];
  } else {
    std::cerr << "FRemovexattr: Attribute name is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeFChmodArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  initArgsMap(args_map, "fchmod");
  int mode_offset = 1;
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  mode_t mode = processMode(args_map, args, 1);
  if (mode != 0) {
    std::cerr << "FChmod: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeFChmodatArgsMap(void **args_map,
						 long *args,
						 void **v_args) {
  int mode_offset = 2;
  initArgsMap(args_map, "fchmodat");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "FChmodat: Pathname is set as NULL!!" << std::endl;
  }

  // set individual  Mode values
  mode_t mode = processMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "FChmodat: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }

  // set flag values
  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[3];
  u_int flag = args[3];
  process_Flag_and_Mode_Args(args_map, flag, AT_SYMLINK_NOFOLLOW, \
			     SYSCALL_FIELD_FLAG_AT_SYMLINK_NOFOLLOW);
  if (flag != 0) {
    std::cerr << "FChmodat: These flags are not processed/unknown->0";
    std::cerr << std::oct << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeLinkArgsMap(void **args_map,
					     long *args,
					     void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_OLDPATHNAME] = &v_args[0];
  } else {
    std::cerr << "Link: Old Pathname is set as NULL!!" << std::endl;
  }
  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_NEWPATHNAME] = &v_args[1];
  } else {
    std::cerr << "Link: New Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeLinkatArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  initArgsMap(args_map, "linkat");

  args_map[SYSCALL_FIELD_OLD_DESCRIPTOR] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_OLD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
  }

  args_map[SYSCALL_FIELD_NEW_DESCRIPTOR] = &args[2];
  if (args[2] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_NEW_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
  }

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_OLDPATHNAME] = &v_args[0];
  } else {
    std::cerr << "Linkat: Old Pathname is set as NULL!!" << std::endl;
  }
  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_NEWPATHNAME] = &v_args[1];
  } else {
    std::cerr << "Linkat: New Pathname is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[4];
  u_int flag = args[4];
  process_Flag_and_Mode_Args(args_map, flag, AT_EMPTY_PATH,
			     SYSCALL_FIELD_FLAG_EMPTY_PATH);
  process_Flag_and_Mode_Args(args_map, flag, AT_SYMLINK_FOLLOW,
			     SYSCALL_FIELD_FLAG_SYMLINK_FOLLOW);
  if (flag != 0) {
    std::cerr << "Linkat: These flags are not processed/unknown->"
	      << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeSymlinkArgsMap(void **args_map,
						long *args,
						void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_TARGET_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Symlink: Target Pathname is set as NULL!!" << std::endl;
  }
  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[1];
  } else {
    std::cerr << "Symlink: Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeSymlinkatArgsMap(void **args_map,
						  long *args,
						  void **v_args) {
  static bool true_ = true;

  initArgsMap(args_map, "symlinkat");

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_TARGET_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Symlinkat: Target Pathname is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_NEW_DESCRIPTOR] = &args[2];
  if (args[2] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_NEW_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[1];
  } else {
    std::cerr << "Symlinkat: Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeTruncateArgsMap(void **args_map,
						 long *args,
						 void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Truncate: Pathname is set as NULL!!" << std::endl;
  }
  args_map[SYSCALL_FIELD_TRUNCATE_LENGTH] = &args[1];
}

void DataSeriesOutputModule::makeAccessArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "access");
  u_int mode_offset = 1;

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Access: Pathname is set as NULL!!" << std::endl;
  }

  // Map the individual mode fields
  mode_t mode = processAccessMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "Access: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeFAccessatArgsMap(SysCallArgsMap &args_map,
						  long *args,
						  void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "faccessat");
  u_int mode_offset = 2;

  args_map["descriptor"] = &args[0];

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "FAccessat: Pathname is set as NULL!!" << std::endl;
  }

  // Map the individual mode fields
  mode_t mode = processAccessMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "FAccessat: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }

  args_map["flags_value"] = &args[3];
  // Map the inividual flag values
  u_int flag = processFAccessatFlags(args_map, args[3]);
  if (flag != 0) {
    std::cerr << "FAccessat: These flags are not processed/unknown->"
	      << std::hex << flag << std::dec << std::endl;
  }
}

u_int DataSeriesOutputModule::processFAccessatFlags(SysCallArgsMap &args_map,
						    u_int faccessat_flags) {
  /*
   * Process each individual faccessat flag bit that has been set
   * in the argument faccessat_flags.
   */
  // set eaccess flag
  process_Flag_and_Mode_Args(args_map, faccessat_flags, AT_EACCESS,
			     "flags_at_eaccess");
  // set symlink nofollow flag
  process_Flag_and_Mode_Args(args_map, faccessat_flags, AT_SYMLINK_NOFOLLOW,
			     "flags_at_symlink_nofollow");

  /*
   * Return remaining faccessat flags so that caller can
   * warn of unknown flags if the faccessat_flags is not set
   * as zero.
   */
  return faccessat_flags;
}

/*
 * This function unwraps the mode value passed as an argument to the
 * Access system call, which has different modes than Open and Mkdir.
 *
 * @param args_map: stores mapping of <field, value> pairs.
 *
 * @param args: represents complete arguments of the actual system call.
 *
 * @param mode_offset: represents index of mode value in the actual
 *		       system call.
 */
mode_t DataSeriesOutputModule::processAccessMode(void **args_map,
						 long *args,
						 u_int mode_offset) {
  // Save the mode argument with mode_value field in the map
  args_map[SYSCALL_FIELD_MODE_VALUE] = &args[mode_offset];
  mode_t mode = args[mode_offset];

  // set read permission bit
  process_Flag_and_Mode_Args(args_map, mode, R_OK, SYSCALL_FIELD_MODE_READ);
  // set write permission bit
  process_Flag_and_Mode_Args(args_map, mode, W_OK, SYSCALL_FIELD_MODE_WRITE);
  // set execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, X_OK, SYSCALL_FIELD_MODE_EXECUTE);
  // set existence bit
  process_Flag_and_Mode_Args(args_map, mode, F_OK, SYSCALL_FIELD_MODE_EXIST);

  /*
   * Return remaining unprocessed modes so that caller can warn
   * of unknown modes if the mode value is not set as zero.
   */
  return mode;
}

void DataSeriesOutputModule::makeLSeekArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_OFFSET] = &args[1];
  args_map[SYSCALL_FIELD_WHENCE] = &args[2];
}

void DataSeriesOutputModule::makePReadArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_DATA_READ] = &v_args[0];
  args_map[SYSCALL_FIELD_BYTES_REQUESTED] = &args[2];
  args_map[SYSCALL_FIELD_OFFSET] = &args[3];
}

void DataSeriesOutputModule::makePWriteArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_DATA_WRITTEN] = &v_args[0];
  args_map[SYSCALL_FIELD_BYTES_REQUESTED] = &args[2];
  args_map[SYSCALL_FIELD_OFFSET] = &args[3];
}

void DataSeriesOutputModule::makeSetpgidArgsMap(void **args_map,
						long *args,
						void **v_args) {
  args_map[SYSCALL_FIELD_PID] = &args[0];
  args_map[SYSCALL_FIELD_PGID] = &args[1];
}

void DataSeriesOutputModule::makeSetrlimitArgsMap(void **args_map,
						  long *args,
						  void **v_args) {
  args_map[SYSCALL_FIELD_RESOURCE_VALUE] = &args[0];
  /*
   * TODO: The correct value of args_map["resource"] should be 0 if resource is
   * RLIMIT_AS, 1 if it is RLIMIT_CORE, 2 if it is RLIMIT_CPU, so and so forth.
   * Currently, we don't do this. We simply assume that resource is same
   * across different platforms.
   */
  args_map[SYSCALL_FIELD_RESOURCE] = &args[0];
  if (v_args[0] != NULL) {
    struct rlimit *rlim = (struct rlimit *) v_args[0];
    args_map[SYSCALL_FIELD_RESOURCE_SOFT_LIMIT] = &rlim->rlim_cur;
    args_map[SYSCALL_FIELD_RESOURCE_HARD_LIMIT] = &rlim->rlim_max;
  } else {
    std::cerr << "Setrlimit: Struct rlimit is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeSetsidArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  // Takes no arguments
}

void DataSeriesOutputModule::makeStatArgsMap(void **args_map,
					     long *args,
					     void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Stat: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    struct stat *statbuf = (struct stat *) v_args[1];

    args_map[SYSCALL_FIELD_STAT_RESULT_DEV] = &statbuf->st_dev;
    args_map[SYSCALL_FIELD_STAT_RESULT_INO] = &statbuf->st_ino;
    args_map[SYSCALL_FIELD_STAT_RESULT_MODE] = &statbuf->st_mode;
    args_map[SYSCALL_FIELD_STAT_RESULT_NLINK] = &statbuf->st_nlink;
    args_map[SYSCALL_FIELD_STAT_RESULT_UID] = &statbuf->st_uid;
    args_map[SYSCALL_FIELD_STAT_RESULT_GID] = &statbuf->st_gid;
    args_map[SYSCALL_FIELD_STAT_RESULT_RDEV] = &statbuf->st_rdev;
    args_map[SYSCALL_FIELD_STAT_RESULT_SIZE] = &statbuf->st_size;
    args_map[SYSCALL_FIELD_STAT_RESULT_BLKSIZE] = &statbuf->st_blksize;
    args_map[SYSCALL_FIELD_STAT_RESULT_BLOCKS] = &statbuf->st_blocks;

    /*
     * Convert stat_result_atime, stat_result_mtime and
     * stat_result_ctime to Tfracs.
     */
    static uint64_t atime_Tfrac = timespec_to_Tfrac(statbuf->st_atim);
    static uint64_t mtime_Tfrac = timespec_to_Tfrac(statbuf->st_mtim);
    static uint64_t ctime_Tfrac = timespec_to_Tfrac(statbuf->st_ctim);
    args_map[SYSCALL_FIELD_STAT_RESULT_ATIME] = &atime_Tfrac;
    args_map[SYSCALL_FIELD_STAT_RESULT_MTIME] = &mtime_Tfrac;
    args_map[SYSCALL_FIELD_STAT_RESULT_CTIME] = &ctime_Tfrac;
  } else {
    std::cerr << "Stat: Struct stat buffer is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeStatfsArgsMap(void **args_map,
                                              long *args,
                                              void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "statfs");

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Statfs: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    struct statfs *statfsbuf = (struct statfs *) v_args[1];

    args_map[SYSCALL_FIELD_STATFS_RESULT_TYPE] = &statfsbuf->f_type;
    args_map[SYSCALL_FIELD_STATFS_RESULT_BSIZE] = &statfsbuf->f_bsize;
    args_map[SYSCALL_FIELD_STATFS_RESULT_BLOCKS] = &statfsbuf->f_blocks;
    args_map[SYSCALL_FIELD_STATFS_RESULT_BFREE] = &statfsbuf->f_bfree;
    args_map[SYSCALL_FIELD_STATFS_RESULT_BAVAIL] = &statfsbuf->f_bavail;
    args_map[SYSCALL_FIELD_STATFS_RESULT_FILES] = &statfsbuf->f_files;
    args_map[SYSCALL_FIELD_STATFS_RESULT_FFREE] = &statfsbuf->f_ffree;
    args_map[SYSCALL_FIELD_STATFS_RESULT_FSID] = &statfsbuf->f_fsid;
    args_map[SYSCALL_FIELD_STATFS_RESULT_NAMELEN] = &statfsbuf->f_namelen;
    args_map[SYSCALL_FIELD_STATFS_RESULT_FRSIZE] = &statfsbuf->f_frsize;
    args_map[SYSCALL_FIELD_STATFS_RESULT_FLAGS] = &statfsbuf->f_flags;

    u_int flag = processStatfsFlags(args_map, statfsbuf->f_flags);
    if (flag != 0) {
      std::cerr << "Statfs: These flag are not processed/unknown->0x";
      std::cerr << std::hex << flag << std::dec << std::endl;
    }
  } else {
    std::cerr << "Statfs: Struct statfs is set as NULL!!" << std::endl;
  }
}


void DataSeriesOutputModule::makeFStatfsArgsMap(void **args_map,
						long *args,
						void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "fstatfs");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  if (v_args[0] != NULL) {
    struct statfs *statfsbuf = (struct statfs *) v_args[0];

    args_map[SYSCALL_FIELD_STATFS_RESULT_TYPE] = &statfsbuf->f_type;
    args_map[SYSCALL_FIELD_STATFS_RESULT_BSIZE] = &statfsbuf->f_bsize;
    args_map[SYSCALL_FIELD_STATFS_RESULT_BLOCKS] = &statfsbuf->f_blocks;
    args_map[SYSCALL_FIELD_STATFS_RESULT_BFREE] = &statfsbuf->f_bfree;
    args_map[SYSCALL_FIELD_STATFS_RESULT_BAVAIL] = &statfsbuf->f_bavail;
    args_map[SYSCALL_FIELD_STATFS_RESULT_FILES] = &statfsbuf->f_files;
    args_map[SYSCALL_FIELD_STATFS_RESULT_FFREE] = &statfsbuf->f_ffree;
    args_map[SYSCALL_FIELD_STATFS_RESULT_FSID] = &statfsbuf->f_fsid;
    args_map[SYSCALL_FIELD_STATFS_RESULT_NAMELEN] = &statfsbuf->f_namelen;
    args_map[SYSCALL_FIELD_STATFS_RESULT_FRSIZE] = &statfsbuf->f_frsize;
    args_map[SYSCALL_FIELD_STATFS_RESULT_FLAGS] = &statfsbuf->f_flags;

    u_int flag = processStatfsFlags(args_map, statfsbuf->f_flags);
    if (flag != 0) {
      std::cerr << "FStatfs: These flags are not processed/unknown->0x";
      std::cerr << std::hex << flag << std::dec << std::endl;
    }
  } else {
    std::cerr << "FStatfs: Struct statfs is set as NULL!!" << std::endl;
  }
}

u_int DataSeriesOutputModule::processStatfsFlags(void **args_map,
                                                u_int statfs_flags) {
  /*
   * Process each individual statfs flag bit that has been set
   * in the argument stafs_flags.
   */
  // set mandatory lock flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_MANDLOCK,
                            SYSCALL_FIELD_STATFS_RESULT_FLAGS_MANDATORY_LOCK);
  // set no access time flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NOATIME,
                            SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_ACCESS_TIME);
  // set no dev flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NODEV,
                            SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_DEV);
  // set no directory access time flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NODIRATIME,
                            SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_DIRECTORY_ACCESS_TIME);
  // set no exec flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NOEXEC,
                            SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_EXEC);
  // set no set uid flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NOSUID,
                            SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_SET_UID);
  // set read only flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_RDONLY,
                            SYSCALL_FIELD_STATFS_RESULT_FLAGS_READ_ONLY);
  // set relative access time flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_RELATIME,
                            SYSCALL_FIELD_STATFS_RESULT_FLAGS_RELATIVE_ACCESS_TIME);
  // set synchronous flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_SYNCHRONOUS,
                            SYSCALL_FIELD_STATFS_RESULT_FLAGS_SYNCHRONOUS);
  // set valid flag (f_flags support is implemented)
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_VALID,
           SYSCALL_FIELD_STATFS_RESULT_FLAGS_VALID);

  /*
   * Return remaining statfs flags so that caller can
   * warn of unknown flags if the statfs_flags is not set
   * as zero.
   */
  return statfs_flags;
}

void DataSeriesOutputModule::makeFTruncateArgsMap(void **args_map,
						  long *args,
						  void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_TRUNCATE_LENGTH] = &args[1];
}

void DataSeriesOutputModule::makeChownArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Chown: Pathname is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_NEW_OWNER] = &args[1];
  args_map[SYSCALL_FIELD_NEW_GROUP] = &args[2];
}

void DataSeriesOutputModule::makeReadlinkArgsMap(void **args_map,
						 long *args,
						 void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Readlink: Pathname is set as NULL!!" << std::endl;
  }
  args_map[SYSCALL_FIELD_LINK_VALUE] = &v_args[1];
  args_map[SYSCALL_FIELD_BUFFER_SIZE] = &args[2];
}

void DataSeriesOutputModule::makeReadvArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  int iov_number = *(int *) v_args[0];

  /*
   * iov_number equal to '-1' denotes the first record for the
   * readv system call. For first record, we save the file
   * descriptor, count, iov_number and total number of bytes
   * requested and do not set the data_read field.
   */
  if (iov_number == -1) {
    args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
    args_map[SYSCALL_FIELD_COUNT] = &args[2];
    args_map[SYSCALL_FIELD_IOV_NUMBER] = v_args[0];
    args_map[SYSCALL_FIELD_BYTES_REQUESTED] = v_args[1];
  } else {
    /*
     * For rest of the records, we do not save file descriptor and
     * count fields. We only save the iov_number, bytes_requested
     * and data_read.
     */
    args_map[SYSCALL_FIELD_IOV_NUMBER] = v_args[0];
    args_map[SYSCALL_FIELD_BYTES_REQUESTED] = v_args[1];
    args_map[SYSCALL_FIELD_DATA_READ] = &v_args[2];
  }
}

void DataSeriesOutputModule::makeWritevArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  int iov_number = *(int *) v_args[0];

  /*
   * iov_number equal to '-1' denotes the first record for the
   * writev system call. For first record, we save the file
   * descriptor, count, iov_number and total number of bytes
   * requested and do not set the data_written field.
   */
  if (iov_number == -1) {
    args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
    args_map[SYSCALL_FIELD_COUNT] = &args[2];
    args_map[SYSCALL_FIELD_IOV_NUMBER] = v_args[0];
    args_map[SYSCALL_FIELD_BYTES_REQUESTED] = v_args[1];
  } else {
    /*
     * For rest of the records, we do not save file descriptor and
     * count fields. We only save the iov_number, bytes_requested
     * and data_written fields.
     */
    args_map[SYSCALL_FIELD_IOV_NUMBER] = v_args[0];
    args_map[SYSCALL_FIELD_BYTES_REQUESTED] = v_args[1];
    args_map[SYSCALL_FIELD_DATA_WRITTEN] = &v_args[2];
  }
}

void DataSeriesOutputModule::makeUtimeArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  static uint64_t access_time_Tfrac;
  static uint64_t mod_time_Tfrac;

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Utime: Pathname is set as NULL!!" << std::endl;
  }

  // If the utimbuf is not NULL, set the corresponding values in the map
  if (v_args[1] != NULL) {
    struct utimbuf *times = (struct utimbuf *) v_args[1];

    // Convert the time_t members of the struct utimbuf to Tfracs (uint64_t)
    access_time_Tfrac = sec_to_Tfrac(times->actime);
    mod_time_Tfrac = sec_to_Tfrac(times->modtime);

    args_map[SYSCALL_FIELD_ACCESS_TIME] = &access_time_Tfrac;
    args_map[SYSCALL_FIELD_MOD_TIME] = &mod_time_Tfrac;
  }
}

void DataSeriesOutputModule::makeLStatArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "LStat: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    struct stat *statbuf = (struct stat *) v_args[1];

    args_map[SYSCALL_FIELD_STAT_RESULT_DEV] = &statbuf->st_dev;
    args_map[SYSCALL_FIELD_STAT_RESULT_INO] = &statbuf->st_ino;
    args_map[SYSCALL_FIELD_STAT_RESULT_MODE] = &statbuf->st_mode;
    args_map[SYSCALL_FIELD_STAT_RESULT_NLINK] = &statbuf->st_nlink;
    args_map[SYSCALL_FIELD_STAT_RESULT_UID] = &statbuf->st_uid;
    args_map[SYSCALL_FIELD_STAT_RESULT_GID] = &statbuf->st_gid;
    args_map[SYSCALL_FIELD_STAT_RESULT_RDEV] = &statbuf->st_rdev;
    args_map[SYSCALL_FIELD_STAT_RESULT_SIZE] = &statbuf->st_size;
    args_map[SYSCALL_FIELD_STAT_RESULT_BLKSIZE] = &statbuf->st_blksize;
    args_map[SYSCALL_FIELD_STAT_RESULT_BLOCKS] = &statbuf->st_blocks;

    /*
     * Convert stat_result_atime, stat_result_mtime and
     * stat_result_ctime to Tfracs.
     */
    static uint64_t atime_Tfrac = timespec_to_Tfrac(statbuf->st_atim);
    static uint64_t mtime_Tfrac = timespec_to_Tfrac(statbuf->st_mtim);
    static uint64_t ctime_Tfrac = timespec_to_Tfrac(statbuf->st_ctim);
    args_map[SYSCALL_FIELD_STAT_RESULT_ATIME] = &atime_Tfrac;
    args_map[SYSCALL_FIELD_STAT_RESULT_MTIME] = &mtime_Tfrac;
    args_map[SYSCALL_FIELD_STAT_RESULT_CTIME] = &ctime_Tfrac;
  } else {
    std::cerr << "LStat: Struct stat buffer is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeFStatArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  if (v_args[0] != NULL) {
    struct stat *statbuf = (struct stat *) v_args[0];

    args_map[SYSCALL_FIELD_STAT_RESULT_DEV] = &statbuf->st_dev;
    args_map[SYSCALL_FIELD_STAT_RESULT_INO] = &statbuf->st_ino;
    args_map[SYSCALL_FIELD_STAT_RESULT_MODE] = &statbuf->st_mode;
    args_map[SYSCALL_FIELD_STAT_RESULT_NLINK] = &statbuf->st_nlink;
    args_map[SYSCALL_FIELD_STAT_RESULT_UID] = &statbuf->st_uid;
    args_map[SYSCALL_FIELD_STAT_RESULT_GID] = &statbuf->st_gid;
    args_map[SYSCALL_FIELD_STAT_RESULT_RDEV] = &statbuf->st_rdev;
    args_map[SYSCALL_FIELD_STAT_RESULT_SIZE] = &statbuf->st_size;
    args_map[SYSCALL_FIELD_STAT_RESULT_BLKSIZE] = &statbuf->st_blksize;
    args_map[SYSCALL_FIELD_STAT_RESULT_BLOCKS] = &statbuf->st_blocks;

    /*
     * Convert stat_result_atime, stat_result_mtime and
     * stat_result_ctime to Tfracs.
     */
    static uint64_t atime_Tfrac = timespec_to_Tfrac(statbuf->st_atim);
    static uint64_t mtime_Tfrac = timespec_to_Tfrac(statbuf->st_mtim);
    static uint64_t ctime_Tfrac = timespec_to_Tfrac(statbuf->st_ctim);
    args_map[SYSCALL_FIELD_STAT_RESULT_ATIME] = &atime_Tfrac;
    args_map[SYSCALL_FIELD_STAT_RESULT_MTIME] = &mtime_Tfrac;
    args_map[SYSCALL_FIELD_STAT_RESULT_CTIME] = &ctime_Tfrac;
  } else {
    std::cerr << "FStat: Struct stat buffer is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeFStatatArgsMap(void **args_map,
						long *args,
						void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "fstatat");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "FStatat: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    struct stat *statbuf = (struct stat *) v_args[1];

    args_map[SYSCALL_FIELD_STAT_RESULT_DEV] = &statbuf->st_dev;
    args_map[SYSCALL_FIELD_STAT_RESULT_INO] = &statbuf->st_ino;
    args_map[SYSCALL_FIELD_STAT_RESULT_MODE] = &statbuf->st_mode;
    args_map[SYSCALL_FIELD_STAT_RESULT_NLINK] = &statbuf->st_nlink;
    args_map[SYSCALL_FIELD_STAT_RESULT_UID] = &statbuf->st_uid;
    args_map[SYSCALL_FIELD_STAT_RESULT_GID] = &statbuf->st_gid;
    args_map[SYSCALL_FIELD_STAT_RESULT_RDEV] = &statbuf->st_rdev;
    args_map[SYSCALL_FIELD_STAT_RESULT_SIZE] = &statbuf->st_size;
    args_map[SYSCALL_FIELD_STAT_RESULT_BLKSIZE] = &statbuf->st_blksize;
    args_map[SYSCALL_FIELD_STAT_RESULT_BLOCKS] = &statbuf->st_blocks;

    /*
     * Convert stat_result_atime, stat_result_mtime and
     * stat_result_ctime to Tfracs.
     */
    static uint64_t atime_Tfrac = timespec_to_Tfrac(statbuf->st_atim);
    static uint64_t mtime_Tfrac = timespec_to_Tfrac(statbuf->st_mtim);
    static uint64_t ctime_Tfrac = timespec_to_Tfrac(statbuf->st_ctim);
    args_map[SYSCALL_FIELD_STAT_RESULT_ATIME] = &atime_Tfrac;
    args_map[SYSCALL_FIELD_STAT_RESULT_MTIME] = &mtime_Tfrac;
    args_map[SYSCALL_FIELD_STAT_RESULT_CTIME] = &ctime_Tfrac;
  } else {
    std::cerr << "FStatat: Struct stat buffer is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_FLAGS_VALUE] = &args[3];

  u_int flag = processFStatatFlags(args_map, args[3]);
  if (flag != 0) {
    std::cerr << "FStatat: These flags are not processed/unknown->"
	      << std::hex << flag << std::dec << std::endl;
  }
}

u_int DataSeriesOutputModule::processFStatatFlags(void **args_map,
						  u_int fstatat_flags) {
  /*
   * Process each individual statfs flag bit that has been set
   * in the argument fstatat_flags.
   */
  // set at empty path flag
  process_Flag_and_Mode_Args(args_map, fstatat_flags, AT_EMPTY_PATH,
			     SYSCALL_FIELD_FLAGS_AT_EMPTY_PATH);
  // set no auto mount flag
  process_Flag_and_Mode_Args(args_map, fstatat_flags, AT_NO_AUTOMOUNT,
			     SYSCALL_FIELD_FLAGS_AT_NO_AUTOMOUNT);
  // set symlink nofollow flag
  process_Flag_and_Mode_Args(args_map, fstatat_flags, AT_SYMLINK_NOFOLLOW,
			     SYSCALL_FIELD_FLAGS_AT_SYMLINK_NOFOLLOW);

  /*
   * Return remaining fstatat flags so that caller can
   * warn of unknown flags if the fstatat_flags is not set
   * as zero.
   */
  return fstatat_flags;
}

void DataSeriesOutputModule::makeUtimesArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  static uint64_t access_time_Tfrac;
  static uint64_t mod_time_Tfrac;

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Utimes: Pathname is set as NULL!!" << std::endl;
  }

  // If the timeval array is not NULL, set the corresponding values in the map
  if (v_args[1] != NULL) {
    struct timeval *tv = (struct timeval *) v_args[1];

    // Convert timeval arguments to Tfracs (uint64_t)
    access_time_Tfrac = timeval_to_Tfrac(tv[0]);
    mod_time_Tfrac = timeval_to_Tfrac(tv[1]);

    args_map[SYSCALL_FIELD_ACCESS_TIME] = &access_time_Tfrac;
    args_map[SYSCALL_FIELD_MOD_TIME] = &mod_time_Tfrac;
  }
}

void DataSeriesOutputModule::makeUtimensatArgsMap(void **args_map,
						  long *args,
						  void **v_args) {
  static uint64_t access_time_Tfrac;
  static uint64_t mod_time_Tfrac;
  initArgsMap(args_map, "utimensat");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
  }

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Utimensat: Pathname is set as NULL!!" << std::endl;
  }

  // If the timespec array is not NULL, set the corresponding values in the map
  if (v_args[1] != NULL) {
    struct timespec *ts = (struct timespec *) v_args[1];

    // Check for the special values UTIME_NOW and UTIME_OMIT
    if ((ts[0].tv_nsec == UTIME_NOW) || (ts[1].tv_nsec == UTIME_NOW))
      args_map[SYSCALL_FIELD_UTIME_NOW] = &true_;
    if ((ts[0].tv_nsec == UTIME_OMIT) || (ts[1].tv_nsec == UTIME_OMIT))
      args_map[SYSCALL_FIELD_UTIME_OMIT] = &true_;

    // Convert timespec arguments to Tfracs (uint64_t)
    access_time_Tfrac = timespec_to_Tfrac(ts[0]);
    mod_time_Tfrac = timespec_to_Tfrac(ts[1]);

    args_map[SYSCALL_FIELD_ACCESS_TIME] = &access_time_Tfrac;
    args_map[SYSCALL_FIELD_MOD_TIME] = &mod_time_Tfrac;
  }

  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[3];
  u_int flag = args[3];
  process_Flag_and_Mode_Args(args_map, flag, AT_SYMLINK_NOFOLLOW,
			     SYSCALL_FIELD_FLAG_SYMLINK_NOFOLLOW);
  if (flag != 0) {
    std::cerr << "Utimensat: These flags are not processed/unknown->"
	      << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeRenameArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_OLDNAME] = &v_args[0];
  } else {
    std::cerr << "Rename: Old name is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_NEWNAME] = &v_args[1];
  } else {
    std::cerr << "Rename: New name is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeFsyncArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
}

void DataSeriesOutputModule::makeMknodArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  static int32_t dev;
  initArgsMap(args_map, "mknod");
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Mknod: Pathname is set as NULL!!" << std::endl;
  }

  mode_t mode = processMode(args_map, args, 1);

  mode = processMknodType(args_map, mode);

  if (mode != 0) {
    std::cerr << "Mknod: These modes are not processed/unknown: ";
    std::cerr << std::oct << mode << std::dec << ". " << std::endl;
  }

  if ((args[1] & S_IFCHR) || (args[1] & S_IFBLK)) {
    dev = (int32_t) args[2];
    args_map[SYSCALL_FIELD_DEV] = &dev;
  }
}

void DataSeriesOutputModule::makeMknodatArgsMap(void **args_map,
						long *args,
						void **v_args) {
  static int32_t dev;
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  initArgsMap(args_map, "mknodat");
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Mknodat: Pathname is set as NULL!!" << std::endl;
  }

  mode_t mode = processMode(args_map, args, 2);

  /* Can reuse processMknodType since the mode values are same */
  mode = processMknodType(args_map, mode);

  if (mode != 0) {
    std::cerr << "Mknodat: These modes are not processed/unknown: ";
    std::cerr << std::oct << mode << std::dec << ". " << std::endl;
  }

  if ((args[2] & S_IFCHR) || (args[2] & S_IFBLK)) {
    dev = (int32_t) args[3];
    args_map[SYSCALL_FIELD_DEV] = &dev;
  }
}

mode_t DataSeriesOutputModule::processMknodType(void **args_map,
						mode_t mode) {
  static u_int type;

  /*
   * Check for each file type.  If the mode is equal to the value for that
   * file type, set type to the encoding value specified by SNIA:
   * Regular = 0
   * Character special = 1
   * Block special = 2
   * FIFO = 3
   * Socket = 4
   */
  if ((S_ISREG(mode)) || (mode == 0)) {
    type = DS_FILE_TYPE_REG;
    mode &= ~S_IFREG;
  } else if (S_ISCHR(mode)) {
    type = DS_FILE_TYPE_CHR;
    mode &= ~S_IFCHR;
  } else if (S_ISBLK(mode)) {
    type = DS_FILE_TYPE_BLK;
    mode &= ~S_IFBLK;
  } else if (S_ISFIFO(mode)) {
    type = DS_FILE_TYPE_FIFO;
    mode &= ~S_IFIFO;
  } else if (S_ISSOCK(mode)) {
    type = DS_FILE_TYPE_SOCK;
    mode &= ~S_IFSOCK;
  }
  args_map[SYSCALL_FIELD_TYPE] = &type;

  /*
   * Return remaining unprocessed modes so that caller can warn
   * of unknown modes if the mode value is not set as zero.
   */
  return mode;
}

void DataSeriesOutputModule::makePipeArgsMap(void **args_map,
					     long *args,
					     void **v_args) {
  static int pipefd[2];

  if (v_args[0] != NULL) {
    pipefd[0] = ((int *) v_args[0])[0];
    pipefd[1] = ((int *) v_args[0])[1];
  } else {
    /*
     * In the case of a NULL file descriptor array, set
     * read_descriptor and write_descriptor equal to 0.
     */
    pipefd[0] = 0;
    pipefd[1] = 0;
    std::cerr << "Pipe: File descriptor array is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_READ_DESCRIPTOR] = &pipefd[0];
  args_map[SYSCALL_FIELD_WRITE_DESCRIPTOR] = &pipefd[1];
}

void DataSeriesOutputModule::makeDupArgsMap(void **args_map,
					    long *args,
					    void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
}

void DataSeriesOutputModule::makeDup2ArgsMap(void **args_map,
					     long *args,
					     void **v_args) {
  args_map[SYSCALL_FIELD_OLD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_NEW_DESCRIPTOR] = &args[1];
}

void DataSeriesOutputModule::makeFcntlArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  // Set all non-nullable boolean fields to false
  initArgsMap(args_map, "fcntl");

  // Save the descriptor and command value to the map
  args_map["descriptor"] = &args[0];
  args_map["command_value"] = &args[1];

  int command = args[1];
  /*
   * Check the command argument passed to fcntl and set the corresponding
   * fields in the map
   */
  switch (command) {

  // File descriptor dup command
  case F_DUPFD:
    args_map["command_dup"] = &true_;
    args_map["argument_value"] = &args[2];
    break;

  // Get file descriptor flags command
  case F_GETFD:
    args_map["command_get_descriptor_flags"] = &true_;
    break;

  // Set file descriptor flags command
  case F_SETFD: {
    args_map["command_set_descriptor_flags"] = &true_;
    args_map["argument_value"] = &args[2];
    u_int fd_flag = (u_int) args[2];
    process_Flag_and_Mode_Args(args_map, fd_flag, FD_CLOEXEC,
			       "argument_descriptor_flag_exec_close");
    if (fd_flag != 0) {
      std::cerr << "Fcntl: SETFD: These flags are not processed/unknown->0x"
		<< std::hex << fd_flag << std::dec << std::endl;
    }
    break;
  }
  // Get file status flags command
  case F_GETFL:
    args_map["command_get_status_flags"] = &true_;
    break;

  // Set file status flags command
  case F_SETFL: {
    args_map["command_set_status_flags"] = &true_;
    args_map["argument_value"] = &args[2];
    u_int status_flag = processFcntlStatusFlags(args_map, args[2]);
    if (status_flag != 0) {
      std::cerr << "Fcntl: SETFL: These flags are not processed/unknown->0x"
		<< std::hex << status_flag << std::dec << std::endl;
    }
    break;
  }
  // Set lock command
  case F_SETLK:
    args_map["command_set_lock"] = &true_;
    processFcntlFlock(args_map, (struct flock *) v_args[0]);
    break;

  // Set lock wait command
  case F_SETLKW:
    args_map["command_set_lock_wait"] = &true_;
    processFcntlFlock(args_map, (struct flock *) v_args[0]);
    break;

  // Get lock command
  case F_GETLK:
    args_map["command_get_lock"] = &true_;
    processFcntlFlock(args_map, (struct flock *) v_args[0]);
    break;

  // Get process id command
  case F_GETOWN:
    args_map["command_get_process_id"] = &true_;
    break;

  // Set process id command
  case F_SETOWN:
    args_map["command_set_process_id"] = &true_;
    args_map["argument_value"] = &args[2];
    break;

  // Get signal command
  case F_GETSIG:
    args_map["command_get_signal"] = &true_;
    break;

  // Set signal command
  case F_SETSIG:
    args_map["command_set_signal"] = &true_;
    args_map["argument_value"] = &args[2];
    break;

  // Get lease command
  case F_GETLEASE: {
    args_map["command_get_lease"] = &true_;
    int return_value = *(int *) args_map["return_value"];
    processFcntlLease(args_map, return_value);
    break;
  }
  // Set lease command
  case F_SETLEASE:
    args_map["command_set_lease"] = &true_;
    args_map["argument_value"] = &args[2];
    processFcntlLease(args_map, args[2]);
    break;

  // Notify command
  case F_NOTIFY: {
    args_map["command_notify"] = &true_;
    args_map["argument_value"] = &args[2];
    u_int notify_value = processFcntlNotify(args_map, args);
    if (notify_value != 0) {
      std::cerr << "Fcntl: F_NOTIFY: These flags are not processed/unknown->"
		<< std::hex << notify_value << std::dec << std::endl;
    }
    break;
  }
  /*
   * If the command value doesn't match a known command, print
   * a warning message
   */
  default:
    std::cerr << "Fcntl: Command is unknown->" << command << std::endl;
    args_map["argument_value"] = &args[2];
  }
}

/*
 * This function unwraps the flag value passed as an argument to the
 * fcntl system call with the F_SETFL command and sets the corresponding
 * flag values as True.
 *
 * @param args_map: stores mapping of <field, value> pairs.
 *
 * @param status_flag: represents the flag value passed as an argument
 *                   to the fcntl system call.
 */
u_int DataSeriesOutputModule::processFcntlStatusFlags(SysCallArgsMap &args_map,
						      u_int status_flag) {

  /*
   * Process each individual flag bit that has been set
   * in the argument status_flag.
   */
  // set read only flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_RDONLY,
			     "argument_status_flag_read_only");
  // set write only flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_WRONLY,
			     "argument_status_flag_write_only");
  // set read and write flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_RDWR,
			     "argument_status_flag_read_and_write");
  // set append flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_APPEND,
			     "argument_status_flag_append");
  // set async flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_ASYNC,
			     "argument_status_flag_async");
  // set create flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_CREAT,
			     "argument_status_flag_create");
  // set direct flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_DIRECT,
			     "argument_status_flag_direct");
  // set directory flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_DIRECTORY,
			     "argument_status_flag_directory");
  // set exclusive flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_EXCL,
			     "argument_status_flag_exclusive");
  // set largefile flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_LARGEFILE,
			     "argument_status_flag_largefile");
  // set last access time flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_NOATIME,
			     "argument_status_flag_no_access_time");
  // set controlling terminal flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_NOCTTY,
			     "argument_status_flag_no_controlling_terminal");
  // set no_follow flag (in case of symbolic link)
  process_Flag_and_Mode_Args(args_map, status_flag, O_NOFOLLOW,
			     "argument_status_flag_no_follow");
  // set non blocking mode flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_NONBLOCK,
			     "argument_status_flag_no_blocking_mode");
  // set no delay flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_NDELAY,
			     "argument_status_flag_no_delay");
  // set synchronized IO flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_SYNC,
			     "argument_status_flag_synchronous");
  // set truncate mode flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_TRUNC,
			     "argument_status_flag_truncate");

  /*
   * Return remaining unprocessed flags so that caller can
   * warn of unknown flags if the status_flag value is not set
   * as zero.
   */
  return status_flag;
}

/*
 * This function saves the values in the flock structure passed to
 * Fcntl with command F_SETLK, F_SETLKW, or F_GETLK into the map
 */
void DataSeriesOutputModule::processFcntlFlock(SysCallArgsMap &args_map,
					       struct flock *lock) {
  if (lock != NULL) {
    // Save the values in the flock structure to the map
    processFcntlFlockType(args_map, lock);
    processFcntlFlockWhence(args_map, lock);
    args_map["lock_start"] = &lock->l_start;
    args_map["lock_length"] = &lock->l_len;
    args_map["lock_pid"] = &lock->l_pid;
  } else {
    /*
     * If the flock passed to Fcntl was NULL, then print a warning message.
     * The int32 fields lock_type, lock_whence, lock_start, lock_length,
     * and lock_pid will be set to 0 by default.
     */
    std::cerr << "Flock: Struct flock is set as NULL!!" << std::endl;
  }
}

/*
 * This function processes the l_type member of an flock structure
 * and sets the corresponding field in the map
 */
void DataSeriesOutputModule::processFcntlFlockType(SysCallArgsMap &args_map,
						   struct flock *lock) {
  // Save the lock type value into the map
  args_map["lock_type"] = &lock->l_type;
  u_int type = lock->l_type;

  /*
   * If the type value matches one of the possible types, set the
   * corresponding field in the map to True
   */
  switch (type) {
  // set read lock field
  case F_RDLCK:
    args_map["lock_type_read"] = &true_;
    break;
  // set write lock field
  case F_WRLCK:
    args_map["lock_type_write"] = &true_;
    break;
  // set unlocked field
  case F_UNLCK:
    args_map["lock_type_unlocked"] = &true_;
    break;
  // If the type value isn't a known type, print a warning message
  default:
    std::cerr << "Fcntl: Lock type is unknown->" << lock << std::endl;
  }
}

/*
 * This function processes the l_whence member of an flock structure
 * and sets the corresponding field in the map
 */
void DataSeriesOutputModule::processFcntlFlockWhence(SysCallArgsMap &args_map,
						     struct flock *lock) {
  // Save the lock whence value into the map
  args_map["lock_whence"] = &lock->l_whence;
  u_int whence = lock->l_whence;

  /*
   * If the whence value matches one of the possible values, set the
   * corresponding field in the map to True
   */
  switch (whence) {
  // set SEEK_SET whence field
  case SEEK_SET:
    args_map["lock_whence_start"] = &true_;
    break;
  // set SEEK_CUR whence field
  case SEEK_CUR:
    args_map["lock_whence_current"] = &true_;
    break;
  // set SEEK_END whence field
  case SEEK_END:
    args_map["lock_whence_end"] = &true_;
    break;
  // If the whence value isn't a known whence value, print a warning message
  default:
    std:: cerr << "Fcntl: Lock whence is unknown->" << whence << std::endl;
  }
}

/*
 * This function processes the lease value passed to an Fcntl system call with
 * an F_SETLEASE command or returned from an F_GETLEASE command
 */
void DataSeriesOutputModule::processFcntlLease(SysCallArgsMap &args_map,
					       int lease) {
  /*
   * If the lease argument matches one of the possible values, set the
   * corresponding field in the map to True
   */
  switch (lease) {
  // set read lock lease field
  case F_RDLCK:
    args_map["argument_lease_read"] = &true_;
    break;
  // set write lock lease field
  case F_WRLCK:
    args_map["argument_lease_write"] = &true_;
    break;
  // set unlocked lease field
  case F_UNLCK:
    args_map["argument_lease_remove"] = &true_;
    break;
  // If the lease argument isn't a known lease, print a warning message
  default:
    std::cerr << "Fcntl: Lease argument is unknown->" << lease << std::endl;
  }
}

/*
 * This function processes the notify value passed to an Fcntl system call
 * with an F_NOTIFY command.  It returns any unprocessed notify_value bits.
 */
u_int DataSeriesOutputModule::processFcntlNotify(SysCallArgsMap &args_map,
						 long *args) {
  u_int notify_value = args[2];

  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_ACCESS,
			     "argument_notify_access");
  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_MODIFY,
			     "argument_notify_modify");
  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_CREATE,
			     "argument_notify_create");
  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_DELETE,
			     "argument_notify_delete");
  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_RENAME,
			     "argument_notify_rename");
  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_ATTRIB,
			     "argument_notify_attribute");

  /*
   * Return remaining notify flags so that caller can
   * warn of unknown flags if the notify_value is not set
   * as zero.
   */
  return notify_value;
}

void DataSeriesOutputModule::makeExitArgsMap(void **args_map,
					     long *args,
					     void **v_args) {
  args_map[SYSCALL_FIELD_EXIT_STATUS] = &args[0];
  args_map[SYSCALL_FIELD_GENERATED] = v_args[0];
}

void DataSeriesOutputModule::makeExecveArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  int continuation_number = *(int *) v_args[0];
  args_map[SYSCALL_FIELD_CONTINUATION_NUMBER] = v_args[0];

  /*
   * Continuation number equal to '0' denotes the first record of
   * single execve system call. For first record, we only save the
   * continuation number and given pathname fields.
   */
  if (continuation_number == 0) {
    if (v_args[1] != NULL)
      args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[1];
    else
      std::cerr << "Execve: Pathname is set as NULL!!" << std::endl;
  } else if (continuation_number > 0) {
    /*
     * If continuation number is greater than '0', then add
     * record to set the argument or environment variables.
     */
    char *arg_env = (char *) v_args[2];
    if (strcmp(arg_env, "arg") == 0) {
      /*
       * If arg_env is equal to "arg", then we only save the
       * continuation number and argument fields in the new
       * record.
       */
      if (v_args[1] != NULL)
	args_map[SYSCALL_FIELD_ARGUMENT] = &v_args[1];
      else
	std::cerr << "Execve: Argument is set as NULL!!" << std::endl;
    } else if (strcmp(arg_env, "env") == 0) {
      /*
       * If arg_env is equal to "env", then we only save the
       * continuation number and environment fields in the
       * new record.
       */
      if (v_args[1] != NULL)
	args_map[SYSCALL_FIELD_ENVIRONMENT] = &v_args[1];
      else
	std::cerr << "Execve : Environment is set as NULL!!" << std::endl;
    }
  }
}

void DataSeriesOutputModule::makeMmapArgsMap(void **args_map,
					     long *args,
					     void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "mmap");

  args_map[SYSCALL_FIELD_START_ADDRESS] = &args[0];
  args_map[SYSCALL_FIELD_LENGTH] = &args[1];

  args_map[SYSCALL_FIELD_PROTECTION_VALUE] = &args[2];
  // Set individual mmap protection bits
  u_int prot_flags = processMmapProtectionArgs(args_map, args[2]);
  if (prot_flags != 0) {
    std::cerr << "Mmap: These protection flags are not processed/unknown->0x";
    std::cerr << std::hex << prot_flags << std::dec << std::endl;
  }

  args_map[SYSCALL_FIELD_FLAGS_VALUE] = &args[3];
  // Set individual mmap flag bits
  u_int flag = processMmapFlags(args_map, args[3]);
  if (flag != 0) {
    std::cerr << "Mmap: These flag are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[4];
  args_map[SYSCALL_FIELD_OFFSET] = &args[5];
}

u_int DataSeriesOutputModule::processMmapProtectionArgs(void **args_map,
							u_int mmap_prot_flags) {
  /*
   * Process each individual mmap protection bit that has been set
   * in the argument mmap_prot_flags.
   */
  // set exec protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_EXEC,
			     SYSCALL_FIELD_PROTECTION_EXEC);
  // set read protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_READ,
			     SYSCALL_FIELD_PROTECTION_READ);
  // set write protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_WRITE,
			     SYSCALL_FIELD_PROTECTION_WRITE);
  // set none protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_NONE,
			     SYSCALL_FIELD_PROTECTION_NONE);

  /*
   * Return remaining mmap protection flags so that caller can
   * warn of unknown flags if the mmap_prot_flags is not set
   * as zero.
   */
  return mmap_prot_flags;
}

u_int DataSeriesOutputModule::processMmapFlags(void **args_map,
					       u_int mmap_flags) {
  /*
   * Process each individual mmap flag bit that has been set
   * in the argument mmap_flags.
   */
  // set mmap fixed flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_FIXED,
			     SYSCALL_FIELD_FLAG_FIXED);
  // set mmap shared flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_SHARED,
			     SYSCALL_FIELD_FLAG_SHARED);
  // set mmap private flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_PRIVATE,
			     SYSCALL_FIELD_FLAG_PRIVATE);
  // set mmap 32bit flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_32BIT,
			     SYSCALL_FIELD_FLAG_32BIT);
  // set mmap anonymous flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_ANONYMOUS,
			     SYSCALL_FIELD_FLAG_ANONYMOUS);
  // set mmap denywrite flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_DENYWRITE,
			     SYSCALL_FIELD_FLAG_DENYWRITE);
  // set mmap executable flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_EXECUTABLE,
			     SYSCALL_FIELD_FLAG_EXECUTABLE);
  // set mmap file flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_FILE,
			     SYSCALL_FIELD_FLAG_FILE);
  // set mmap grows_down flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_GROWSDOWN,
			     SYSCALL_FIELD_FLAG_GROWS_DOWN);
  // set mmap huge TLB flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_HUGETLB,
			     SYSCALL_FIELD_FLAG_HUGE_TLB);
  // set mmap locked flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_LOCKED,
			     SYSCALL_FIELD_FLAG_LOCKED);
  // set mmap non-blocking flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_NONBLOCK,
			     SYSCALL_FIELD_FLAG_NON_BLOCK);
  // set mmap no reserve flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_NORESERVE,
			     SYSCALL_FIELD_FLAG_NO_RESERVE);
  // set mmap populate flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_POPULATE,
			     SYSCALL_FIELD_FLAG_POPULATE);
  // set mmap stack flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_STACK,
			     SYSCALL_FIELD_FLAG_STACK);

  /*
   * Return remaining mmap flags so that caller can
   * warn of unknown flags if the mmap_flags is not set
   * as zero.
   */
  return mmap_flags;
}

void DataSeriesOutputModule::makeMunmapArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  args_map[SYSCALL_FIELD_START_ADDRESS] = &args[0];
  args_map[SYSCALL_FIELD_LENGTH] = &args[1];
}

void DataSeriesOutputModule::makeGetdentsArgsMap(void **args_map,
                                                long *args,
                                                void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_DIRENT_BUFFER] = &v_args[0];
  args_map[SYSCALL_FIELD_COUNT] = &args[2];
}

void DataSeriesOutputModule::makeGetrlimitArgsMap(void **args_map,
                                                  long *args,
                                                  void **v_args) {
  args_map[SYSCALL_FIELD_RESOURCE_VALUE] = &args[0];
  /*
   * TODO: The correct value of args_map["resource"] should be 0 if resource is
   * RLIMIT_AS, 1 if it is RLIMIT_CORE, 2 if it is RLIMIT_CPU, so and so forth.
   * Currently, we don't do this. We simply assume that resource is same
   * across different platforms.
   */
  args_map[SYSCALL_FIELD_RESOURCE] = &args[0];
  if (v_args[0] != NULL) {
    struct rlimit *rlim = (struct rlimit *) v_args[0];
    args_map[SYSCALL_FIELD_RESOURCE_SOFT_LIMIT] = &rlim->rlim_cur;
    args_map[SYSCALL_FIELD_RESOURCE_HARD_LIMIT] = &rlim->rlim_max;
  } else {
    std::cerr << "Getrlimit: Struct rlimit is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeIoctlArgsMap(void **args_map,
                                             long *args,
                                             void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_REQUEST] = &args[1];
  args_map[SYSCALL_FIELD_PARAMETER] = &args[2];
  args_map[SYSCALL_FIELD_IOCTL_BUFFER] = &v_args[0];
  args_map[SYSCALL_FIELD_BUFFER_SIZE] = &ioctl_size_;
}

void DataSeriesOutputModule::makeCloneArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  initArgsMap(args_map, "clone");

  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[0];
  u_int flag = processCloneFlags(args_map, args[0]);
  flag = processCloneSignals(args_map, flag);
  if (flag != 0) {
    std::cerr << "Clone: These flags are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }

  args_map[SYSCALL_FIELD_CHILD_STACK_ADDRESS] = &args[1];

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_PARENT_THREAD_ID] = &v_args[0];
  } else if (args[0] & CLONE_PARENT_SETTID) {
    std::cerr << "Clone: Parent thread ID is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_CHILD_THREAD_ID] = &v_args[1];
  } else if ((args[0] & CLONE_CHILD_SETTID)
	     || (args[0] & CLONE_CHILD_CLEARTID)) {
    std::cerr << "Clone: Child thread ID is set as NULL!!" << std::endl;
  }

  if (clone_ctid_index_ == 3)
    args_map[SYSCALL_FIELD_NEW_TLS] = &args[4];
  else
    args_map[SYSCALL_FIELD_NEW_TLS] = &args[3];
}


u_int DataSeriesOutputModule::processCloneFlags(void **args_map,
						u_int flag) {
  /*
   * Process each individual clone flag bit that has been set.
   */
  // set child_cleartid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_CHILD_CLEARTID,
			     SYSCALL_FIELD_FLAG_CHILD_CLEARTID);

  // set child_settid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_CHILD_SETTID,
			     SYSCALL_FIELD_FLAG_CHILD_SETTID);

  // set files flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_FILES,
			     SYSCALL_FIELD_FLAG_FILES);

  // set filesystem flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_FS,
			     SYSCALL_FIELD_FLAG_FILESYSTEM);

  // set I/O flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_IO,
			     SYSCALL_FIELD_FLAG_IO);

  // set newipc flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWIPC,
			     SYSCALL_FIELD_FLAG_NEWIPC);

  // set newnet flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWNET,
			     SYSCALL_FIELD_FLAG_NEWNET);

  // set newns flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWNS,
			     SYSCALL_FIELD_FLAG_NEWNS);

  // set newpid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWPID,
			     SYSCALL_FIELD_FLAG_NEWPID);

  // set newuser flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWUSER,
			     SYSCALL_FIELD_FLAG_NEWUSER);

  // set newuts flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWUTS,
			     SYSCALL_FIELD_FLAG_NEWUTS);

  // set parent flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_PARENT,
			     SYSCALL_FIELD_FLAG_PARENT);

  // set parent_settid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_PARENT_SETTID,
			     SYSCALL_FIELD_FLAG_PARENT_SETTID);

  // set ptrace flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_PTRACE,
			     SYSCALL_FIELD_FLAG_PTRACE);

  // set settls flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_SETTLS,
			     SYSCALL_FIELD_FLAG_SETTLS);

  // set sighand flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_SIGHAND,
			     SYSCALL_FIELD_FLAG_SIGHAND);

  // set sysvsem flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_SYSVSEM,
			     SYSCALL_FIELD_FLAG_SYSVSEM);

  // set thread flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_THREAD,
			     SYSCALL_FIELD_FLAG_THREAD);

  // set untraced flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_UNTRACED,
			     SYSCALL_FIELD_FLAG_UNTRACED);

  // set vfork flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_VFORK,
			     SYSCALL_FIELD_FLAG_VFORK);

  // set vm flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_VM,
			     SYSCALL_FIELD_FLAG_VM);

  return flag;
}

u_int DataSeriesOutputModule::processCloneSignals(void **args_map,
						 u_int flag) {
  /*
   * Process each individual clone signal bit that has been set in the flags
   * passed to clone
   */
  // set signal_hangup field
  process_Flag_and_Mode_Args(args_map, flag, SIGHUP,
			     SYSCALL_FIELD_SIGNAL_HANGUP);
  // set signal_terminal_interrupt field
  process_Flag_and_Mode_Args(args_map, flag, SIGINT,
			     SYSCALL_FIELD_SIGNAL_TERMINAL_INTERRUPT);

  // set signal_terminal_quit field
  process_Flag_and_Mode_Args(args_map, flag, SIGQUIT,
			     SYSCALL_FIELD_SIGNAL_TERMINAL_QUIT);

  // set signal_illegal field
  process_Flag_and_Mode_Args(args_map, flag, SIGILL,
			     SYSCALL_FIELD_SIGNAL_ILLEGAL);

  // set signal_trace_trap field
  process_Flag_and_Mode_Args(args_map, flag, SIGTRAP,
			     SYSCALL_FIELD_SIGNAL_TRACE_TRAP);

  // set signal_abort field
  process_Flag_and_Mode_Args(args_map, flag, SIGABRT,
			     SYSCALL_FIELD_SIGNAL_ABORT);

  // set signal_iot_trap field
  process_Flag_and_Mode_Args(args_map, flag, SIGIOT,
			     SYSCALL_FIELD_SIGNAL_IOT_TRAP);

  // set signal_bus field
  process_Flag_and_Mode_Args(args_map, flag, SIGBUS,
			     SYSCALL_FIELD_SIGNAL_BUS);

  // set signal_floating_point_exception field
  process_Flag_and_Mode_Args(args_map, flag, SIGFPE,
			     SYSCALL_FIELD_SIGNAL_FLOATING_POINT_EXCEPTION);

  // set signal_kill field
  process_Flag_and_Mode_Args(args_map, flag, SIGKILL,
			     SYSCALL_FIELD_SIGNAL_KILL);

  // set signal_user_defined_1 field
  process_Flag_and_Mode_Args(args_map, flag, SIGUSR1,
			     SYSCALL_FIELD_SIGNAL_USER_DEFINED_1);

  // set signal_segv field
  process_Flag_and_Mode_Args(args_map, flag, SIGSEGV,
			     SYSCALL_FIELD_SIGNAL_SEGV);

  // set signal_user_defined_2 field
  process_Flag_and_Mode_Args(args_map, flag, SIGUSR2,
			     SYSCALL_FIELD_SIGNAL_USER_DEFINED_2);

  // set signal_pipe field
  process_Flag_and_Mode_Args(args_map, flag, SIGPIPE,
			     SYSCALL_FIELD_SIGNAL_PIPE);

  // set signal_alarm field
  process_Flag_and_Mode_Args(args_map, flag, SIGALRM,
			     SYSCALL_FIELD_SIGNAL_ALARM);

  // set signal_termination field
  process_Flag_and_Mode_Args(args_map, flag, SIGTERM,
			     SYSCALL_FIELD_SIGNAL_TERMINATION);

  // set signal_stack_fault field
  process_Flag_and_Mode_Args(args_map, flag, SIGSTKFLT,
			     SYSCALL_FIELD_SIGNAL_STACK_FAULT);

  // set signal_child field
  process_Flag_and_Mode_Args(args_map, flag, SIGCHLD,
			     SYSCALL_FIELD_SIGNAL_CHILD);

  // set signal_continue field
  process_Flag_and_Mode_Args(args_map, flag, SIGCONT,
			     SYSCALL_FIELD_SIGNAL_CONTINUE);

  // set signal_stop field
  process_Flag_and_Mode_Args(args_map, flag, SIGSTOP,
			     SYSCALL_FIELD_SIGNAL_STOP);

  // set signal_terminal_stop field
  process_Flag_and_Mode_Args(args_map, flag, SIGTSTP,
			     SYSCALL_FIELD_SIGNAL_TERMINAL_STOP);

  // set signal_tty_read field
  process_Flag_and_Mode_Args(args_map, flag, SIGTTIN,
			     SYSCALL_FIELD_SIGNAL_TTY_READ);

  // set signal_tty_write field
  process_Flag_and_Mode_Args(args_map, flag, SIGTTOU,
			     SYSCALL_FIELD_SIGNAL_TTY_WRITE);

  // set signal_urgent field
  process_Flag_and_Mode_Args(args_map, flag, SIGURG,
			     SYSCALL_FIELD_SIGNAL_URGENT);

  // set signal_cpu_exceeded field
  process_Flag_and_Mode_Args(args_map, flag, SIGXCPU,
			     SYSCALL_FIELD_SIGNAL_CPU_EXCEEDED);

  // set signal_file_size_exceeded field
  process_Flag_and_Mode_Args(args_map, flag, SIGXFSZ,
			     SYSCALL_FIELD_SIGNAL_FILE_SIZE_EXCEEDED);

  // set signal_virtual_alarm field
  process_Flag_and_Mode_Args(args_map, flag, SIGVTALRM,
			     SYSCALL_FIELD_SIGNAL_VIRTUAL_ALARM);

  // set signal_prof_alarm field
  process_Flag_and_Mode_Args(args_map, flag, SIGPROF,
			     SYSCALL_FIELD_SIGNAL_PROF_ALARM);

  // set signal_window_size_change field
  process_Flag_and_Mode_Args(args_map, flag, SIGWINCH,
			     SYSCALL_FIELD_SIGNAL_WINDOW_SIZE_CHANGE);

  // set signal_io field
  process_Flag_and_Mode_Args(args_map, flag, SIGIO,
			     SYSCALL_FIELD_SIGNAL_IO);

  // set signal_power field
  process_Flag_and_Mode_Args(args_map, flag, SIGPWR,
			     SYSCALL_FIELD_SIGNAL_POWER);

  return flag;
}

void DataSeriesOutputModule::makeVForkArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  /*
   * VFork takes no arguments, so we do not need to set any specific
   * fields in args_map
   */
}

void DataSeriesOutputModule::makeSocketArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  args_map["domain"] = &args[0];
  args_map["type"] = &args[1];
  args_map["protocol"] = &args[2];
}
