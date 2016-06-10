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
#include <fcntl.h>

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
      std::cout << extent_name << ": Coud not open xml file!\n";
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

// Register the record and field values in into DS fields 
bool DataSeriesOutputModule::writeRecord(const char *extent_name, long *args) {
  std::map<std::string, void *> sys_call_args_map;

  sys_call_args_map["unique_id"] = &record_num_;
  /*
   * Create a map from field names to field values.
   * Iterate through every possible fields (via table_).
   * If the field is in the map, then set value of the
   * field. Otherwise set it to null.
   */
  if (strcmp(extent_name, "close") == 0) {
    makeCloseArgsMap(sys_call_args_map, args);
  } else if (strcmp(extent_name, "open") == 0) {
    makeOpenArgsMap(sys_call_args_map, args);
  }

  // Create a new record to write
  modules_[extent_name]->newRecord();

  // Write values to the new record
  for (config_table_entry_type::iterator iter = config_table_[extent_name].begin();
       iter != config_table_[extent_name].end();
       iter++) {
    std::string field_name = iter->first;
    bool nullable = iter->second.first;

    if (sys_call_args_map.find(field_name) != sys_call_args_map.end()) {
      void *field_value = sys_call_args_map[field_name];
      setField(extent_name, field_name, field_value);
      continue;
    } else {
      if (nullable) {
	setFieldNull(extent_name, field_name);
      } else {
	std::cerr << extent_name << ":" << field_name << " ";
	std::cerr << "WARNING: Attempting to setNull to a non-nullable field. ";
	std::cerr << "This field will take on default value instead." << std::endl;
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
    // iter->second is a OutputModule
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
    else if (field_type == "int64" or field_type=="time")
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

// Add a extent(system call)
void DataSeriesOutputModule::addExtent(const std::string &extent_name,
				       ExtentSeries &series) { 
  const ExtentType::Ptr extent_type = series.getTypePtr();
  for (uint32_t i = 0; i < extent_type->getNFields(); i++) {
    const std::string &field_name = extent_type->getFieldName(i);
    bool nullable = extent_type->getNullable(field_name);

    switch ((ExtentType::fieldType) extent_type->getFieldType(field_name)) {
    case ExtentType::ft_bool:
      addField(extent_name, field_name, new BoolField(series, field_name, nullable),
	       ExtentType::ft_bool);
      break;
    case ExtentType::ft_byte:
      addField(extent_name, field_name, new ByteField(series, field_name, nullable),
	       ExtentType::ft_byte);
      break;
    case ExtentType::ft_int32:
      addField(extent_name, field_name, new Int32Field(series, field_name, nullable),
	       ExtentType::ft_int32);
      break;
    case ExtentType::ft_int64:
      addField(extent_name, field_name, new Int64Field(series, field_name, nullable),
	       ExtentType::ft_int64);
      break;
    case ExtentType::ft_double:
      addField(extent_name, field_name, new DoubleField(series, field_name, nullable),
	       ExtentType::ft_double);
      break;
    case ExtentType::ft_variable32:
      addField(extent_name, field_name, new Variable32Field(series, field_name, nullable),
	       ExtentType::ft_variable32);
      break;
    default:
      std::stringstream error_msg;
      error_msg << "Unsupported field type: " << extent_type->getFieldType(field_name) << std::endl;
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
				      void *field_value) {
  bool buffer;
  switch (extents_[extent_name][field_name].second) {
  case ExtentType::ft_bool:
    if (field_value == 0)
      buffer = false;
    else
      buffer = true;
    doSetField<BoolField, bool>(extent_name, field_name, &buffer);
    break;
  case ExtentType::ft_byte:
    doSetField<ByteField, ExtentType::byte>(extent_name, field_name, field_value);
    break;
  case ExtentType::ft_int32:
    doSetField<Int32Field, ExtentType::int32>(extent_name, field_name, field_value);
    break;
  case ExtentType::ft_int64:
    doSetField<Int64Field, ExtentType::int64>(extent_name, field_name, field_value);
    break;
  case ExtentType::ft_double:
    doSetField<DoubleField, double>(extent_name, field_name, field_value);
    break;
  case ExtentType::ft_variable32:
    ((Variable32Field *)(extents_[extent_name][field_name].first))->set((*(std::string *)field_value).c_str(),
									(*(std::string *)field_value).size()+1);
    break;
  default:
    std::stringstream error_msg;
    error_msg << "Unsupported field type: " << extents_[extent_name][field_name].second << std::endl;
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
    error_msg << "Unsupported field type: " << extents_[extent_name][field_name].second << std::endl;
    throw std::runtime_error(error_msg.str());
  }
}

template <typename FieldType, typename ValueType>
void DataSeriesOutputModule::doSetField(const std::string &extent_name,
					const std::string &field_name,
					void* field_value) {
  ((FieldType *)(extents_[extent_name][field_name].first))->set(*(ValueType *)field_value);
}

void DataSeriesOutputModule::fetch_path_string(const char *path) {
  // Save the path string obtained from strace.
  path_string = path;
}

void DataSeriesOutputModule::makeCloseArgsMap(std::map<std::string, void *> &args_map, long *args) {
  args_map["descriptor"] = &args[0];
}

void DataSeriesOutputModule::makeOpenArgsMap(std::map<std::string, void *> &args_map, long *args) {
  int offset = 0;
  if (!path_string.empty()) {
    args_map["given_pathname"] = &path_string;
  } else {
    std::cerr << "Open: Pathname is set as NULL." << std::endl;
    exit(1);
  }

  /* Setting flag values */
  args_map["open_value"] = &args[offset + 1];
  if (!processOpenFlags(args_map, args[offset + 1])) {
    std::cerr << "Open: Unknown flag bits are set" << std::endl;
    exit(1);
  }

  /*
   * The mode argument for open system call is optional.
   * If open system call is called with only two argumnets,
   * all the mode bits are set as False and we do not need
   * to call processMode() function.
   */
  args_map["mode_uid"] = 0;
  args_map["mode_gid"] = 0;
  args_map["mode_sticky_bit"] = 0;
  args_map["mode_R_user"] = 0;
  args_map["mode_W_user"] = 0;
  args_map["mode_X_user"] = 0;
  args_map["mode_R_group"] = 0;
  args_map["mode_W_group"] = 0;
  args_map["mode_X_group"] = 0;
  args_map["mode_R_others"] = 0;
  args_map["mode_W_others"] = 0;
  args_map["mode_X_others"] = 0;

  /*
   * If only, open is called with 3 arguments, set the corresponding
   * mode value and mode bits as True.
   */
  if (args[offset + 1] & O_CREAT) {
    if (!processMode(args_map, args, offset + 2)) {
      std::cerr << "Open:: Unknown mode bits are set" << std::endl;
      exit(1);
    }
  }
}

/*
 * This function unwraps the flag value passed as an argument to
 * open system call and set the corresponding flag values as True.
 */
bool DataSeriesOutputModule::processOpenFlags(std::map<std::string, void *> &args_map,
						unsigned int flag) {

  /*
   * Intially set all access mode bits as False.
   */
  args_map["flag_read_only"] = 0;
  args_map["flag_write_only"] = 0;
  args_map["flag_read_and_write"] = 0;

  /*
   * First check which access mode: O_RDONLY, O_WRONLY, O_RDWR
   * has been included in the argument flag.
   */

  if (flag & O_RDONLY) {
  // read only permission
    args_map["flag_read_only"] = (void *) 1;
    flag &= ~O_RDONLY;
  } else if (flag & O_WRONLY) {
  // write only permission
    args_map["flag_write_only"] = (void *) 1;
    flag &= ~O_WRONLY;
  } else if (flag & O_RDWR) {
  // read write permission
    args_map["flag_read_and_write"] = (void *) 1;
    flag &= ~O_RDWR;
  }

  /*
   * In addition, check for more file creation and file status flags
   * such as O_CREAT, O_DIRECTORY, etc that has been set or not.
   */
  if (flag & O_APPEND) {
    args_map["flag_append"] = (void *) 1;
    flag &= ~O_APPEND;
  } else
    args_map["flag_append"] = 0;

  if (flag & O_ASYNC) {
    args_map["flag_async"] = (void *) 1;
    flag &= ~O_ASYNC;
  } else
    args_map["flag_async"] = 0;

  if (flag & O_CLOEXEC) {
    args_map["flag_close_on_exec"] = (void *) 1;
    flag &= ~O_CLOEXEC;
  } else
    args_map["flag_close_on_exec"] = 0;

  if (flag & O_CREAT) {
    args_map["flag_create"] = (void *) 1;
    flag &= ~O_CREAT;
  } else
    args_map["flag_create"] = 0;

  if (flag & O_DIRECT) {
    args_map["flag_direct"] = (void *) 1;
    flag &= ~O_DIRECT;
  } else
    args_map["flag_direct"] = 0;

  if (flag & O_DIRECTORY) {
    args_map["flag_directory"] = (void *) 1;
    flag &= ~O_DIRECTORY;
  } else
    args_map["flag_directory"] = 0;

  if (flag & O_EXCL) {
    args_map["flag_exclusive"] = (void *) 1;
    flag &= ~O_EXCL;
  } else
    args_map["flag_exclusive"] = 0;

  if (flag & O_LARGEFILE) {
    args_map["flag_largefile"] = (void *) 1;
    flag &= ~O_LARGEFILE;
  } else
    args_map["flag_largefile"] = 0;

  if (flag & O_NOATIME) {
    args_map["flag_no_access_time"] = (void *) 1;
    flag &= ~O_NOATIME;
  } else
    args_map["flag_no_access_time"] = 0;

  if (flag & O_NOCTTY) {
    args_map["flag_no_controlling_terminal"] = (void *) 1;
    flag &= ~O_NOCTTY;
  } else
    args_map["flag_no_controlling_terminal"] = 0;

  if (flag & O_NOFOLLOW) {
    args_map["flag_no_follow"] = (void *) 1;
    flag &= ~O_NOFOLLOW;
  } else
    args_map["flag_no_follow"] = 0;

  if (flag & O_NONBLOCK) {
    args_map["flag_no_blocking_mode"] = (void *) 1;
    flag &= ~O_NONBLOCK;
  } else
    args_map["flag_no_blocking_mode"] = 0;

  if (flag & O_NDELAY) {
    args_map["flag_no_delay"] = (void *) 1;
    flag &= ~O_NDELAY;
  } else
    args_map["flag_no_delay"] = 0;

  if (flag & O_SYNC) {
    args_map["flag_synchronous"] = (void *) 1;
    flag &= ~O_SYNC;
  } else
    args_map["flag_synchronous"] = 0;

  if (flag & O_TRUNC) {
    args_map["flag_truncate"] = (void *) 1;
    flag &= ~O_TRUNC;
  } else
    args_map["flag_truncate"] = 0;

  /*
   * Finally check if the value of flag is now zero or not.
   * If the value of flag is not set as zero, unknown flag
   * bit is set.
   */
  if (flag == 0)
    return true;
  else
    return false;
}

/*
 * This function unwraps the mode value passed as an argument to system
 * call.
 */
bool DataSeriesOutputModule::processMode(std::map<std::string, void *> &args_map,
                                         long *args, int offset) {
  // Save the mode argument with mode_value file in map
  args_map["mode_value"] = &args[offset];

  // Initially set the individual mode bits to False.
  const char *mode_field_name[] = {"mode_uid", "mode_gid", "mode_sticky_bit", \
				   "mode_R_user", "mode_W_user", "mode_X_user", \
				   "mode_R_group", "mode_W_group", "mode_X_group", \
				   "mode_R_others", "mode_W_others", "mode_X_others"};

  for (int i = 0; i < 12; i++) {
    args_map[mode_field_name[i]] = 0;
  }

  mode_t mode = args[offset];

  // set-user-ID bit
  if (mode & S_ISUID) {
    args_map["mode_uid"] = (void *) 1;
    mode &= ~S_ISUID;
  }

  // set-group-ID bit
  if (mode & S_ISGID) {
    args_map["mode_gid"] = (void *) 1;
    mode &= ~S_ISGID;
  }

  // sticky bit
  if (mode & S_ISVTX) {
    args_map["mode_sticky_bit"] = (void *) 1;
    mode &= ~S_ISVTX;
  }

  // user read permission bit
  if (mode & S_IRUSR) {
    args_map["mode_R_user"] = (void *) 1;
    mode &= ~S_IRUSR;
  }

  // user write permission bit
  if (mode & S_IWUSR) {
    args_map["mode_W_user"] = (void *) 1;
    mode &= ~S_IWUSR;
  }

  // user execute permission bit
  if (mode & S_IXUSR) {
    args_map["mode_X_user"] = (void *) 1;
    mode &= ~S_IXUSR;
  }

  // group read permission bit
  if (mode & S_IRGRP) {
    args_map["mode_R_group"] = (void *) 1;
    mode &= ~S_IRGRP;
  }

  // group write permission bit
  if (mode & S_IWGRP) {
    args_map["mode_W_group"] = (void *) 1;
    mode &= ~S_IWGRP;
  }

  // group execute permission bit
  if (mode & S_IXGRP) {
    args_map["mode_X_others"] = (void *) 1;
    mode &= ~S_IXGRP;
  }

  // others read permission bit
  if (mode & S_IROTH) {
    args_map["mode_R_others"] = (void *) 1;
    mode &= ~S_IROTH;
  }

  // others write permission bit
  if (mode & S_IWOTH) {
    args_map["mode_W_others"] = (void *) 1;
    mode &= ~S_IWOTH;
  }

  // others execute permission bit
  if (mode & S_IXOTH) {
    args_map["mode_X_others"] = (void *) 1;
    mode &= ~S_IXOTH;
  }

  if (mode == 0)
    return true;
  else
    return false;
}
