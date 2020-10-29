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

void DataSeriesOutputModule::makeStatfsArgsMap(void **args_map, long *args,
                                               void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "statfs");

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Statfs: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    if (!args_map[SYSCALL_FIELD_BUFFER_NOT_CAPTURED]) {
      struct statfs *statfsbuf = (struct statfs *)v_args[1];

      args_map[SYSCALL_FIELD_STATFS_RESULT_TYPE] = &statfsbuf->f_type;
      args_map[SYSCALL_FIELD_STATFS_RESULT_BSIZE] = &statfsbuf->f_bsize;
      args_map[SYSCALL_FIELD_STATFS_RESULT_BLOCKS] = &statfsbuf->f_blocks;
      args_map[SYSCALL_FIELD_STATFS_RESULT_BFREE] = &statfsbuf->f_bfree;
      args_map[SYSCALL_FIELD_STATFS_RESULT_BAVAIL] = &statfsbuf->f_bavail;
      args_map[SYSCALL_FIELD_STATFS_RESULT_FILES] = &statfsbuf->f_files;
      args_map[SYSCALL_FIELD_STATFS_RESULT_FFREE] = &statfsbuf->f_ffree;
      args_map[SYSCALL_FIELD_STATFS_RESULT_FSID] = &statfsbuf->f_fsid;
      args_map[SYSCALL_FIELD_STATFS_RESULT_NAMELEN] = &statfsbuf->f_namelen;
      args_map[SYSCALL_FIELD_STATFS_RESULT_FRSIZE] = &statfsbuf->f_frsize;
      args_map[SYSCALL_FIELD_STATFS_RESULT_FLAGS] = &statfsbuf->f_flags;

      u_int flag = processStatfsFlags(args_map, statfsbuf->f_flags);
      if (flag != 0) {
        std::cerr << "Statfs: These flag are not processed/unknown->0x";
        std::cerr << std::hex << flag << std::dec << std::endl;
      }
    }
  } else {
    std::cerr << "Statfs: Struct statfs is set as NULL!!" << std::endl;
  }
}
