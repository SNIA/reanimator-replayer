/*
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
  ds_sink_(output_file) {
  // Initialize record number
  record_num_ = (u_int *)malloc(sizeof(u_int));
  *record_num_ = 0;

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

  std::map<std::string, void *> sys_call_args_map;
  struct timeval tv_time_recorded;
  u_int var32_len;

  sys_call_args_map["unique_id"] = record_num_;
  /*
   * Create a map from field names to field values.
   * Iterate through every possible fields (via table_).
   * If the field is in the map, then set value of the
   * field.  Otherwise set it to null.
   */

  // Convert tv_time_called and tv_time_returned to Tfracs
  uint64_t time_called_Tfrac = timeval_to_Tfrac(
    *(struct timeval *) common_fields[DS_COMMON_FIELD_TIME_CALLED]);
  uint64_t time_returned_Tfrac = timeval_to_Tfrac(
    *(struct timeval *) common_fields[DS_COMMON_FIELD_TIME_RETURNED]);

  // Add the common field values to the map
  sys_call_args_map["time_called"] = &time_called_Tfrac;
  sys_call_args_map["time_returned"] = &time_returned_Tfrac;
  sys_call_args_map["return_value"] =
    common_fields[DS_COMMON_FIELD_RETURN_VALUE];
  sys_call_args_map["errno_number"] =
    common_fields[DS_COMMON_FIELD_ERRNO_NUMBER];
  sys_call_args_map["executing_pid"] =
    common_fields[DS_COMMON_FIELD_EXECUTING_PID];

  if (strcmp(extent_name, "close") == 0) {
    makeCloseArgsMap(sys_call_args_map, args);
  } else if (strcmp(extent_name, "open") == 0) {
    makeOpenArgsMap(sys_call_args_map, args, v_args);
  } else if (strcmp(extent_name, "read") == 0) {
    makeReadArgsMap(sys_call_args_map, args, v_args);
  } else if (strcmp(extent_name, "write") == 0) {
    makeWriteArgsMap(sys_call_args_map, args, v_args);
  } else if ((strcmp(extent_name, "chdir") == 0) ||
	     (strcmp(extent_name, "rmdir") == 0) ||
	     (strcmp(extent_name, "unlink") == 0)) {
    // Chdir, Rmdir, and Unlink have the same arguments (a pathname)
    makeChdirRmdirUnlinkArgsMap(sys_call_args_map, v_args);
  } else if ((strcmp(extent_name, "mkdir") == 0) ||
	     (strcmp(extent_name, "creat") == 0)) {
    makeMkdirCreatArgsMap(sys_call_args_map, args, v_args);
  } else if (strcmp(extent_name, "link") == 0) {
    makeLinkArgsMap(sys_call_args_map, v_args);
  } else if (strcmp(extent_name, "symlink") == 0) {
    makeSymlinkArgsMap(sys_call_args_map, v_args);
  } else if (strcmp(extent_name, "truncate") == 0) {
    makeTruncateArgsMap(sys_call_args_map, args, v_args);
  } else if (strcmp(extent_name, "access") == 0) {
    makeAccessArgsMap(sys_call_args_map, args, v_args);
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
  (*record_num_)++;
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
  delete record_num_;
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
u_int DataSeriesOutputModule::getVariable32FieldLength(std::map<std::string,
						       void *> &args_map,
						       const std::string
						       &field_name) {
  u_int length = 0;
  std::map<std::string, void *>::iterator it = args_map.find(field_name);
  if (it != args_map.end()) {
    /*
     * If field_name refers to the pathname passed as an argument to
     * the system call, string length function can be used to determine
     * the length.
     */
    if (field_name == "given_pathname") {
      void *field_value = args_map[field_name];
      length = strlen(*(char **)field_value);
    /*
     * If field_name refers to the actual data read or written, then length
     * of buffer must be the return value of that corresponding system call.
     */
    } else if (field_name == "data_read" || field_name == "data_written")
      length = *(int *)(args_map["return_value"]);
  } else {
    std::cerr << "WARNING: field_name = " << field_name << " ";
    std::cerr << "is not set in the arguments map";
  }
  return length;
}

// Initialize all non-nullable boolean fields as False of given extent_name.
void DataSeriesOutputModule::initArgsMap(std::map<std::string,
					 void *> &args_map,
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

void DataSeriesOutputModule::makeCloseArgsMap(std::map<std::string,
					      void *> &args_map,
					      long *args) {
  args_map["descriptor"] = &args[0];
}

void DataSeriesOutputModule::makeOpenArgsMap(std::map<std::string,
					     void *> &args_map,
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
void DataSeriesOutputModule::process_Flag_and_Mode_Args(std::map<std::string,
							void *> &args_map,
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
u_int DataSeriesOutputModule::processOpenFlags(std::map<std::string,
					       void *> &args_map,
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
 * 		       system call.
 */
mode_t DataSeriesOutputModule::processMode(std::map<std::string,
					   void *> &args_map,
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
  uint64_t time_Tfracs = (uint64_t)(time_seconds * (((uint64_t)1)<<32));
  return time_Tfracs;
}

void DataSeriesOutputModule::makeReadArgsMap(std::map<std::string,
					     void *> &args_map,
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

void DataSeriesOutputModule::makeWriteArgsMap(std::map<std::string,
					      void *> &args_map,
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

void DataSeriesOutputModule::makeChdirRmdirUnlinkArgsMap(std::map<std::string,
					      void *> &args_map,
					      void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Chdir/Rmdir/Unlink: Pathname is set as NULL!!" << std::endl;
  }
}

void DataSeriesOutputModule::makeMkdirCreatArgsMap(std::map<std::string,
					      void *> &args_map,
					      long *args,
					      void **v_args) {
  initArgsMap(args_map, "mkdir");
  int mode_offset = 1;
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Mkdir/Creat: Pathname is set as NULL!!" << std::endl;
  }
  mode_t mode = processMode(args_map, args, 1);
  if (mode != 0) {
    std::cerr << "Mkdir/Creat: these modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}

void DataSeriesOutputModule::makeLinkArgsMap(std::map<std::string,
					      void *> &args_map,
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

void DataSeriesOutputModule::makeSymlinkArgsMap(std::map<std::string,
						void *> &args_map,
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

void DataSeriesOutputModule::makeTruncateArgsMap(std::map<std::string,
						 void *> &args_map,
						 long *args,
						 void **v_args) {
  if (v_args[0] != NULL) {
    args_map["given_pathname"] = &v_args[0];
  } else {
    std::cerr << "Truncate: Pathname is set as NULL!!" << std::endl;
  }
  args_map["truncate_length"] = &args[1];
}

void DataSeriesOutputModule::makeAccessArgsMap(std::map<std::string,
					       void *> &args_map,
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
mode_t DataSeriesOutputModule::processAccessMode(std::map<std::string,
						 void *> &args_map,
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
