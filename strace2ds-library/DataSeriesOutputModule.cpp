/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * FIX ME: fix following comments
 * 2ds.
 * This program is similar to DataSeries's csv2ds utility, but handles
 * extents with different types and nullable fields and is primarily used
 * for converting system call csv traces.
 *
 * Usage: ./csv2ds <outputfile> <tablefile> <spec_string_file>
 *        <xml directory> <inputfiles...>
 *
 * <outputfile>: name of the dataseries output file
 * <tablefile>: name of the table file to refer to
 * <spec_string_file>: name of the file that contains a string
 *                    that specifies the format of the input file
 * <xml directory>: directory path that contains extent xml.
 *                 Remember to '/' should be the last character.
 * <inputfiles...>: input CSV files
 */

#include "DataSeriesOutputModule.hpp"

// Constructor to set up all extents and fields
DataSeriesOutputModule::DataSeriesOutputModule(std::ifstream &table_stream,
					       const std::string xml_dir,
					       const char *output_file) :
  ds_sink_(output_file), record_num_(0) {
  // Initialize config table
  initConfigTable(table_stream);

  // Registering extent types to the library
  ExtentTypeLibrary extent_type_library;

  // Set each extent's size to be 4096 bytes
  uint32_t extent_size = 4096;

  // Loop through each extent and create its fields from xmls
  for (config_table_type::iterator extent = config_table_.begin();
       extent != config_table_.end();
       extent++) {
    std::string extent_name = extent->first;

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

  // Creates mapping of sys call name to its corresponding args map function
  makeArgsMapFuncPtrTable();
}

/*
 * Inserts <syscall_name, address of syscall_args_map_func> pair
 * into func_ptr_map_.
 */
void DataSeriesOutputModule::makeArgsMapFuncPtrTable() {
  // access system call
  func_ptr_map_["access"] = &DataSeriesOutputModule::makeAccessArgsMap;
  // chdir system call
  func_ptr_map_["chdir"] = &DataSeriesOutputModule::makeChdirArgsMap;
  // chmod system call
  func_ptr_map_["chmod"] = &DataSeriesOutputModule::makeChmodArgsMap;
  // chown system call
  func_ptr_map_["chown"] = &DataSeriesOutputModule::makeChownArgsMap;
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
  // fcntl system call
  func_ptr_map_["fcntl"] = &DataSeriesOutputModule::makeFcntlArgsMap;
  // fstat system call
  func_ptr_map_["fstat"] = &DataSeriesOutputModule::makeFStatArgsMap;
  // fsync system call
  func_ptr_map_["fsync"] = &DataSeriesOutputModule::makeFsyncArgsMap;
  // getdents system call
  func_ptr_map_["getdents"] = &DataSeriesOutputModule::makeGetdentsArgsMap;
  // link system call
  func_ptr_map_["link"] = &DataSeriesOutputModule::makeLinkArgsMap;
  // lseek system call
  func_ptr_map_["lseek"] = &DataSeriesOutputModule::makeLSeekArgsMap;
  // lstat system call
  func_ptr_map_["lstat"] = &DataSeriesOutputModule::makeLStatArgsMap;
  // mkdir system call
  func_ptr_map_["mkdir"] = &DataSeriesOutputModule::makeMkdirArgsMap;
  // mknod system call
  func_ptr_map_["mknod"] = &DataSeriesOutputModule::makeMknodArgsMap;
  // mmap system call
  func_ptr_map_["mmap"] = &DataSeriesOutputModule::makeMmapArgsMap;
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
  // rename system call
  func_ptr_map_["rename"] = &DataSeriesOutputModule::makeRenameArgsMap;
  // rmdir system call
  func_ptr_map_["rmdir"] = &DataSeriesOutputModule::makeRmdirArgsMap;
  // stat system call
  func_ptr_map_["stat"] = &DataSeriesOutputModule::makeStatArgsMap;
  // symlink system call
  func_ptr_map_["symlink"] = &DataSeriesOutputModule::makeSymlinkArgsMap;
  // truncate system call
  func_ptr_map_["truncate"] = &DataSeriesOutputModule::makeTruncateArgsMap;
  // unlink system call
  func_ptr_map_["unlink"] = &DataSeriesOutputModule::makeUnlinkArgsMap;
  // unlinkat system call
  func_ptr_map_["unlinkat"] = &DataSeriesOutputModule::makeUnlinkatArgsMap;
  // utime system call
  func_ptr_map_["utime"] = &DataSeriesOutputModule::makeUtimeArgsMap;
  // utimes system call
  func_ptr_map_["utimes"] = &DataSeriesOutputModule::makeUtimesArgsMap;
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
  u_int var32_len;
  uint64_t time_called_Tfrac, time_returned_Tfrac;

  /*
   * Create a map from field names to field values.
   * Iterate through every possible fields (via table_).
   * If the field is in the map, then set value of the
   * field.  Otherwise set it to null.
   */

  sys_call_args_map["unique_id"] = &record_num_;
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
    SysCallArgsMapFuncPtr fxn = func_ptr_map_[extent_name];
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

  // Write values to the new record
  for (config_table_entry_type::iterator iter =
       config_table_[extent_name].begin();
       iter != config_table_[extent_name].end();
       iter++) {
    std::string field_name = iter->first;
    bool nullable = iter->second.first;
    var32_len = 0;

    if (sys_call_args_map.find(field_name) != sys_call_args_map.end()) {
      void *field_value = sys_call_args_map[field_name];
      /*
       * If field is of type Variable32, then retrieve the length of the
       * field that needs to be set.
       */
      if (extents_[extent_name][field_name].second == ExtentType::ft_variable32)
	var32_len = getVariable32FieldLength(sys_call_args_map, field_name);
      setField(extent_name, field_name, field_value, var32_len);

      continue;
    } else {
      if (nullable) {
	setFieldNull(extent_name, field_name);
      } else {
	std::cerr << extent_name << ":" << field_name << " ";
	std::cerr << "WARNING: Attempting to setNull to a non-nullable field. ";
	std::cerr << "This field will take on default value instead."
		  << std::endl;
      }
    }
  }

  // Update record number
  record_num_++;
}

// Destructor to delete the module
DataSeriesOutputModule::~DataSeriesOutputModule() {
  for (OutputModuleMap::iterator iter = modules_.begin();
       iter != modules_.end();
       iter++) {
    // iter->second is an OutputModule
    iter->second->flushExtent();
    iter->second->close();
    delete iter->second;
  }
}

// Initialize config table
void DataSeriesOutputModule::initConfigTable(std::ifstream &table_stream) {
  std::string line;

  /* Special case for Common fields */
  config_table_entry_type common_field_map;
  while (getline(table_stream, line)) {
    std::vector<std::string> split_data;
    boost::split(split_data, line, boost::is_any_of("\t"));

    if (split_data.size() != 4) {
      std::cout << "Illegal field table file" << std::endl;
      exit(1);
    }
    std::string extent_name = split_data[0];
    std::string field_name = split_data[1];
    std::string nullable_str = split_data[2];
    std::string field_type = split_data[3];

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
void DataSeriesOutputModule::setField(const std::string &extent_name,
				      const std::string &field_name,
				      void *field_value,
				      u_int var32_len) {
  bool buffer;
  switch (extents_[extent_name][field_name].second) {
  case ExtentType::ft_bool:
    buffer = (field_value != 0);
    doSetField<BoolField, bool>(extent_name, field_name, &buffer);
    break;
  case ExtentType::ft_byte:
    doSetField<ByteField, ExtentType::byte>(extent_name,
					    field_name,
					    field_value);
    break;
  case ExtentType::ft_int32:
    doSetField<Int32Field, ExtentType::int32>(extent_name,
					      field_name,
					      field_value);
    break;
  case ExtentType::ft_int64:
    doSetField<Int64Field, ExtentType::int64>(extent_name,
					      field_name,
					      field_value);
    break;
  case ExtentType::ft_double:
    doSetField<DoubleField, double>(extent_name,
				    field_name,
				    field_value);
    break;
  case ExtentType::ft_variable32:
    ((Variable32Field *)(extents_[extent_name][field_name].first))
      ->set((*(char **)field_value), var32_len);
    break;
  default:
    std::stringstream error_msg;
    error_msg << "Unsupported field type: "
	      << extents_[extent_name][field_name].second << std::endl;
    throw std::runtime_error(error_msg.str());
  }
}

/*
 * Set corresponding DS field to null
 */
void DataSeriesOutputModule::setFieldNull(const std::string &extent_name,
					  const std::string &field_name) {
  switch (extents_[extent_name][field_name].second) {
  case ExtentType::ft_bool:
    ((BoolField *)(extents_[extent_name][field_name].first))->setNull();
    break;
  case ExtentType::ft_byte:
    ((ByteField *)(extents_[extent_name][field_name].first))->setNull();
    break;
  case ExtentType::ft_int32:
    ((Int32Field *)(extents_[extent_name][field_name].first))->setNull();
    break;
  case ExtentType::ft_int64:
    ((Int64Field *)(extents_[extent_name][field_name].first))->setNull();
    break;
  case ExtentType::ft_double:
    ((DoubleField *)(extents_[extent_name][field_name].first))->setNull();
    break;
  case ExtentType::ft_variable32:
    ((Variable32Field *)(extents_[extent_name][field_name].first))->setNull();
    break;
  default:
    std::stringstream error_msg;
    error_msg << "Unsupported field type: "
	      << extents_[extent_name][field_name].second << std::endl;
    throw std::runtime_error(error_msg.str());
  }
}

template <typename FieldType, typename ValueType>
void DataSeriesOutputModule::doSetField(const std::string &extent_name,
					const std::string &field_name,
					void* field_value) {
  ((FieldType *)(extents_[extent_name][field_name].first))->set(
					*(ValueType *)field_value);
}

/*
 * Standard string functions does not work for buffer data that is read
 * or written.  Hence we cannot use strlen() in setField() function.
 * This function returns the length of variable32 type field.
 * NOTE: This function should be extended according to the field name of
 * system call as described in SNIA document.
 */
u_int DataSeriesOutputModule::getVariable32FieldLength(SysCallArgsMap &args_map,
						       const std::string
						       &field_name) {
  u_int length = 0;
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
	       (field_name == "dirent_buffer"))
      length = *(int *)(args_map["return_value"]);
  } else {
    std::cerr << "WARNING: field_name = " << field_name << " ";
    std::cerr << "is not set in the arguments map";
  }
  return length;
}

// Initialize all non-nullable boolean fields as False of given extent_name.
void DataSeriesOutputModule::initArgsMap(SysCallArgsMap &args_map,
					 const char *extent_name) {
  for (config_table_entry_type::iterator iter =
	config_table_[extent_name].begin();
	iter != config_table_[extent_name].end();
	iter++) {
    std::string field_name = iter->first;
    bool nullable = iter->second.first;
    if (!nullable &&
	extents_[extent_name][field_name].second == ExtentType::ft_bool)
      args_map[field_name] = 0;
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
  static bool true_ = true;
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

  if (v_args[0] != NULL) {
    args_map["data_read"] = &v_args[0];
  } else {
    std::cerr << "Read: Data to be read is set as NULL!!" << std::endl;
  }

  args_map["bytes_requested"] = &args[2];
}

void DataSeriesOutputModule::makeWriteArgsMap(SysCallArgsMap &args_map,
					      long *args,
					      void **v_args) {
  args_map["descriptor"] = &args[0];

  if (v_args[0] != NULL) {
    args_map["data_written"] = &v_args[0];
  } else {
    std::cerr << "Write: Data to be written is set as NULL!!" << std::endl;
  }

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
  static bool true_ = true;

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
  mode_t mode = processMode(args_map, args, 1);
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
  mode_t mode = processMode(args_map, args, 1);
  if (mode != 0) {
    std::cerr << "Chmod: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
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

  if (v_args[0] != NULL) {
    args_map["data_read"] = &v_args[0];
  } else {
    std::cerr << "PRead: Data to be read is set as NULL!!" << std::endl;
  }

  args_map["bytes_requested"] = &args[2];
  args_map["offset"] = &args[3];
}

void DataSeriesOutputModule::makePWriteArgsMap(SysCallArgsMap &args_map,
					       long *args,
					       void **v_args) {
  args_map["descriptor"] = &args[0];

  if (v_args[0] != NULL) {
    args_map["data_written"] = &v_args[0];
  } else {
    std::cerr << "PWrite: Data to be written is set as NULL!!" << std::endl;
  }

  args_map["bytes_requested"] = &args[2];
  args_map["offset"] = &args[3];
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

  if (v_args[1] != NULL) {
    args_map["link_value"] = &v_args[1];
  } else {
    std::cerr << "Readlink: Link value is set as NULL!" << std::endl;
  }

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
    if (v_args[2] != NULL)
      args_map["data_read"] = &v_args[2];
    else
      std::cerr << "Readv: Data to be read is set as NULL" << std::endl;
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
    if (v_args[2] != NULL)
      args_map["data_written"] = &v_args[2];
    else
      std::cerr << "Writev: Data to be written is set as NULL" << std::endl;
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

  if (v_args[1] != NULL) {
    struct utimbuf *times = (struct utimbuf *) v_args[1];

    // Convert the time_t members of the struct utimbuf to Tfracs (uint64_t)
    access_time_Tfrac = sec_to_Tfrac(times->actime);
    mod_time_Tfrac = sec_to_Tfrac(times->modtime);
  } else {
    // In the case of a NULL utimbuf, set access_time and mod_time equal to 0
    access_time_Tfrac = 0;
    mod_time_Tfrac = 0;
  }
  args_map["access_time"] = &access_time_Tfrac;
  args_map["mod_time"] = &mod_time_Tfrac;
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

  if (v_args[1] != NULL) {
    struct timeval *tv = (struct timeval *) v_args[1];

    // Convert timeval arguments to Tfracs (uint64_t)
    access_time_Tfrac = timeval_to_Tfrac(tv[0]);
    mod_time_Tfrac = timeval_to_Tfrac(tv[1]);
  } else {
    /*
     * In the case of a NULL timeval array, set access_time and
     * mod_time equal to 0.
     */
    access_time_Tfrac = 0;
    mod_time_Tfrac = 0;
  }
  args_map["access_time"] = &access_time_Tfrac;
  args_map["mod_time"] = &mod_time_Tfrac;
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
  static bool true_ = true;
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
  static bool true_ = true;

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
  static bool true_ = true;

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
  static bool true_ = true;
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

void DataSeriesOutputModule::makeGetdentsArgsMap(SysCallArgsMap &args_map,
						 long *args,
						 void **v_args) {
  args_map["descriptor"] = &args[0];
  if (v_args[0] != NULL) {
    args_map["dirent_buffer"] = &v_args[0];
  } else {
    std::cerr << "Getdents: Dirent buffer is set as NULL!!" << std::endl;
  }
  args_map["count"] = &args[2];
}
