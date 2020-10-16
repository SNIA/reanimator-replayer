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

void DataSeriesOutputModule::makeLinkatArgsMap(void **args_map, long *args,
                                               void **v_args) {
  initArgsMap(args_map, "linkat");

  args_map[SYSCALL_FIELD_OLD_DESCRIPTOR] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_OLD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
  }

  args_map[SYSCALL_FIELD_NEW_DESCRIPTOR] = &args[2];
  if (args[2] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_NEW_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
  }

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_OLDPATHNAME] = &v_args[0];
  } else {
    std::cerr << "Linkat: Old Pathname is set as NULL!!" << std::endl;
  }
  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_NEWPATHNAME] = &v_args[1];
  } else {
    std::cerr << "Linkat: New Pathname is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[4];
  u_int flag = args[4];
  process_Flag_and_Mode_Args(args_map, flag, AT_EMPTY_PATH,
                             SYSCALL_FIELD_FLAG_EMPTY_PATH);
  process_Flag_and_Mode_Args(args_map, flag, AT_SYMLINK_FOLLOW,
                             SYSCALL_FIELD_FLAG_SYMLINK_FOLLOW);
  if (flag != 0) {
    std::cerr << "Linkat: These flags are not processed/unknown->" << std::hex
              << flag << std::dec << std::endl;
  }
}
