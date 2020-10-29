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
 * This file implements all the functions in the DataSeriesOutputModule.hpp
 * header file.
 *
 * Read the DataSeriesOutputModule.hpp file for more information about this
 * class.
 */

#include "DataSeriesOutputModule.hpp"

void DataSeriesOutputModule::makeWritevArgsMap(void **args_map, long *args,
                                               void **v_args) {
  int iov_number = *(int *)v_args[0];

  /*
   * iov_number equal to '-1' denotes the first record for the
   * writev system call. For first record, we save the file
   * descriptor, count, iov_number and total number of bytes
   * requested and do not set the iov_data_written field.
   */
  if (iov_number == -1) {
    args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
    args_map[SYSCALL_FIELD_COUNT] = &args[2];
    args_map[SYSCALL_FIELD_IOV_NUMBER] = v_args[0];
    args_map[SYSCALL_FIELD_BYTES_REQUESTED] = v_args[1];
  } else {
    /*
     * For rest of the records, we do not save file descriptor and
     * count fields. We only save the iov_number, bytes_requested
     * and iov_data_written fields.
     */
    args_map[SYSCALL_FIELD_IOV_NUMBER] = v_args[0];
    args_map[SYSCALL_FIELD_BYTES_REQUESTED] = v_args[1];
    args_map[SYSCALL_FIELD_IOV_DATA_WRITTEN] = &v_args[2];
  }
}
