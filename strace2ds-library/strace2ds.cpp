/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
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
#include <syscall.h>

#include "DataSeriesOutputModule.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define UMASK_SYSCALL_NUM SYS_umask

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
  DataSeriesOutputModule *ds_module =
      new DataSeriesOutputModule(table_stream, xml_dir, output_file);
  return ds_module;
}

/*
 * Traced application has a default mask value that is different from the
 * replayer.
 * Therefore, we need to call umask twice in here. First call is to get the
 * original value of mask value, and second call is to restore the mask
 * value. Write one record at the very beginning of DataSeries output file.
 * By doing so, replayer will set its mask value to be same as the traced
 * application.
 * This function simulates the behavior of strace trace_syscall_exiting
 * function. Note that we can't
 * call trace_syscall_exiting directly because strace does its
 * own things in trace_syscall_exiting function.
 */
void ds_write_umask_at_start(DataSeriesOutputModule *ds_module, int pid) {
  struct timeval etime; /* Syscall entry time */
  struct timeval dtime; /* Syscall departure time */
  long args[1];         /* umask args */
  int unique_id = 0;    /* unique id of this umask record*/
  const char *syscall_name = "umask";
  uint64_t syscall_num = UMASK_SYSCALL_NUM;
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
  common_fields[DS_COMMON_FIELD_UNIQUE_ID] = &unique_id;
  common_fields[DS_COMMON_FIELD_SYSCALL_NUM] = &syscall_num;
  common_fields[DS_COMMON_FIELD_BUFFER_NOT_CAPTURED] = (void *)false;

  /*
   * Now everything is set. Just call ds_write_record to write one umask record
   * at the very beginning of DataSeries file.
   */
  ds_write_record(ds_module, syscall_name, args, common_fields, NULL);
}

/*
 * Write a record into the DataSeries output file.
 * We are incrementing record number atomically.
 */
void ds_write_record(DataSeriesOutputModule *ds_module, const char *extent_name,
                     long *args, void *common_fields[DS_NUM_COMMON_FIELDS],
                     void **v_args) {
  uint64_t unique_id = ds_module->getNextID();
  common_fields[DS_COMMON_FIELD_UNIQUE_ID] = &unique_id;
  ds_module->writeRecord(extent_name, args, common_fields, v_args);
}

/*
 * Write data into the same record in the DataSeries output file.
 * Note: We are not incrementing record number here.
 */
void ds_write_into_same_record(DataSeriesOutputModule *ds_module,
                               const char *extent_name, long *args,
                               void *common_fields[DS_NUM_COMMON_FIELDS],
                               void **v_args) {
  ds_module->writeRecord(extent_name, args, common_fields, v_args);
}

/*
 * If the program attempts to trace a system call not supported
 * by the library, print a warning message.
 */
void ds_print_warning(DataSeriesOutputModule *ds_module,
                      const char *sys_call_name, long sys_call_number) {
  ((DataSeriesOutputModule *)ds_module)
      ->untraced_sys_call_counts_[sys_call_number]++;
  std::cerr << "WARNING: Attempting to trace unsupported system call: "
            << sys_call_name << " (" << sys_call_number << ")" << std::endl;
}

/*
 * If a program, attempts to trace a system call which are chosen
 * to be ignored while replaying, maintain a set of untraced
 * system calls along with their count.
 */
void ds_add_to_untraced_set(DataSeriesOutputModule *ds_module,
                            const char *sys_call_name, long sys_call_number) {
  if (!((DataSeriesOutputModule *)ds_module)
           ->untraced_sys_call_counts_[sys_call_number]) {
    ((DataSeriesOutputModule *)ds_module)
        ->untraced_sys_call_counts_[sys_call_number] = 1;
    std::cerr << "WARNING: Ignoring to replay system call: " << sys_call_name
              << " (" << sys_call_number << ")" << std::endl;
  } else {
    ((DataSeriesOutputModule *)ds_module)
        ->untraced_sys_call_counts_[sys_call_number]++;
  }
}

/*
 * Record the size of the buffer passed to an ioctl system call
 */
void ds_set_ioctl_size(DataSeriesOutputModule *ds_module, int size) {
  ((DataSeriesOutputModule *)ds_module)->setIoctlSize(size);
}

/*
 * Return the size of the buffer passed to an ioctl system call
 */
int ds_get_ioctl_size(DataSeriesOutputModule *ds_module) {
  return ((DataSeriesOutputModule *)ds_module)->getIoctlSize();
}

/*
 * Return the next record number in this dataseries file
 */
uint64_t ds_get_next_id(DataSeriesOutputModule *ds_module) {
  return ds_module->getNextID();
}

/*
 * Record the index of the child thread id argument passed to a clone
 * system call, since the argument order for clone differs for different
 * architectures
 */
void ds_set_clone_ctid_index(DataSeriesOutputModule *ds_module,
                             unsigned int ctid_index) {
  ((DataSeriesOutputModule *)ds_module)->setCloneCTIDIndex(ctid_index);
}

/*
 * Return the index of the child thread id argument passed to a clone
 * system call
 */
unsigned int ds_get_clone_ctid_index(DataSeriesOutputModule *ds_module) {
  return ((DataSeriesOutputModule *)ds_module)->getCloneCTIDIndex();
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
