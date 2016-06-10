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
bool DataSeriesOutputModule::writeRecord(const char *extent_name, long *args, struct timeval time_called, struct timeval time_returned, int return_value, int errno_number, int executing_pid) {
  std::map<std::string, void *> sys_call_args_map;
  struct timeval time_recorded;

  sys_call_args_map["unique_id"] = &record_num_;
  /*
   * Create a map from field names to field values.
   * Iterate through every possible fields (via table_).
   * If the field is in the map, then set value of the
   * field. Otherwise set it to null.
   */

  // Converts the timeval time_called to a double
  double time_called_doub = (double) time_called.tv_sec + pow(10.0, -6)*time_called.tv_usec;
  // Converts the timeval time_returned to a uint64_t
  uint64_t time_returned_u64 = (uint64_t)(((double) time_returned.tv_sec + pow(10.0, -6)*time_returned.tv_usec)*(((uint64_t)1)<<32));

  // Adds the common field values to the map
  sys_call_args_map["time_called"] = &time_called_doub;
  sys_call_args_map["time_returned"] = &time_returned_u64;
  sys_call_args_map["return_value"] = &return_value;
  sys_call_args_map["errno_number"] = &errno_number;
  sys_call_args_map["executing_pid"] = &executing_pid;

  if (strcmp(extent_name, "close") == 0) {
    makeCloseArgsMap(sys_call_args_map, args);
  } else if (strcmp(extent_name, "open") == 0) {
    makeOpenArgsMap(sys_call_args_map, args);
  }
  // Create a new record to write
  modules_[extent_name]->newRecord();
 
  // Get the time the record was written as late as possible before we actually write the record
  gettimeofday(&time_recorded, NULL);
  // Convert the timeval time_recorded to a uint_64 and add it to the map
  uint64_t time_recorded_u64 = (uint64_t)(((double) time_recorded.tv_sec + pow(10.0, -6)*time_recorded.tv_usec)*(((uint64_t)1)<<32));
  sys_call_args_map["time_recorded"] = &time_recorded_u64;

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
