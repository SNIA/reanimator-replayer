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

#include "strace2ds.h"

#include <cassert>

#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <DataSeries/ExtentType.hpp>
#include <DataSeries/DataSeriesFile.hpp>
#include <DataSeries/DataSeriesModule.hpp>

#include "FieldSpace.hpp"

/* map<fieldname, pair<nullable, ExtentType> */
typedef map<string, pair<bool, ExtentType::fieldType> > config_table_entry_type;
/* map<extentname, config_table_entry_type> */
typedef map<string,config_table_entry_type > config_table_type;
// map<extent name, OutputModule>
typedef std::map<std::string, OutputModule*> OutputModuleMap;

class DataSeriesWriteModule {
public:
  // Constructor to set up all extents and fields
  DataSeriesWriteModule(std::ifstream &table_stream,
		   const std::string xml_dir, const char *output_file);

  // Destructor to delete the module
  ~DataSeriesWriteModule();

private:
  // Disable copy constructor
  DataSeriesWriteModule(const DataSeriesWriteModule&);

  // Initialize config table
  void init_config_table(std::ifstream &table_stream);

  OutputModuleMap outputModuleSpace_;
  FieldSpace fieldSpace_;
  /* Sink is a wrapper for a DataSeries output file. */
  DataSeriesSink ds_sink_;
  config_table_type config_table_;
};

// Constructor to set up all extents and fields
DataSeriesWriteModule::DataSeriesWriteModule(std::ifstream &table_stream,
					     const std::string xml_dir,
					     const char *output_file) :
  ds_sink_(output_file) {
  // Initialize config table
  init_config_table(table_stream);

  // Registering extent types to the library
  ExtentTypeLibrary extent_type_library;

  // Set each extent's size to be 4096 bytes
  uint32_t extent_size = 4096;
  
  // Loop through each extent and create its fields from xmls
  for (config_table_type::iterator extent = config_table_.begin();
       extent != config_table_.end();
       extent++) {
    string extent_name = extent->first;

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
    outputModuleSpace_[extent_name] = new OutputModule(ds_sink_, *extent_series,
						   extent_type, extent_size);
    fieldSpace_.addFields(extent_name, *extent_series);
  }
  
  // Write out the extent type extent.
  ds_sink_.writeExtentLibrary(extent_type_library);
}

void DataSeriesWriteModule::init_config_table(std::ifstream &table_stream) {
  std::string line;

  /* Special case for Common fields */
  config_table_entry_type common_field_map;
  while (getline(table_stream, line)) {
    std::vector<string> split_data;
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
      common_field_map[field_name] = make_pair(nullable, ftype);
    else if (config_table_.find(extent_name) != config_table_.end())
      config_table_[extent_name][field_name] = make_pair(nullable, ftype);
    else { /* New extent detected */
      config_table_[extent_name] = common_field_map;
      config_table_[extent_name][field_name] = make_pair(nullable, ftype);
    }
  }
}

// Register the record and field values in string format into fields 
void DataSeriesWriteModule::write_record(const char *extent_name, int fd) {
  outputModuleSpace[extentname]->newRecord();
   /*
   * Create a map from field names to field values.
   * Iterate through every possible fields (via table_).
   * If the field is in the map, then set value of the
   * field. Otherwise set it to null.
   */
  /*
  std::map<std::string, std::string> fieldvaluemap;
  for (size_t i = 0; i != specs_[extentname].size(); i++) {
    string fieldname = specs_[extentname][i];
    string fieldvalue = fields[i+1];
    fieldvaluemap[fieldname] = fieldvalue;
  }
  
  for (config_table_entry_type::iterator iter = config_table_[extentname].begin();
       iter != config_table_[extentname].end();
       iter++) {
    string field_name = iter->first;
    bool nullable = iter->second.first;
    if (fieldvaluemap.find(fieldname) != fieldvaluemap.end()) {
      string fieldvalue = fieldvaluemap[fieldname];
      if (fieldvalue != "") {
	fieldSpace_.setField(extentname, fieldname, fieldvalue);
	continue;
      }
    }
    if (nullable)
      fieldSpace_.setNullField(extentname, fieldname);
    else if (!quietmode) {
      cerr << extentname << ":" << fieldname << " ";
      cerr << "WARNING: Attempting to setNull to a non-nullable field. ";
      cerr << "This field will take on default value instead." << endl;
    }
  }*/
}

DataSeriesWriteModule::~DataSeriesWriteModule() {
  for (OutputModuleMap::iterator iter = outputModuleSpace_.begin();
       iter != outputModuleSpace_.end();
       iter++) {
    iter->second->flushExtent();
    iter->second->close();
    delete iter->second;
  }
}

/*
 * Create DataSeries
 * return NULL if failed
 */
DataSeriesModule *create_ds_module(const char *output_file, const char *table_file_name,
				   const char *xml_dir_path) {
  std::ifstream table_stream(table_file_name);
  std::string xml_dir(xml_dir_path);

  if (!table_stream.is_open()) {
    std::cout << "Could not open table file!\n";
    return NULL;
  }

  /* Create Tables and Fields */
  DataSeriesWriteModule *ds_module = new DataSeriesWriteModule(table_stream, xml_dir, output_file);
  return (DataSeriesModule *)ds_module;
}

/*
 * Write a record into the DataSeries output file
 * return NULL if failed
 */
void *write_ds_record(DataSeriesModule *ds_module, const char *extent_name,
		      int fd) {
  ds_module->write_record(extent_name, fd);
}

/*
 * Free the module and flush all the records
 */
void *destroy_ds_module(DataSeriesModule *ds_module) {
  delete ds_module;
}

