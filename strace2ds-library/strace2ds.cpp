/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the strace2ds.h header file.
 *
 * Read strace2ds.h file for more information about this file.
 */

#include "strace2ds.h"

#include "DataSeriesOutputModule.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void ds_write_two_umask_records(DataSeriesOutputModule *ds_module, int pid);
/*
 * Create DataSeries
 * return NULL if failed
 */
DataSeriesOutputModule *ds_create_module(const char *output_file,
					 const char *table_file_name,
					 const char *xml_dir_path) {
  std::ifstream table_stream(table_file_name);
  std::string xml_dir(xml_dir_path);

  if (!table_stream.is_open()) {
    std::cout << "Could not open table file!\n";
    return NULL;
  }
  /* Create Tables and Fields */
  DataSeriesOutputModule *ds_module = new DataSeriesOutputModule(table_stream,
								 xml_dir,
								 output_file);
  return ds_module;
}

/*
 * Traced application has a default mask value that is different from the replayer.
 * Therefore, we need to call umask twice in here. First call is to get the
 * original value of mask value, and second call is to restore the mask
 * value. Write one record at the very beginning of DataSeries output file.
 * By doing so, replayer will set its mask value to be same as the traced application.
 * This function simulates the behavior of strace trace_syscall_exiting function. Note that we can't
 * call trace_syscall_exiting directly because strace does its
 * own things in trace_syscall_exiting function.
 */
void ds_write_two_umask_records(DataSeriesOutputModule *ds_module, int pid) {
  struct timeval etime; /* Syscall entry time */
  struct timeval dtime; /* Syscall departure time */
  long args[1]; /* umask args */
  const char *syscall_name = "umask";
  void *common_fields[DS_NUM_COMMON_FIELDS];
  // Initialize common_fields with NULL arguments.
  memset(common_fields, 0, sizeof(void *) * DS_NUM_COMMON_FIELDS);

  // Get a time stamp for time_called.
  gettimeofday(&etime, NULL);
  // Call umask to get the original value of mask value
  mode_t orig_mask = umask(0);
  // Get a time stamp for time_returned.
  gettimeofday(&dtime, NULL);
  // Call umask again to restore the original value of mask value
  umask(orig_mask);
  args[0] = orig_mask;
  mode_t ret_val = 0;

  /* Then, store the common field values */
  common_fields[DS_COMMON_FIELD_TIME_CALLED] = &etime;
  common_fields[DS_COMMON_FIELD_TIME_RETURNED] = &dtime;
  common_fields[DS_COMMON_FIELD_RETURN_VALUE] = &ret_val;
  common_fields[DS_COMMON_FIELD_ERRNO_NUMBER] = &errno;
  common_fields[DS_COMMON_FIELD_EXECUTING_PID] = &pid;

  // Now everything is set. Just call ds_write_record to write the umask record.
  ds_write_record(ds_module, syscall_name, args, common_fields, NULL);
}

/*
 * Write a record into the DataSeries output file
 * return NULL if failed
 */
void ds_write_record(DataSeriesOutputModule *ds_module,
		     const char *extent_name,
		     long *args,
		     void *common_fields[DS_NUM_COMMON_FIELDS],
		     void **v_args) {

  ((DataSeriesOutputModule *)ds_module)->writeRecord(extent_name, args,
						     common_fields, v_args);
}

/*
 * If the program attempts to trace a system call not supported
 * by the library, print a warning message.
 */
void ds_print_warning(const char *sys_call_name, long sys_call_number) {
  std::cerr << "WARNING: Attempting to trace unsupported system call: "
	    << sys_call_name << " (" << sys_call_number << ")" << std::endl;
}

/*
 * Record the size of the buffer passed to an ioctl system call
 */
void ds_set_ioctl_size(DataSeriesOutputModule *ds_module, int size) {
  ((DataSeriesOutputModule *) ds_module)->setIoctlSize(size);
}

/*
 * Return the size of the buffer passed to an ioctl system call
 */
int ds_get_ioctl_size(DataSeriesOutputModule *ds_module) {
  ((DataSeriesOutputModule *) ds_module)->getIoctlSize();
}

/*
 * Record the index of the child thread id argument passed to a clone
 * system call, since the argument order for clone differs for different
 * architectures
 */
void ds_set_clone_ctid_index(DataSeriesOutputModule *ds_module,
			     unsigned int ctid_index) {
  ((DataSeriesOutputModule *) ds_module)->setCloneCTIDIndex(ctid_index);
}

/*
 * Return the index of the child thread id argument passed to a clone
 * system call
 */
unsigned int ds_get_clone_ctid_index(DataSeriesOutputModule *ds_module) {
  ((DataSeriesOutputModule *) ds_module)->getCloneCTIDIndex();
}

/*
 * Free the module and flush all the records
 */
void ds_destroy_module(DataSeriesOutputModule *ds_module) {
  delete (DataSeriesOutputModule *)ds_module;
}
#ifdef __cplusplus
}
#endif
