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

void DataSeriesOutputModule::makeUnlinkatArgsMap(void **args_map, long *args,
                                                 void **v_args) {
  initArgsMap(args_map, "unlinkat");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
  }
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Unlinkat: Pathname is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[2];
  u_int flag = args[2];
  process_Flag_and_Mode_Args(args_map, flag, AT_REMOVEDIR,
                             SYSCALL_FIELD_FLAG_REMOVE_DIRECTORY);
  if (flag != 0) {
    std::cerr << "Unlinkat: These flags are not processed/unknown->" << std::hex
              << flag << std::dec << std::endl;
  }
}
