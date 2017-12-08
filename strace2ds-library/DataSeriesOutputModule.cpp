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

  SysCallArgsMap sys_call_args_map;
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
  sys_call_args_map["unique_id"] = common_fields[DS_COMMON_FIELD_UNIQUE_ID];

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
    sys_call_args_map["time_called"] = &time_called_Tfrac;
  }

  /* set time returned field */
  if (common_fields[DS_COMMON_FIELD_TIME_RETURNED] != NULL) {
    // Convert tv_time_returned to Tfracs
    time_returned_Tfrac = timeval_to_Tfrac(
      *(struct timeval *) common_fields[DS_COMMON_FIELD_TIME_RETURNED]);
    sys_call_args_map["time_returned"] = &time_returned_Tfrac;
  }

  /* set executing pid field */
  if (common_fields[DS_COMMON_FIELD_EXECUTING_PID] != NULL) {
    sys_call_args_map["executing_pid"] =
      common_fields[DS_COMMON_FIELD_EXECUTING_PID];
  }

  /* set executing tid field */
  if (common_fields[DS_COMMON_FIELD_EXECUTING_TID] != NULL) {
    sys_call_args_map["executing_tid"] =
      common_fields[DS_COMMON_FIELD_EXECUTING_TID];
  }

  /* set return value field */
  if (common_fields[DS_COMMON_FIELD_RETURN_VALUE] != NULL) {
    sys_call_args_map["return_value"] =
      common_fields[DS_COMMON_FIELD_RETURN_VALUE];
  }

  /* set errno number field */
  if (common_fields[DS_COMMON_FIELD_ERRNO_NUMBER] != NULL) {
    sys_call_args_map["errno_number"] =
      common_fields[DS_COMMON_FIELD_ERRNO_NUMBER];
  }

  /* set system call specific field */
  FuncPtrMap::iterator iter = func_ptr_map_.find(extent_name);
  if (iter != func_ptr_map_.end()) {
    SysCallArgsMapFuncPtr fxn = iter->second;
    (this->*fxn)(sys_call_args_map, args, v_args);
  }

  // Create a new record to write
  modules_[extent_name]->newRecord();

  /*
   * Get the time the record was written as late as possible
   * before we actually write the record.
   */
  gettimeofday(&tv_time_recorded, NULL);
  // Convert time_recorded_timeval to Tfracs and add it to the map
  uint64_t time_recorded_Tfrac = timeval_to_Tfrac(tv_time_recorded);
  sys_call_args_map["time_recorded"] = &time_recorded_Tfrac;

  config_table_entry_type& extent_config_table_ =
    config_table_[extent_name];

  // Write values to the new record
  for (auto const &extent_config_table_entry : extent_config_table_) {
    const std::string& field_name = extent_config_table_entry.first;
    const bool nullable = extent_config_table_entry.second.first;
    const ExtentFieldTypePair& extent_field_value_ =
      extents_[extent_name][field_name];
    var32_len = 0;

    if (sys_call_args_map.find(field_name) != sys_call_args_map.end() &&
      sys_call_args_map[field_name] != NULL) {
      void *field_value = sys_call_args_map[field_name];
      /*
       * If field is of type Variable32, then retrieve the length of the
       * field that needs to be set.
       */
      if (extent_field_value_.second == ExtentType::ft_variable32) {
        var32_len = getVariable32FieldLength(sys_call_args_map, field_name);
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
    /* We are ignoring  split_data[1]: syscall_id, split_data[2]: field_id for now */
    if (split_data.size() == 6) {  
      std::string field_name = split_data[3];
      std::string nullable_str = split_data[4];
      std::string field_type = split_data[5];
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
      common_field_map[field_name] = std::make_pair(nullable, ftype);
    else if (config_table_.find(extent_name) != config_table_.end())
      config_table_[extent_name][field_name] = std::make_pair(nullable, ftype);
    else { /* New extent detected */
      config_table_[extent_name] = common_field_map;
      config_table_[extent_name][field_name] = std::make_pair(nullable, ftype);
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
int DataSeriesOutputModule::getVariable32FieldLength(SysCallArgsMap &args_map,
						     const std::string
						     &field_name) {
  int length = 0;
  SysCallArgsMap::iterator it = args_map.find(field_name);
  if (it != args_map.end()) {
    /*
     * If field_name refers to the pathname passed as an argument to
     * the system call, string length function can be used to determine
     * the length.  Strlen does not count the terminating null character,
     * so we add 1 to its return value to get the full length of the pathname.
     */
    if ((field_name == "given_pathname") ||
      (field_name == "given_oldpathname") ||
      (field_name == "given_newpathname") ||
      (field_name == "target_pathname") ||
      (field_name == "given_oldname") ||
      (field_name == "given_newname") ||
      (field_name == "argument") ||
      (field_name == "environment")) {
      void *field_value = args_map[field_name];
      length = strlen(*(char **) field_value) + 1;
      /*
       * If field_name refers to the actual data read or written, then length
       * of buffer must be the return value of that corresponding system call.
       */
    } else if ((field_name == "data_read") ||
      (field_name == "data_written") ||
      (field_name == "link_value") ||
      (field_name == "dirent_buffer")) {
      length = *(int *)(args_map["return_value"]);
    } else if (field_name == "ioctl_buffer") {
      length = ioctl_size_;
    }
  } else {
    std::cerr << "WARNING: field_name = " << field_name << " ";
    std::cerr << "is not set in the arguments map";
  }
  return length;
}

// Initialize all non-nullable boolean fields as False of given extent_name.
void DataSeriesOutputModule::initArgsMap(SysCallArgsMap &args_map,
					 const char *extent_name) {
  const config_table_entry_type& extent_config_table_ =
    config_table_[extent_name];
  FieldMap& extent_field_map_ = extents_[extent_name];
  for (auto const &extent_config_table_entry : extent_config_table_) {
    const std::string& field_name = extent_config_table_entry.first;
    const bool nullable = extent_config_table_entry.second.first;
    if (!nullable && extent_field_map_[field_name].second == ExtentType::ft_bool)
      args_map[field_name] = &false_;
  }
}

void DataSeriesOutputModule::makeCloseArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  args_map["descriptor"] = &args[0];
}

void DataSeriesOutputModule::makeOpenArgsMap(SysCallArgsMap &args_map,
					     long *args,
					     void **v_args) {
  int offset = 0;

  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "open");

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Open: Pathname is set as NULL!!" << std::endl;
  }

  /* Setting flag values */
  args_map["open_value"] = &args[offset + 1];
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

void DataSeriesOutputModule::makeOpenatArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  int offset = 1;

  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "openat");

  args_map["descriptor"] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map["descriptor_current_working_directory"] = &true_;
  }

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Openat: Pathname is set as NULL!!" << std::endl;
  }

  /* Setting flag values */
  args_map["open_value"] = &args[offset + 1];
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
void DataSeriesOutputModule::process_Flag_and_Mode_Args(SysCallArgsMap &args_map,
							u_int &num,
							int value,
							std::string field_name) {
  if (num & value) {
    args_map[field_name] = (void *) 1;
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
u_int DataSeriesOutputModule::processOpenFlags(SysCallArgsMap &args_map,
					       u_int open_flag) {

  /*
   * Process each individual flag bits that has been set
   * in the argument open_flag.
   */
  // set read only flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_RDONLY,
			     "flag_read_only");
  // set write only flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_WRONLY,
			     "flag_write_only");
  // set both read and write flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_RDWR,
			     "flag_read_and_write");
  // set append flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_APPEND,
			     "flag_append");
  // set async flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_ASYNC,
			     "flag_async");
  // set close-on-exec flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_CLOEXEC,
			     "flag_close_on_exec");
  // set create flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_CREAT,
			     "flag_create");
  // set direct flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_DIRECT,
			     "flag_direct");
  // set directory flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_DIRECTORY,
			     "flag_directory");
  // set exclusive flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_EXCL,
			     "flag_exclusive");
  // set largefile flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_LARGEFILE,
			     "flag_largefile");
  // set last access time flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NOATIME,
			     "flag_no_access_time");
  // set controlling terminal flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NOCTTY,
			     "flag_no_controlling_terminal");
  // set no_follow flag (in case of symbolic link)
  process_Flag_and_Mode_Args(args_map, open_flag, O_NOFOLLOW,
			     "flag_no_follow");
  // set non blocking mode flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NONBLOCK,
			     "flag_no_blocking_mode");
  // set no delay flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NDELAY,
			     "flag_no_delay");
  // set synchronized IO flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_SYNC,
			     "flag_synchronous");
  // set truncate mode flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_TRUNC,
			     "flag_truncate");

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
mode_t DataSeriesOutputModule::processMode(SysCallArgsMap &args_map,
					   long *args,
					   u_int mode_offset) {
  // Save the mode argument with mode_value file in map
  args_map["mode_value"] = &args[mode_offset];
  mode_t mode = args[mode_offset];

  // set user-ID bit
  process_Flag_and_Mode_Args(args_map, mode, S_ISUID, "mode_uid");
  // set group-ID bit
  process_Flag_and_Mode_Args(args_map, mode, S_ISGID, "mode_gid");
  //set sticky bit
  process_Flag_and_Mode_Args(args_map, mode, S_ISVTX, "mode_sticky_bit");
  // set user read permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IRUSR, "mode_R_user");
  // set user write permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IWUSR, "mode_W_user");
  // set user execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IXUSR, "mode_X_user");
  // set group read permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IRGRP, "mode_R_group");
  // set group write permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IWGRP, "mode_W_group");
  // set group execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IXGRP, "mode_X_group");
  // set others read permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IROTH, "mode_R_others");
  // set others write permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IWOTH, "mode_W_others");
  // set others execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IXOTH, "mode_X_others");

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

void DataSeriesOutputModule::makeReadArgsMap(SysCallArgsMap &args_map,
					     long *args,
					     void **v_args) {
  args_map["descriptor"] = &args[0];
  args_map["data_read"] = &v_args[0];
  args_map["bytes_requested"] = &args[2];
}

void DataSeriesOutputModule::makeWriteArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  args_map["descriptor"] = &args[0];
  args_map["data_written"] = &v_args[0];
  args_map["bytes_requested"] = &args[2];
}

void DataSeriesOutputModule::makeChdirArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Chdir: Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeRmdirArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Rmdir: Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeUnlinkArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Unlink: Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeUnlinkatArgsMap(SysCallArgsMap &args_map,
						 long *args,
						 void **v_args) {
  initArgsMap(args_map, "unlinkat");

  args_map["descriptor"] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map["descriptor_current_working_directory"] = &true_;
  }
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Unlinkat: Pathname is set as NULL!!" << std::endl;
  }

  args_map["flag_value"] = &args[2];
  u_int flag = args[2];
  process_Flag_and_Mode_Args(args_map, flag, AT_REMOVEDIR,
			     "flag_remove_directory");
  if (flag != 0) {
    std::cerr << "Unlinkat: These flags are not processed/unknown->"
	      << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeMkdirArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  initArgsMap(args_map, "mkdir");
  int mode_offset = 1;
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Mkdir: Pathname is set as NULL!!" << std::endl;
  }
  mode_t mode = processMode(args_map, args, 1);
  if (mode != 0) {
    std::cerr << "Mkdir: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeMkdiratArgsMap(SysCallArgsMap &args_map,
						long *args,
						void **v_args) {
  int mode_offset = 2;

  // Initialize all non-nullable boolean fields
  initArgsMap(args_map, "mkdirat");

  args_map["descriptor"] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map["descriptor_current_working_directory"] = &true_;
  }

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Mkdirat: Pathname is set as NULL!!" << std::endl;
  }
  mode_t mode = processMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "Mkdirat: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeCreatArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  initArgsMap(args_map, "creat");
  int mode_offset = 1;
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Creat: Pathname is set as NULL!!" << std::endl;
  }
  mode_t mode = processMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "Creat: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeChmodArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  initArgsMap(args_map, "chmod");
  int mode_offset = 1;
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Chmod: Pathname is set as NULL!!" << std::endl;
  }
  mode_t mode = processMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "Chmod: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeUmaskArgsMap(SysCallArgsMap &args_map,
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

void DataSeriesOutputModule::makeSetxattrArgsMap(SysCallArgsMap &args_map,
						 long *args,
						 void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "setxattr");

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Setxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map["xattr_name"] = &v_args[1];
  } else {
    std::cerr << "Setxattr: Attribute name is set as NULL!!" << std::endl;
  }

  args_map["value_written"] = &v_args[2];
  args_map["value_size"] = &args[3];

  /* Setting flag values */
  args_map["flag_value"] = &args[4];
  u_int flag = args[4];
  process_Flag_and_Mode_Args(args_map, flag, XATTR_CREATE, "flag_xattr_create");
  process_Flag_and_Mode_Args(args_map, flag, XATTR_REPLACE, "flag_xattr_replace");
  if (flag != 0) {
    std::cerr << "Setxattr: These flags are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeLSetxattrArgsMap(SysCallArgsMap &args_map,
						  long *args,
						  void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "lsetxattr");

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "LSetxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map["xattr_name"] = &v_args[1];
  } else {
    std::cerr << "LSetxattr: Attribute name is set as NULL!!" << std::endl;
  }

  args_map["value_written"] = &v_args[2];
  args_map["value_size"] = &args[3];

  /* Setting flag values */
  args_map["flag_value"] = &args[4];
  u_int flag = args[4];
  process_Flag_and_Mode_Args(args_map, flag, XATTR_CREATE, "flag_xattr_create");
  process_Flag_and_Mode_Args(args_map, flag, XATTR_REPLACE, "flag_xattr_replace");
  if (flag != 0) {
    std::cerr << "LSetxattr: These flags are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeGetxattrArgsMap(SysCallArgsMap &args_map,
                                                 long *args,
                                                 void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Getxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map["xattr_name"] = &v_args[1];
  } else {
    std::cerr << "Getxattr: Attribute name is set as NULL!!" << std::endl;
  }

  args_map["value_read"] = &v_args[2];
  args_map["value_size"] = &args[3];
}

void DataSeriesOutputModule::makeLGetxattrArgsMap(SysCallArgsMap &args_map,
                                                  long *args,
                                                  void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "LGetxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map["xattr_name"] = &v_args[1];
  } else {
    std::cerr << "LGetxattr: Attribute name is set as NULL!!" << std::endl;
  }

  args_map["value_read"] = &v_args[2];
  args_map["value_size"] = &args[3];
}

void DataSeriesOutputModule::makeFSetxattrArgsMap(SysCallArgsMap &args_map,
						  long *args,
						  void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "fsetxattr");
  args_map["descriptor"] = &args[0];
  if (v_args[0] != NULL) {
    args_map["xattr_name"] = &v_args[0];
  } else {
    std::cerr << "FSetxattr: Attribute name is set as NULL!!" << std::endl;
  }

  args_map["value_written"] = &v_args[1];
  args_map["value_size"] = &args[3];

  /* Setting flag values */
  args_map["flag_value"] = &args[4];
  u_int flag = args[4];
  process_Flag_and_Mode_Args(args_map, flag, XATTR_CREATE, "flag_xattr_create");
  process_Flag_and_Mode_Args(args_map, flag, XATTR_REPLACE, "flag_xattr_replace");
  if (flag != 0) {
    std::cerr << "FSetxattr: These flag are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeFGetxattrArgsMap(SysCallArgsMap &args_map,
						  long *args,
						  void **v_args) {
  args_map["descriptor"] = &args[0];

  if (v_args[0] != NULL) {
    args_map["xattr_name"] = &v_args[0];
  } else {
    std::cerr << "FGetxattr: Attribute name is set as NULL!!" << std::endl;
  }
  args_map["value_read"] = &v_args[1];
  args_map["value_size"] = &args[3];
}

void DataSeriesOutputModule::makeListxattrArgsMap(SysCallArgsMap &args_map,
						  long *args,
						  void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Listxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map["xattr_list"] = &v_args[1];
  } else {
    std::cerr << "Listxattr: Attribute list is set as NULL!!" << std::endl;
  }

  args_map["list_size"] = &args[2];
}

void DataSeriesOutputModule::makeLListxattrArgsMap(SysCallArgsMap &args_map,
						   long *args,
						   void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "LListxattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map["xattr_list"] = &v_args[1];
  } else {
    std::cerr << "LListxattr: Attribute list is set as NULL!!" << std::endl;
  }

  args_map["list_size"] = &args[2];
}

void DataSeriesOutputModule::makeFListxattrArgsMap(SysCallArgsMap &args_map,
						   long *args,
						   void **v_args) {
  args_map["descriptor"] = &args[0];

  if (v_args[0] != NULL) {
    args_map["xattr_list"] = &v_args[0];
  } else {
    std::cerr << "FListxattr: Attribute list is set as NULL!!" << std::endl;
  }

  args_map["list_size"] = &args[2];
}

void DataSeriesOutputModule::makeFLockArgsMap(SysCallArgsMap &args_map,
               long *args,
               void **v_args) {
  args_map["descriptor"] = &args[0];

  args_map["operation_value"] = &args[1];
  /*
   * TODO: The correct value of args_map["operation"] should be 0 if operation is
   * LOCK_SH, 1 if it is LOCK_EX, 2 if it is LOCK_UN, so and so forth.
   * Currently, we don't do this. We simply assume that resource is same
   * across different platforms.
   */
  args_map["operation"] = &args[1];
}

void DataSeriesOutputModule::makeRemovexattrArgsMap(SysCallArgsMap &args_map,
						    long *args,
						    void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Removexattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map["xattr_name"] = &v_args[1];
  } else {
    std::cerr << "Removexattr: Attribute name is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeLRemovexattrArgsMap(SysCallArgsMap &args_map,
						     long *args,
						     void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "LRemovexattr: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map["xattr_name"] = &v_args[1];
  } else {
    std::cerr << "LRemovexattr: Attribute name is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeFRemovexattrArgsMap(SysCallArgsMap &args_map,
						     long *args,
						     void **v_args) {
  args_map["descriptor"] = &args[0];

  if (v_args[0] != NULL) {
    args_map["xattr_name"] = &v_args[0];
  } else {
    std::cerr << "FRemovexattr: Attribute name is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeFChmodArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  initArgsMap(args_map, "fchmod");
  int mode_offset = 1;
  args_map["descriptor"] = &args[0];
  mode_t mode = processMode(args_map, args, 1);
  if (mode != 0) {
    std::cerr << "FChmod: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeFChmodatArgsMap(SysCallArgsMap &args_map,
						 long *args,
						 void **v_args) {
  int mode_offset = 2;
  initArgsMap(args_map, "fchmodat");

  args_map["descriptor"] = &args[0];

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
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
  args_map["flag_value"] = &args[3];
  u_int flag = args[3];
  process_Flag_and_Mode_Args(args_map, flag, AT_SYMLINK_NOFOLLOW, \
			     "flag_at_symlink_nofollow");
  if (flag != 0) {
    std::cerr << "FChmodat: These flags are not processed/unknown->0";
    std::cerr << std::oct << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeLinkArgsMap(SysCallArgsMap &args_map,
					     long *args,
					     void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_oldpathname"] = &v_args[0];
  } else {
    std::cerr << "Link: Old Pathname is set as NULL!!" << std::endl;
  }
  if (v_args[1] != NULL) {
    args_map["given_newpathname"] = &v_args[1];
  } else {
    std::cerr << "Link: New Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeLinkatArgsMap(SysCallArgsMap &args_map,
					     long *args,
					     void **v_args) {
  initArgsMap(args_map, "linkat");

  args_map["old_descriptor"] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map["old_descriptor_current_working_directory"] = &true_;
  }

  args_map["new_descriptor"] = &args[2];
  if (args[2] == AT_FDCWD) {
    args_map["new_descriptor_current_working_directory"] = &true_;
  }

  if (v_args[0] != NULL) {
    args_map["given_oldpathname"] = &v_args[0];
  } else {
    std::cerr << "Linkat: Old Pathname is set as NULL!!" << std::endl;
  }
  if (v_args[1] != NULL) {
    args_map["given_newpathname"] = &v_args[1];
  } else {
    std::cerr << "Linkat: New Pathname is set as NULL!!" << std::endl;
  }

  args_map["flag_value"] = &args[4];
  u_int flag = args[4];
  process_Flag_and_Mode_Args(args_map, flag, AT_EMPTY_PATH,
			     "flag_empty_path");
  process_Flag_and_Mode_Args(args_map, flag, AT_SYMLINK_FOLLOW,
			     "flag_symlink_follow");
  if (flag != 0) {
    std::cerr << "Linkat: These flags are not processed/unknown->"
	      << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeSymlinkArgsMap(SysCallArgsMap &args_map,
						long *args,
						void **v_args) {
  if (v_args[0] != NULL) {
    args_map["target_pathname"] = &v_args[0];
  } else {
    std::cerr << "Symlink: Target Pathname is set as NULL!!" << std::endl;
  }
  if (v_args[1] != NULL) {
    args_map["given_pathname"] = &v_args[1];
  } else {
    std::cerr << "Symlink: Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeSymlinkatArgsMap(SysCallArgsMap &args_map,
						  long *args,
						  void **v_args) {
  static bool true_ = true;

  initArgsMap(args_map, "symlinkat");

  if (v_args[0] != NULL) {
    args_map["target_pathname"] = &v_args[0];
  } else {
    std::cerr << "Symlinkat: Target Pathname is set as NULL!!" << std::endl;
  }

  args_map["new_descriptor"] = &args[2];
  if (args[2] == AT_FDCWD) {
    args_map["new_descriptor_current_working_directory"] = &true_;
  }

  if (v_args[1] != NULL) {
    args_map["given_pathname"] = &v_args[1];
  } else {
    std::cerr << "Symlinkat: Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeTruncateArgsMap(SysCallArgsMap &args_map,
						 long *args,
						 void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Truncate: Pathname is set as NULL!!" << std::endl;
  }
  args_map["truncate_length"] = &args[1];
}

void DataSeriesOutputModule::makeAccessArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "access");
  u_int mode_offset = 1;

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
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
mode_t DataSeriesOutputModule::processAccessMode(SysCallArgsMap &args_map,
						 long *args,
						 u_int mode_offset) {
  // Save the mode argument with mode_value field in the map
  args_map["mode_value"] = &args[mode_offset];
  mode_t mode = args[mode_offset];

  // set read permission bit
  process_Flag_and_Mode_Args(args_map, mode, R_OK, "mode_read");
  // set write permission bit
  process_Flag_and_Mode_Args(args_map, mode, W_OK, "mode_write");
  // set execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, X_OK, "mode_execute");
  // set existence bit
  process_Flag_and_Mode_Args(args_map, mode, F_OK, "mode_exist");

  /*
   * Return remaining unprocessed modes so that caller can warn
   * of unknown modes if the mode value is not set as zero.
   */
  return mode;
}

void DataSeriesOutputModule::makeLSeekArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  args_map["descriptor"] = &args[0];
  args_map["offset"] = &args[1];
  args_map["whence"] = &args[2];
}

void DataSeriesOutputModule::makePReadArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  args_map["descriptor"] = &args[0];
  args_map["data_read"] = &v_args[0];
  args_map["bytes_requested"] = &args[2];
  args_map["offset"] = &args[3];
}

void DataSeriesOutputModule::makePWriteArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  args_map["descriptor"] = &args[0];
  args_map["data_written"] = &v_args[0];
  args_map["bytes_requested"] = &args[2];
  args_map["offset"] = &args[3];
}

void DataSeriesOutputModule::makeSetpgidArgsMap(SysCallArgsMap &args_map,
						long *args,
						void **v_args) {
  args_map["pid"] = &args[0];
  args_map["pgid"] = &args[1];
}

void DataSeriesOutputModule::makeSetrlimitArgsMap(SysCallArgsMap &args_map,
						  long *args,
						  void **v_args) {
  args_map["resource_value"] = &args[0];
  /*
   * TODO: The correct value of args_map["resource"] should be 0 if resource is
   * RLIMIT_AS, 1 if it is RLIMIT_CORE, 2 if it is RLIMIT_CPU, so and so forth.
   * Currently, we don't do this. We simply assume that resource is same
   * across different platforms.
   */
  args_map["resource"] = &args[0];
  if (v_args[0] != NULL) {
    struct rlimit *rlim = (struct rlimit *) v_args[0];
    args_map["resource_soft_limit"] = &rlim->rlim_cur;
    args_map["resource_hard_limit"] = &rlim->rlim_max;
  } else {
    std::cerr << "Setrlimit: Struct rlimit is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeSetsidArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  // Takes no arguments
}

void DataSeriesOutputModule::makeStatArgsMap(SysCallArgsMap &args_map,
					     long *args,
					     void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Stat: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    struct stat *statbuf = (struct stat *) v_args[1];

    args_map["stat_result_dev"] = &statbuf->st_dev;
    args_map["stat_result_ino"] = &statbuf->st_ino;
    args_map["stat_result_mode"] = &statbuf->st_mode;
    args_map["stat_result_nlink"] = &statbuf->st_nlink;
    args_map["stat_result_uid"] = &statbuf->st_uid;
    args_map["stat_result_gid"] = &statbuf->st_gid;
    args_map["stat_result_rdev"] = &statbuf->st_rdev;
    args_map["stat_result_size"] = &statbuf->st_size;
    args_map["stat_result_blksize"] = &statbuf->st_blksize;
    args_map["stat_result_blocks"] = &statbuf->st_blocks;

    /*
     * Convert stat_result_atime, stat_result_mtime and
     * stat_result_ctime to Tfracs.
     */
    static uint64_t atime_Tfrac = timespec_to_Tfrac(statbuf->st_atim);
    static uint64_t mtime_Tfrac = timespec_to_Tfrac(statbuf->st_mtim);
    static uint64_t ctime_Tfrac = timespec_to_Tfrac(statbuf->st_ctim);
    args_map["stat_result_atime"] = &atime_Tfrac;
    args_map["stat_result_mtime"] = &mtime_Tfrac;
    args_map["stat_result_ctime"] = &ctime_Tfrac;
  } else {
    std::cerr << "Stat: Struct stat buffer is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeStatfsArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "statfs");

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Statfs: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    struct statfs *statfsbuf = (struct statfs *) v_args[1];

    args_map["statfs_result_type"] = &statfsbuf->f_type;
    args_map["statfs_result_bsize"] = &statfsbuf->f_bsize;
    args_map["statfs_result_blocks"] = &statfsbuf->f_blocks;
    args_map["statfs_result_bfree"] = &statfsbuf->f_bfree;
    args_map["statfs_result_bavail"] = &statfsbuf->f_bavail;
    args_map["statfs_result_files"] = &statfsbuf->f_files;
    args_map["statfs_result_ffree"] = &statfsbuf->f_ffree;
    args_map["statfs_result_fsid"] = &statfsbuf->f_fsid;
    args_map["statfs_result_namelen"] = &statfsbuf->f_namelen;
    args_map["statfs_result_frsize"] = &statfsbuf->f_frsize;
    args_map["statfs_result_flags"] = &statfsbuf->f_flags;

    u_int flag = processStatfsFlags(args_map, statfsbuf->f_flags);
    if (flag != 0) {
      std::cerr << "Statfs: These flag are not processed/unknown->0x";
      std::cerr << std::hex << flag << std::dec << std::endl;
    }
  } else {
    std::cerr << "Statfs: Struct statfs is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeFStatfsArgsMap(SysCallArgsMap &args_map,
						long *args,
						void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "fstatfs");

  args_map["descriptor"] = &args[0];

  if (v_args[0] != NULL) {
    struct statfs *statfsbuf = (struct statfs *) v_args[0];

    args_map["statfs_result_type"] = &statfsbuf->f_type;
    args_map["statfs_result_bsize"] = &statfsbuf->f_bsize;
    args_map["statfs_result_blocks"] = &statfsbuf->f_blocks;
    args_map["statfs_result_bfree"] = &statfsbuf->f_bfree;
    args_map["statfs_result_bavail"] = &statfsbuf->f_bavail;
    args_map["statfs_result_files"] = &statfsbuf->f_files;
    args_map["statfs_result_ffree"] = &statfsbuf->f_ffree;
    args_map["statfs_result_fsid"] = &statfsbuf->f_fsid;
    args_map["statfs_result_namelen"] = &statfsbuf->f_namelen;
    args_map["statfs_result_frsize"] = &statfsbuf->f_frsize;
    args_map["statfs_result_flags"] = &statfsbuf->f_flags;

    u_int flag = processStatfsFlags(args_map, statfsbuf->f_flags);
    if (flag != 0) {
      std::cerr << "FStatfs: These flags are not processed/unknown->0x";
      std::cerr << std::hex << flag << std::dec << std::endl;
    }
  } else {
    std::cerr << "FStatfs: Struct statfs is set as NULL!!" << std::endl;
  }
}

u_int DataSeriesOutputModule::processStatfsFlags(SysCallArgsMap &args_map,
						 u_int statfs_flags) {
  /*
   * Process each individual statfs flag bit that has been set
   * in the argument stafs_flags.
   */
  // set mandatory lock flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_MANDLOCK,
			     "flags_mandatory_lock");
  // set no access time flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NOATIME,
			     "flags_no_access_time");
  // set no dev flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NODEV,
			     "flags_no_dev");
  // set no directory access time flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NODIRATIME,
			     "flags_no_directory_access_time");
  // set no exec flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NOEXEC,
			     "flags_no_exec");
  // set no set uid flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NOSUID,
			     "flags_no_set_uid");
  // set read only flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_RDONLY,
			     "flags_read_only");
  // set relative access time flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_RELATIME,
			     "flags_relative_access_time");
  // set synchronous flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_SYNCHRONOUS,
			     "flags_synchronous");
  // set valid flag (f_flags support is implemented)
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_VALID,
           "flags_valid");

  /*
   * Return remaining statfs flags so that caller can
   * warn of unknown flags if the statfs_flags is not set
   * as zero.
   */
  return statfs_flags;
}

void DataSeriesOutputModule::makeFTruncateArgsMap(SysCallArgsMap &args_map,
						  long *args,
						  void **v_args) {
  args_map["descriptor"] = &args[0];
  args_map["truncate_length"] = &args[1];
}

void DataSeriesOutputModule::makeChownArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Chown: Pathname is set as NULL!!" << std::endl;
  }

  args_map["new_owner"] = &args[1];
  args_map["new_group"] = &args[2];
}

void DataSeriesOutputModule::makeReadlinkArgsMap(SysCallArgsMap &args_map,
						 long *args,
						 void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Readlink: Pathname is set as NULL!!" << std::endl;
  }
  args_map["link_value"] = &v_args[1];
  args_map["buffer_size"] = &args[2];
}

void DataSeriesOutputModule::makeReadvArgsMap(SysCallArgsMap &args_map,
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
    args_map["descriptor"] = &args[0];
    args_map["count"] = &args[2];
    args_map["iov_number"] = v_args[0];
    args_map["bytes_requested"] = v_args[1];
  } else {
    /*
     * For rest of the records, we do not save file descriptor and
     * count fields. We only save the iov_number, bytes_requested
     * and data_read.
     */
    args_map["iov_number"] = v_args[0];
    args_map["bytes_requested"] = v_args[1];
    args_map["data_read"] = &v_args[2];
  }
}

void DataSeriesOutputModule::makeWritevArgsMap(SysCallArgsMap &args_map,
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
    args_map["descriptor"] = &args[0];
    args_map["count"] = &args[2];
    args_map["iov_number"] = v_args[0];
    args_map["bytes_requested"] = v_args[1];
  } else {
    /*
     * For rest of the records, we do not save file descriptor and
     * count fields. We only save the iov_number, bytes_requested
     * and data_written fields.
     */
    args_map["iov_number"] = v_args[0];
    args_map["bytes_requested"] = v_args[1];
    args_map["data_written"] = &v_args[2];
  }
}

void DataSeriesOutputModule::makeUtimeArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  static uint64_t access_time_Tfrac;
  static uint64_t mod_time_Tfrac;

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Utime: Pathname is set as NULL!!" << std::endl;
  }

  // If the utimbuf is not NULL, set the corresponding values in the map
  if (v_args[1] != NULL) {
    struct utimbuf *times = (struct utimbuf *) v_args[1];

    // Convert the time_t members of the struct utimbuf to Tfracs (uint64_t)
    access_time_Tfrac = sec_to_Tfrac(times->actime);
    mod_time_Tfrac = sec_to_Tfrac(times->modtime);

    args_map["access_time"] = &access_time_Tfrac;
    args_map["mod_time"] = &mod_time_Tfrac;
  }
}

void DataSeriesOutputModule::makeLStatArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "LStat: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    struct stat *statbuf = (struct stat *) v_args[1];

    args_map["stat_result_dev"] = &statbuf->st_dev;
    args_map["stat_result_ino"] = &statbuf->st_ino;
    args_map["stat_result_mode"] = &statbuf->st_mode;
    args_map["stat_result_nlink"] = &statbuf->st_nlink;
    args_map["stat_result_uid"] = &statbuf->st_uid;
    args_map["stat_result_gid"] = &statbuf->st_gid;
    args_map["stat_result_rdev"] = &statbuf->st_rdev;
    args_map["stat_result_size"] = &statbuf->st_size;
    args_map["stat_result_blksize"] = &statbuf->st_blksize;
    args_map["stat_result_blocks"] = &statbuf->st_blocks;

    /*
     * Convert stat_result_atime, stat_result_mtime and
     * stat_result_ctime to Tfracs.
     */
    static uint64_t atime_Tfrac = timespec_to_Tfrac(statbuf->st_atim);
    static uint64_t mtime_Tfrac = timespec_to_Tfrac(statbuf->st_mtim);
    static uint64_t ctime_Tfrac = timespec_to_Tfrac(statbuf->st_ctim);
    args_map["stat_result_atime"] = &atime_Tfrac;
    args_map["stat_result_mtime"] = &mtime_Tfrac;
    args_map["stat_result_ctime"] = &ctime_Tfrac;
  } else {
    std::cerr << "LStat: Struct stat buffer is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeFStatArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  args_map["descriptor"] = &args[0];

  if (v_args[0] != NULL) {
    struct stat *statbuf = (struct stat *) v_args[0];

    args_map["stat_result_dev"] = &statbuf->st_dev;
    args_map["stat_result_ino"] = &statbuf->st_ino;
    args_map["stat_result_mode"] = &statbuf->st_mode;
    args_map["stat_result_nlink"] = &statbuf->st_nlink;
    args_map["stat_result_uid"] = &statbuf->st_uid;
    args_map["stat_result_gid"] = &statbuf->st_gid;
    args_map["stat_result_rdev"] = &statbuf->st_rdev;
    args_map["stat_result_size"] = &statbuf->st_size;
    args_map["stat_result_blksize"] = &statbuf->st_blksize;
    args_map["stat_result_blocks"] = &statbuf->st_blocks;

    /*
     * Convert stat_result_atime, stat_result_mtime and
     * stat_result_ctime to Tfracs.
     */
    static uint64_t atime_Tfrac = timespec_to_Tfrac(statbuf->st_atim);
    static uint64_t mtime_Tfrac = timespec_to_Tfrac(statbuf->st_mtim);
    static uint64_t ctime_Tfrac = timespec_to_Tfrac(statbuf->st_ctim);
    args_map["stat_result_atime"] = &atime_Tfrac;
    args_map["stat_result_mtime"] = &mtime_Tfrac;
    args_map["stat_result_ctime"] = &ctime_Tfrac;
  } else {
    std::cerr << "FStat: Struct stat buffer is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeFStatatArgsMap(SysCallArgsMap &args_map,
						long *args,
						void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "fstatat");

  args_map["descriptor"] = &args[0];

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "FStatat: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    struct stat *statbuf = (struct stat *) v_args[1];

    args_map["stat_result_dev"] = &statbuf->st_dev;
    args_map["stat_result_ino"] = &statbuf->st_ino;
    args_map["stat_result_mode"] = &statbuf->st_mode;
    args_map["stat_result_nlink"] = &statbuf->st_nlink;
    args_map["stat_result_uid"] = &statbuf->st_uid;
    args_map["stat_result_gid"] = &statbuf->st_gid;
    args_map["stat_result_rdev"] = &statbuf->st_rdev;
    args_map["stat_result_size"] = &statbuf->st_size;
    args_map["stat_result_blksize"] = &statbuf->st_blksize;
    args_map["stat_result_blocks"] = &statbuf->st_blocks;

    /*
     * Convert stat_result_atime, stat_result_mtime and
     * stat_result_ctime to Tfracs.
     */
    static uint64_t atime_Tfrac = timespec_to_Tfrac(statbuf->st_atim);
    static uint64_t mtime_Tfrac = timespec_to_Tfrac(statbuf->st_mtim);
    static uint64_t ctime_Tfrac = timespec_to_Tfrac(statbuf->st_ctim);
    args_map["stat_result_atime"] = &atime_Tfrac;
    args_map["stat_result_mtime"] = &mtime_Tfrac;
    args_map["stat_result_ctime"] = &ctime_Tfrac;
  } else {
    std::cerr << "FStatat: Struct stat buffer is set as NULL!!" << std::endl;
  }

  args_map["flags_value"] = &args[3];

  u_int flag = processFStatatFlags(args_map, args[3]);
  if (flag != 0) {
    std::cerr << "FStatat: These flags are not processed/unknown->"
	      << std::hex << flag << std::dec << std::endl;
  }
}

u_int DataSeriesOutputModule::processFStatatFlags(SysCallArgsMap &args_map,
						  u_int fstatat_flags) {
  /*
   * Process each individual statfs flag bit that has been set
   * in the argument fstatat_flags.
   */
  // set at empty path flag
  process_Flag_and_Mode_Args(args_map, fstatat_flags, AT_EMPTY_PATH,
			     "flags_at_empty_path");
  // set no auto mount flag
  process_Flag_and_Mode_Args(args_map, fstatat_flags, AT_NO_AUTOMOUNT,
			     "flags_at_no_automount");
  // set symlink nofollow flag
  process_Flag_and_Mode_Args(args_map, fstatat_flags, AT_SYMLINK_NOFOLLOW,
			     "flags_at_symlink_nofollow");

  /*
   * Return remaining fstatat flags so that caller can
   * warn of unknown flags if the fstatat_flags is not set
   * as zero.
   */
  return fstatat_flags;
}

void DataSeriesOutputModule::makeUtimesArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  static uint64_t access_time_Tfrac;
  static uint64_t mod_time_Tfrac;

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Utimes: Pathname is set as NULL!!" << std::endl;
  }

  // If the timeval array is not NULL, set the corresponding values in the map
  if (v_args[1] != NULL) {
    struct timeval *tv = (struct timeval *) v_args[1];

    // Convert timeval arguments to Tfracs (uint64_t)
    access_time_Tfrac = timeval_to_Tfrac(tv[0]);
    mod_time_Tfrac = timeval_to_Tfrac(tv[1]);

    args_map["access_time"] = &access_time_Tfrac;
    args_map["mod_time"] = &mod_time_Tfrac;
  }
}

void DataSeriesOutputModule::makeUtimensatArgsMap(SysCallArgsMap &args_map,
						  long *args,
						  void **v_args) {
  static uint64_t access_time_Tfrac;
  static uint64_t mod_time_Tfrac;
  initArgsMap(args_map, "utimensat");

  args_map["descriptor"] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map["descriptor_current_working_directory"] = &true_;
  }

  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Utimensat: Pathname is set as NULL!!" << std::endl;
  }

  // If the timespec array is not NULL, set the corresponding values in the map
  if (v_args[1] != NULL) {
    struct timespec *ts = (struct timespec *) v_args[1];

    // Check for the special values UTIME_NOW and UTIME_OMIT
    if ((ts[0].tv_nsec == UTIME_NOW) || (ts[1].tv_nsec == UTIME_NOW))
      args_map["utime_now"] = &true_;
    if ((ts[0].tv_nsec == UTIME_OMIT) || (ts[1].tv_nsec == UTIME_OMIT))
      args_map["utime_omit"] = &true_;

    // Convert timespec arguments to Tfracs (uint64_t)
    access_time_Tfrac = timespec_to_Tfrac(ts[0]);
    mod_time_Tfrac = timespec_to_Tfrac(ts[1]);

    args_map["access_time"] = &access_time_Tfrac;
    args_map["mod_time"] = &mod_time_Tfrac;
  }

  args_map["flag_value"] = &args[3];
  u_int flag = args[3];
  process_Flag_and_Mode_Args(args_map, flag, AT_SYMLINK_NOFOLLOW,
			     "flag_symlink_nofollow");
  if (flag != 0) {
    std::cerr << "Utimensat: These flags are not processed/unknown->"
	      << std::hex << flag << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeRenameArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_oldname"] = &v_args[0];
  } else {
    std::cerr << "Rename: Old name is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map["given_newname"] = &v_args[1];
  } else {
    std::cerr << "Rename: New name is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeFsyncArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  args_map["descriptor"] = &args[0];
}

void DataSeriesOutputModule::makeMknodArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  static int32_t dev;
  initArgsMap(args_map, "mknod");
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
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
    args_map["dev"] = &dev;
  }
}

void DataSeriesOutputModule::makeMknodatArgsMap(SysCallArgsMap &args_map,
						long *args,
						void **v_args) {
  static int32_t dev;
  args_map["descriptor"] = &args[0];
  initArgsMap(args_map, "mknodat");
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
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
    args_map["dev"] = &dev;
  }
}

mode_t DataSeriesOutputModule::processMknodType(SysCallArgsMap &args_map,
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
  args_map["type"] = &type;

  /*
   * Return remaining unprocessed modes so that caller can warn
   * of unknown modes if the mode value is not set as zero.
   */
  return mode;
}

void DataSeriesOutputModule::makePipeArgsMap(SysCallArgsMap &args_map,
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

  args_map["read_descriptor"] = &pipefd[0];
  args_map["write_descriptor"] = &pipefd[1];
}

void DataSeriesOutputModule::makeDupArgsMap(SysCallArgsMap &args_map,
					    long *args,
					    void **v_args) {
  args_map["descriptor"] = &args[0];
}

void DataSeriesOutputModule::makeDup2ArgsMap(SysCallArgsMap &args_map,
					     long *args,
					     void **v_args) {
  args_map["old_descriptor"] = &args[0];
  args_map["new_descriptor"] = &args[1];
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

void DataSeriesOutputModule::makeExitArgsMap(SysCallArgsMap &args_map,
					     long *args,
					     void **v_args) {
  args_map["exit_status"] = &args[0];
  args_map["generated"] = v_args[0];
}

void DataSeriesOutputModule::makeExecveArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  int continuation_number = *(int *) v_args[0];
  args_map["continuation_number"] = v_args[0];

  /*
   * Continuation number equal to '0' denotes the first record of
   * single execve system call. For first record, we only save the
   * continuation number and given pathname fields.
   */
  if (continuation_number == 0) {
    if (v_args[1] != NULL)
      args_map["given_pathname"] = &v_args[1];
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
	args_map["argument"] = &v_args[1];
      else
	std::cerr << "Execve: Argument is set as NULL!!" << std::endl;
    } else if (strcmp(arg_env, "env") == 0) {
      /*
       * If arg_env is equal to "env", then we only save the
       * continuation number and environment fields in the
       * new record.
       */
      if (v_args[1] != NULL)
	args_map["environment"] = &v_args[1];
      else
	std::cerr << "Execve : Environment is set as NULL!!" << std::endl;
    }
  }
}

void DataSeriesOutputModule::makeMmapArgsMap(SysCallArgsMap &args_map,
					     long *args,
					     void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "mmap");

  args_map["start_address"] = &args[0];
  args_map["length"] = &args[1];

  args_map["protection_value"] = &args[2];
  // Set individual mmap protection bits
  u_int prot_flags = processMmapProtectionArgs(args_map, args[2]);
  if (prot_flags != 0) {
    std::cerr << "Mmap: These protection flags are not processed/unknown->0x";
    std::cerr << std::hex << prot_flags << std::dec << std::endl;
  }

  args_map["flags_value"] = &args[3];
  // Set individual mmap flag bits
  u_int flag = processMmapFlags(args_map, args[3]);
  if (flag != 0) {
    std::cerr << "Mmap: These flag are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }

  args_map["descriptor"] = &args[4];
  args_map["offset"] = &args[5];
}

u_int DataSeriesOutputModule::processMmapProtectionArgs(SysCallArgsMap &args_map,
							u_int mmap_prot_flags) {
  /*
   * Process each individual mmap protection bit that has been set
   * in the argument mmap_prot_flags.
   */
  // set exec protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_EXEC,
			     "protection_exec");
  // set read protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_READ,
			     "protection_read");
  // set write protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_WRITE,
			     "protection_write");
  // set none protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_NONE,
			     "protection_none");

  /*
   * Return remaining mmap protection flags so that caller can
   * warn of unknown flags if the mmap_prot_flags is not set
   * as zero.
   */
  return mmap_prot_flags;
}

u_int DataSeriesOutputModule::processMmapFlags(SysCallArgsMap &args_map,
					       u_int mmap_flags) {
  /*
   * Process each individual mmap flag bit that has been set
   * in the argument mmap_flags.
   */
  // set mmap fixed flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_FIXED,
			     "flag_fixed");
  // set mmap shared flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_SHARED,
			     "flag_shared");
  // set mmap private flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_PRIVATE,
			     "flag_private");
  // set mmap 32bit flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_32BIT,
			     "flag_32bit");
  // set mmap anonymous flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_ANONYMOUS,
			     "flag_anonymous");
  // set mmap denywrite flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_DENYWRITE,
			     "flag_denywrite");
  // set mmap executable flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_EXECUTABLE,
			     "flag_executable");
  // set mmap file flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_FILE,
			     "flag_file");
  // set mmap grows_down flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_GROWSDOWN,
			     "flag_grows_down");
  // set mmap huge TLB flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_HUGETLB,
			     "flag_huge_tlb");
  // set mmap locked flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_LOCKED,
			     "flag_locked");
  // set mmap non-blocking flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_NONBLOCK,
			     "flag_non_block");
  // set mmap no reserve flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_NORESERVE,
			     "flag_no_reserve");
  // set mmap populate flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_POPULATE,
			     "flag_populate");
  // set mmap stack flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_STACK,
			     "flag_stack");

  /*
   * Return remaining mmap flags so that caller can
   * warn of unknown flags if the mmap_flags is not set
   * as zero.
   */
  return mmap_flags;
}

void DataSeriesOutputModule::makeMunmapArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  args_map["start_address"] = &args[0];
  args_map["length"] = &args[1];
}

void DataSeriesOutputModule::makeGetdentsArgsMap(SysCallArgsMap &args_map,
						 long *args,
						 void **v_args) {
  args_map["descriptor"] = &args[0];
  args_map["dirent_buffer"] = &v_args[0];
  args_map["count"] = &args[2];
}

void DataSeriesOutputModule::makeGetrlimitArgsMap(SysCallArgsMap &args_map,
  long *args,
  void **v_args) {
  args_map["resource_value"] = &args[0];
  /*
   * TODO: The correct value of args_map["resource"] should be 0 if resource is
   * RLIMIT_AS, 1 if it is RLIMIT_CORE, 2 if it is RLIMIT_CPU, so and so forth.
   * Currently, we don't do this. We simply assume that resource is same
   * across different platforms.
   */
  args_map["resource"] = &args[0];
  if (v_args[0] != NULL) {
    struct rlimit *rlim = (struct rlimit *) v_args[0];
    args_map["resource_soft_limit"] = &rlim->rlim_cur;
    args_map["resource_hard_limit"] = &rlim->rlim_max;
  } else {
    std::cerr << "Getrlimit: Struct rlimit is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeIoctlArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  args_map["descriptor"] = &args[0];
  args_map["request"] = &args[1];
  args_map["parameter"] = &args[2];
  args_map["ioctl_buffer"] = &v_args[0];
  args_map["buffer_size"] = &ioctl_size_;
}

void DataSeriesOutputModule::makeCloneArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  initArgsMap(args_map, "clone");

  args_map["flag_value"] = &args[0];
  u_int flag = processCloneFlags(args_map, args[0]);
  flag = processCloneSignals(args_map, flag);
  if (flag != 0) {
    std::cerr << "Clone: These flags are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }

  args_map["child_stack_address"] = &args[1];

  if (v_args[0] != NULL) {
    args_map["parent_thread_id"] = &v_args[0];
  } else if (args[0] & CLONE_PARENT_SETTID) {
    std::cerr << "Clone: Parent thread ID is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map["child_thread_id"] = &v_args[1];
  } else if ((args[0] & CLONE_CHILD_SETTID)
	     || (args[0] & CLONE_CHILD_CLEARTID)) {
    std::cerr << "Clone: Child thread ID is set as NULL!!" << std::endl;
  }

  if (clone_ctid_index_ == 3)
    args_map["new_tls"] = &args[4];
  else
    args_map["new_tls"] = &args[3];
}


u_int DataSeriesOutputModule::processCloneFlags(SysCallArgsMap &args_map,
						u_int flag) {
  /*
   * Process each individual clone flag bit that has been set.
   */
  // set child_cleartid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_CHILD_CLEARTID,
			     "flag_child_cleartid");

  // set child_settid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_CHILD_SETTID,
			     "flag_child_settid");

  // set files flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_FILES,
			     "flag_files");

  // set filesystem flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_FS,
			     "flag_filesystem");

  // set I/O flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_IO,
			     "flag_io");

  // set newipc flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWIPC,
			     "flag_newipc");

  // set newnet flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWNET,
			     "flag_newnet");

  // set newns flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWNS,
			     "flag_newns");

  // set newpid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWPID,
			     "flag_newpid");

  // set newuser flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWUSER,
			     "flag_newuser");

  // set newuts flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWUTS,
			     "flag_newuts");

  // set parent flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_PARENT,
			     "flag_parent");

  // set parent_settid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_PARENT_SETTID,
			     "flag_parent_settid");

  // set ptrace flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_PTRACE,
			     "flag_ptrace");

  // set settls flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_SETTLS,
			     "flag_settls");

  // set sighand flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_SIGHAND,
			     "flag_sighand");

  // set sysvsem flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_SYSVSEM,
			     "flag_sysvsem");

  // set thread flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_THREAD,
			     "flag_thread");

  // set untraced flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_UNTRACED,
			     "flag_untraced");

  // set vfork flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_VFORK,
			     "flag_vfork");

  // set vm flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_VM,
			     "flag_vm");

  return flag;
}

u_int DataSeriesOutputModule::processCloneSignals(SysCallArgsMap &args_map,
						 u_int flag) {
  /*
   * Process each individual clone signal bit that has been set in the flags
   * passed to clone
   */
  // set signal_hangup field
  process_Flag_and_Mode_Args(args_map, flag, SIGHUP,
			     "signal_hangup");
  // set signal_terminal_interrupt field
  process_Flag_and_Mode_Args(args_map, flag, SIGINT,
			     "signal_terminal_interrupt");

  // set signal_terminal_quit field
  process_Flag_and_Mode_Args(args_map, flag, SIGQUIT,
			     "signal_terminal_quit");

  // set signal_illegal field
  process_Flag_and_Mode_Args(args_map, flag, SIGILL,
			     "signal_illegal");

  // set signal_trace_trap field
  process_Flag_and_Mode_Args(args_map, flag, SIGTRAP,
			     "signal_trace_trap");

  // set signal_abort field
  process_Flag_and_Mode_Args(args_map, flag, SIGABRT,
			     "signal_abort");

  // set signal_iot_trap field
  process_Flag_and_Mode_Args(args_map, flag, SIGIOT,
			     "signal_iot_trap");

  // set signal_bus field
  process_Flag_and_Mode_Args(args_map, flag, SIGBUS,
			     "signal_bus");

  // set signal_floating_point_exception field
  process_Flag_and_Mode_Args(args_map, flag, SIGFPE,
			     "signal_floating_point_exception");

  // set signal_kill field
  process_Flag_and_Mode_Args(args_map, flag, SIGKILL,
			     "signal_kill");

  // set signal_user_defined_1 field
  process_Flag_and_Mode_Args(args_map, flag, SIGUSR1,
			     "signal_user_defined_1");

  // set signal_segv field
  process_Flag_and_Mode_Args(args_map, flag, SIGSEGV,
			     "signal_segv");

  // set signal_user_defined_2 field
  process_Flag_and_Mode_Args(args_map, flag, SIGUSR2,
			     "signal_user_defined_2");

  // set signal_pipe field
  process_Flag_and_Mode_Args(args_map, flag, SIGPIPE,
			     "signal_pipe");

  // set signal_alarm field
  process_Flag_and_Mode_Args(args_map, flag, SIGALRM,
			     "signal_alarm");

  // set signal_termination field
  process_Flag_and_Mode_Args(args_map, flag, SIGTERM,
			     "signal_termination");

  // set signal_stack_fault field
  process_Flag_and_Mode_Args(args_map, flag, SIGSTKFLT,
			     "signal_stack_fault");

  // set signal_child field
  process_Flag_and_Mode_Args(args_map, flag, SIGCHLD,
			     "signal_child");

  // set signal_continue field
  process_Flag_and_Mode_Args(args_map, flag, SIGCONT,
			     "signal_continue");

  // set signal_stop field
  process_Flag_and_Mode_Args(args_map, flag, SIGSTOP,
			     "signal_stop");

  // set signal_terminal_stop field
  process_Flag_and_Mode_Args(args_map, flag, SIGTSTP,
			     "signal_terminal_stop");

  // set signal_tty_read field
  process_Flag_and_Mode_Args(args_map, flag, SIGTTIN,
			     "signal_tty_read");

  // set signal_tty_write field
  process_Flag_and_Mode_Args(args_map, flag, SIGTTOU,
			     "signal_tty_write");

  // set signal_urgent field
  process_Flag_and_Mode_Args(args_map, flag, SIGURG,
			     "signal_urgent");

  // set signal_cpu_exceeded field
  process_Flag_and_Mode_Args(args_map, flag, SIGXCPU,
			     "signal_cpu_exceeded");

  // set signal_file_size_exceeded field
  process_Flag_and_Mode_Args(args_map, flag, SIGXFSZ,
			     "signal_file_size_exceeded");

  // set signal_virtual_alarm field
  process_Flag_and_Mode_Args(args_map, flag, SIGVTALRM,
			     "signal_virtual_alarm");

  // set signal_prof_alarm field
  process_Flag_and_Mode_Args(args_map, flag, SIGPROF,
			     "signal_prof_alarm");

  // set signal_window_size_change field
  process_Flag_and_Mode_Args(args_map, flag, SIGWINCH,
			     "signal_window_size_change");

  // set signal_io field
  process_Flag_and_Mode_Args(args_map, flag, SIGIO,
			     "signal_io");

  // set signal_power field
  process_Flag_and_Mode_Args(args_map, flag, SIGPWR,
			     "signal_power");

  return flag;
}

void DataSeriesOutputModule::makeVForkArgsMap(SysCallArgsMap &args_map,
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
