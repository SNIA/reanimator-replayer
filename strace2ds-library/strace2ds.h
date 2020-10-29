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
 * This program is a wrapper class which allows to link the C++ program
 * (the DataSeries's APIs) to a C program (the strace code). It acts as
 * a glue so that C++ functions are made callable from C code.
 */

#ifndef STRACE2DS_H
#define STRACE2DS_H

#define DS_NUM_COMMON_FIELDS 9
#define DS_COMMON_FIELD_TIME_CALLED 0
#define DS_COMMON_FIELD_TIME_RETURNED 1
#define DS_COMMON_FIELD_RETURN_VALUE 2
#define DS_COMMON_FIELD_ERRNO_NUMBER 3
#define DS_COMMON_FIELD_EXECUTING_PID 4
#define DS_COMMON_FIELD_EXECUTING_TID 5
#define DS_COMMON_FIELD_UNIQUE_ID 6
#define DS_COMMON_FIELD_SYSCALL_NUM 7
#define DS_COMMON_FIELD_BUFFER_NOT_CAPTURED 8

#define DS_FILE_TYPE_REG 0
#define DS_FILE_TYPE_CHR 1
#define DS_FILE_TYPE_BLK 2
#define DS_FILE_TYPE_FIFO 3
#define DS_FILE_TYPE_SOCK 4

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
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
void ds_write_umask_at_start(DataSeriesOutputModule *ds_module, int pid);

/*
 * Write a record into the DataSeries output file.
 * We are incrementing record number atomically.
 */
void ds_write_record(DataSeriesOutputModule *ds_module, const char *extent_name,
                     long *args, void *common_fields[DS_NUM_COMMON_FIELDS],
                     void **v_args);

/*
 * Write data into the same record in the DataSeries output file.
 * Note: We are not incrementing record number here.
 */
void ds_write_into_same_record(DataSeriesOutputModule *ds_module,
                               const char *extent_name, long *args,
                               void *common_fields[DS_NUM_COMMON_FIELDS],
                               void **v_args);

/*
 * If the program attempts to trace a system call not supported
 * by the library, print a warning message.
 */
void ds_print_warning(DataSeriesOutputModule *ds_module,
                      const char *sys_call_name, long sys_call_number);

/*
 * If a program, attempts to trace a system call which are chosen
 * to be ignored while replaying, maintain a set of untraced syscall.
 */
void ds_add_to_untraced_set(DataSeriesOutputModule *ds_module,
                            const char *sys_call_name, long sys_call_number);

/*
 * Record the size of the buffer passed to an ioctl system call
 */
void ds_set_ioctl_size(DataSeriesOutputModule *ds_module, int size);

/*
 * Return the size of the buffer passed to an ioctl system call
 */
int ds_get_ioctl_size(DataSeriesOutputModule *ds_module);

/*
 * Return the next record number in this dataseries file
 */
uint64_t ds_get_next_id(DataSeriesOutputModule *ds_module);

/*
 * Record the index of the child thread id argument passed to a clone
 * system call, since the argument order for clone differs for different
 * architectures
 */
void ds_set_clone_ctid_index(DataSeriesOutputModule *ds_module,
                             unsigned int ctid_index);

/*
 * Return the index of the child thread id argument passed to a clone
 * system call
 */
unsigned int ds_get_clone_ctid_index(DataSeriesOutputModule *ds_module);

/*
 * Free the module and flush all the records
 */
void ds_destroy_module(DataSeriesOutputModule *ds_module);
#ifdef __cplusplus
}
#endif

#endif  // STRACE2DS_H
