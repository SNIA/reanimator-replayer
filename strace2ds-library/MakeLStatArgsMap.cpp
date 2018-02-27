/*
 * Copyright (c) 2016-2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
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

void DataSeriesOutputModule::makeLStatArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "LStat: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    struct stat *statbuf = (struct stat *) v_args[1];

    args_map[SYSCALL_FIELD_STAT_RESULT_DEV] = &statbuf->st_dev;
    args_map[SYSCALL_FIELD_STAT_RESULT_INO] = &statbuf->st_ino;
    args_map[SYSCALL_FIELD_STAT_RESULT_MODE] = &statbuf->st_mode;
    args_map[SYSCALL_FIELD_STAT_RESULT_NLINK] = &statbuf->st_nlink;
    args_map[SYSCALL_FIELD_STAT_RESULT_UID] = &statbuf->st_uid;
    args_map[SYSCALL_FIELD_STAT_RESULT_GID] = &statbuf->st_gid;
    args_map[SYSCALL_FIELD_STAT_RESULT_RDEV] = &statbuf->st_rdev;
    args_map[SYSCALL_FIELD_STAT_RESULT_SIZE] = &statbuf->st_size;
    args_map[SYSCALL_FIELD_STAT_RESULT_BLKSIZE] = &statbuf->st_blksize;
    args_map[SYSCALL_FIELD_STAT_RESULT_BLOCKS] = &statbuf->st_blocks;

    /*
     * Convert stat_result_atime, stat_result_mtime and
     * stat_result_ctime to Tfracs.
     */
    static uint64_t atime_Tfrac = timespec_to_Tfrac(statbuf->st_atim);
    static uint64_t mtime_Tfrac = timespec_to_Tfrac(statbuf->st_mtim);
    static uint64_t ctime_Tfrac = timespec_to_Tfrac(statbuf->st_ctim);
    args_map[SYSCALL_FIELD_STAT_RESULT_ATIME] = &atime_Tfrac;
    args_map[SYSCALL_FIELD_STAT_RESULT_MTIME] = &mtime_Tfrac;
    args_map[SYSCALL_FIELD_STAT_RESULT_CTIME] = &ctime_Tfrac;
  } else {
    std::cerr << "LStat: Struct stat buffer is set as NULL!!" << std::endl;
  }
}
