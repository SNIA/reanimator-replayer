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

void DataSeriesOutputModule::makeOpenatArgsMap(void **args_map, long *args,
                                               void **v_args) {
  int offset = 1;

  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "openat");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
  }

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Openat: Pathname is set as NULL!!" << std::endl;
  }

  /* Setting flag values */
  args_map[SYSCALL_FIELD_OPEN_VALUE] = &args[offset + 1];
  u_int flag = processOpenFlags(args_map, args[offset + 1]);
  if (flag != 0) {
    std::cerr << "Openat: These flags are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }

  /*
   * If openat is called with 4 arguments, set the corresponding
   * mode value and mode bits as True.
   */
  if (args[offset + 1] & O_CREAT) {
    mode_t mode = processMode(args_map, args, offset + 2);
    if (mode != 0) {
      std::cerr << "Openat: These modes are not processed/unknown->0";
      std::cerr << std::oct << mode << std::dec << std::endl;
    }
  }
}

/*
 * This function unwraps the flag value passed as an argument to the
 * open system call and sets the corresponding flag values as True.
 *
 * @param args_map: stores mapping of <field, value> pairs.
 *
 * @param open_flag: represents the flag value passed as an argument
 *                   to the open system call.
 */
u_int DataSeriesOutputModule::processOpenFlags(void **args_map,
                                               u_int open_flag) {
  /*
   * Process each individual flag bits that has been set
   * in the argument open_flag.
   */
  // set read only flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_RDONLY,
                             SYSCALL_FIELD_FLAG_READ_ONLY);
  // set write only flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_WRONLY,
                             SYSCALL_FIELD_FLAG_WRITE_ONLY);
  // set both read and write flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_RDWR,
                             SYSCALL_FIELD_FLAG_READ_AND_WRITE);
  // set append flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_APPEND,
                             SYSCALL_FIELD_FLAG_APPEND);
  // set async flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_ASYNC,
                             SYSCALL_FIELD_FLAG_ASYNC);
  // set close-on-exec flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_CLOEXEC,
                             SYSCALL_FIELD_FLAG_CLOSE_ON_EXEC);
  // set create flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_CREAT,
                             SYSCALL_FIELD_FLAG_CREATE);
  // set direct flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_DIRECT,
                             SYSCALL_FIELD_FLAG_DIRECT);
  // set directory flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_DIRECTORY,
                             SYSCALL_FIELD_FLAG_DIRECTORY);
  // set exclusive flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_EXCL,
                             SYSCALL_FIELD_FLAG_EXCLUSIVE);
  // set largefile flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_LARGEFILE,
                             SYSCALL_FIELD_FLAG_LARGEFILE);
  // set last access time flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NOATIME,
                             SYSCALL_FIELD_FLAG_NO_ACCESS_TIME);
  // set controlling terminal flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NOCTTY,
                             SYSCALL_FIELD_FLAG_NO_CONTROLLING_TERMINAL);
  // set no_follow flag (in case of symbolic link)
  process_Flag_and_Mode_Args(args_map, open_flag, O_NOFOLLOW,
                             SYSCALL_FIELD_FLAG_NO_FOLLOW);
  // set non blocking mode flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NONBLOCK,
                             SYSCALL_FIELD_FLAG_NO_BLOCKING_MODE);
  // set no delay flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_NDELAY,
                             SYSCALL_FIELD_FLAG_NO_DELAY);
  // set synchronized IO flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_SYNC,
                             SYSCALL_FIELD_FLAG_SYNCHRONOUS);
  // set truncate mode flag
  process_Flag_and_Mode_Args(args_map, open_flag, O_TRUNC,
                             SYSCALL_FIELD_FLAG_TRUNCATE);

  /*
   * Return remaining unprocessed flags so that caller can
   * warn of unknown flags if the open_flag value is not set
   * as zero.
   */
  return open_flag;
}

/*
 * This function unwraps the mode value passed as an argument to the
 * system call.
 *
 * @param args_map: stores mapping of <field, value> pairs.
 *
 * @param args: represents complete arguments of the actual system call.
 *
 * @param mode_offset: represents index of mode value in the actual
 *		       system call.
 */
mode_t DataSeriesOutputModule::processMode(void **args_map, long *args,
                                           u_int mode_offset) {
  // Save the mode argument with mode_value file in map
  args_map[SYSCALL_FIELD_MODE_VALUE] = &args[mode_offset];
  mode_t mode = args[mode_offset];

  // set user-ID bit
  process_Flag_and_Mode_Args(args_map, mode, S_ISUID, SYSCALL_FIELD_MODE_UID);
  // set group-ID bit
  process_Flag_and_Mode_Args(args_map, mode, S_ISGID, SYSCALL_FIELD_MODE_GID);
  // set sticky bit
  process_Flag_and_Mode_Args(args_map, mode, S_ISVTX,
                             SYSCALL_FIELD_MODE_STICKY_BIT);
  // set user read permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IRUSR,
                             SYSCALL_FIELD_MODE_R_USER);
  // set user write permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IWUSR,
                             SYSCALL_FIELD_MODE_W_USER);
  // set user execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IXUSR,
                             SYSCALL_FIELD_MODE_X_USER);
  // set group read permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IRGRP,
                             SYSCALL_FIELD_MODE_R_GROUP);
  // set group write permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IWGRP,
                             SYSCALL_FIELD_MODE_W_GROUP);
  // set group execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IXGRP,
                             SYSCALL_FIELD_MODE_X_GROUP);
  // set others read permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IROTH,
                             SYSCALL_FIELD_MODE_R_OTHERS);
  // set others write permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IWOTH,
                             SYSCALL_FIELD_MODE_W_OTHERS);
  // set others execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, S_IXOTH,
                             SYSCALL_FIELD_MODE_X_OTHERS);

  /*
   * Return remaining unprocessed modes so that caller can warn
   * of unknown modes if the mode value is not set as zero.
   */
  return mode;
}
