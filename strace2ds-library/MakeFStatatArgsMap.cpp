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

void DataSeriesOutputModule::makeFStatatArgsMap(void **args_map, long *args,
                                                void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "fstatat");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "FStatat: Pathname is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    struct stat *statbuf = (struct stat *)v_args[1];

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
    std::cerr << "FStatat: Struct stat buffer is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_FLAGS_VALUE] = &args[3];

  u_int flag = processFStatatFlags(args_map, args[3]);
  if (flag != 0) {
    std::cerr << "FStatat: These flags are not processed/unknown->" << std::hex
              << flag << std::dec << std::endl;
  }
}

u_int DataSeriesOutputModule::processFStatatFlags(void **args_map,
                                                  u_int fstatat_flags) {
  /*
   * Process each individual statfs flag bit that has been set
   * in the argument fstatat_flags.
   */
  // set at empty path flag
  process_Flag_and_Mode_Args(args_map, fstatat_flags, AT_EMPTY_PATH,
                             SYSCALL_FIELD_FLAGS_AT_EMPTY_PATH);
  // set no auto mount flag
  process_Flag_and_Mode_Args(args_map, fstatat_flags, AT_NO_AUTOMOUNT,
                             SYSCALL_FIELD_FLAGS_AT_NO_AUTOMOUNT);
  // set symlink nofollow flag
  process_Flag_and_Mode_Args(args_map, fstatat_flags, AT_SYMLINK_NOFOLLOW,
                             SYSCALL_FIELD_FLAGS_AT_SYMLINK_NOFOLLOW);

  /*
   * Return remaining fstatat flags so that caller can
   * warn of unknown flags if the fstatat_flags is not set
   * as zero.
   */
  return fstatat_flags;
}
