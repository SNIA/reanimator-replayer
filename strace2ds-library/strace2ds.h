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

#ifndef STRACE2DS_H
#define STRACE2DS_H

#define DS_NUM_COMMON_FIELDS 5
#define DS_COMMON_FIELD_TIME_CALLED 0
#define DS_COMMON_FIELD_TIME_RETURNED 1
#define DS_COMMON_FIELD_RETURN_VALUE 2
#define DS_COMMON_FIELD_ERRNO_NUMBER 3
#define DS_COMMON_FIELD_EXECUTING_PID 4

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>

typedef struct DataSeriesOutputModule DataSeriesOutputModule;

/*
 * Create DataSeries
 * return NULL if failed
 */
DataSeriesOutputModule *ds_create_module(const char *output_file,
					 const char *table_file_name,
					 const char *xml_dir_path);

/*
 * Write a record into the DataSeries output file
 * return NULL if failed
 */
void ds_write_record(DataSeriesOutputModule *ds_module,
		     const char *extent_name,
		     long *args,
		     void *common_fields[DS_NUM_COMMON_FIELDS],
		     void **v_args);

/*
 * If the program attempts to trace a system call not supported
 * by the library, print a warning message.
 */
void ds_print_warning(const char *sys_call_name, long sys_call_number);

/*
 * Free the module and flush all the records
 */
void ds_destroy_module(DataSeriesOutputModule *ds_module);
#ifdef __cplusplus
}
#endif

#endif //STRACE2DS_H
