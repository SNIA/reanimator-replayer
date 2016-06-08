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

#include "DataSeriesOutputModule.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Create DataSeries
 * return NULL if failed
 */
DataSeriesOutputModule *create_ds_module(const char *output_file, const char *table_file_name,
					 const char *xml_dir_path) {
  std::ifstream table_stream(table_file_name);
  std::string xml_dir(xml_dir_path);

  if (!table_stream.is_open()) {
    std::cout << "Could not open table file!\n";
    return NULL;
  }
  /* Create Tables and Fields */
  DataSeriesOutputModule *ds_module = new DataSeriesOutputModule(table_stream, xml_dir, output_file);
  return ds_module;
}

/*
 * Write a record into the DataSeries output file
 * return NULL if failed
 */
void *write_ds_record(DataSeriesOutputModule *ds_module, const char *extent_name,
		      long *args) {
  ((DataSeriesOutputModule *)ds_module)->writeRecord(extent_name, args);
}

/*
 * Free the module and flush all the records
 */
void *destroy_ds_module(DataSeriesOutputModule *ds_module) {
  delete (DataSeriesOutputModule *)ds_module;
}

/*
 * Save the string value of path_name passed as an argument to
 * system calls.
 */
void *save_path_string(DataSeriesOutputModule *ds_module, const char *path) {
  ((DataSeriesOutputModule *)ds_module)->fetch_path_string(path);
}

#ifdef __cplusplus
}
#endif
