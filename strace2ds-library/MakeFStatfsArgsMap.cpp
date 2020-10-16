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

void DataSeriesOutputModule::makeFStatfsArgsMap(void **args_map, long *args,
                                                void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "fstatfs");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  if (v_args[0] != NULL) {
    struct statfs *statfsbuf = (struct statfs *)v_args[0];

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
      std::cerr << "FStatfs: These flags are not processed/unknown->0x";
      std::cerr << std::hex << flag << std::dec << std::endl;
    }
  } else {
    std::cerr << "FStatfs: Struct statfs is set as NULL!!" << std::endl;
  }
}

u_int DataSeriesOutputModule::processStatfsFlags(void **args_map,
                                                 u_int statfs_flags) {
  /*
   * Process each individual statfs flag bit that has been set
   * in the argument stafs_flags.
   */
  // set mandatory lock flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_MANDLOCK,
                             SYSCALL_FIELD_STATFS_RESULT_FLAGS_MANDATORY_LOCK);
  // set no access time flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NOATIME,
                             SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_ACCESS_TIME);
  // set no dev flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NODEV,
                             SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_DEV);
  // set no directory access time flag
  process_Flag_and_Mode_Args(
      args_map, statfs_flags, ST_NODIRATIME,
      SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_DIRECTORY_ACCESS_TIME);
  // set no exec flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NOEXEC,
                             SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_EXEC);
  // set no set uid flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_NOSUID,
                             SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_SET_UID);
  // set read only flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_RDONLY,
                             SYSCALL_FIELD_STATFS_RESULT_FLAGS_READ_ONLY);
  // set relative access time flag
  process_Flag_and_Mode_Args(
      args_map, statfs_flags, ST_RELATIME,
      SYSCALL_FIELD_STATFS_RESULT_FLAGS_RELATIVE_ACCESS_TIME);
  // set synchronous flag
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_SYNCHRONOUS,
                             SYSCALL_FIELD_STATFS_RESULT_FLAGS_SYNCHRONOUS);
  // set valid flag (f_flags support is implemented)
  process_Flag_and_Mode_Args(args_map, statfs_flags, ST_VALID,
                             SYSCALL_FIELD_STATFS_RESULT_FLAGS_VALID);

  /*
   * Return remaining statfs flags so that caller can
   * warn of unknown flags if the statfs_flags is not set
   * as zero.
   */
  return statfs_flags;
}
