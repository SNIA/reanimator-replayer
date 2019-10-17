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

void DataSeriesOutputModule::makeFChmodatArgsMap(void **args_map, long *args,
                                                 void **v_args) {
  int mode_offset = 2;
  initArgsMap(args_map, "fchmodat");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "FChmodat: Pathname is set as NULL!!" << std::endl;
  }

  // set individual  Mode values
  mode_t mode = processMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "FChmodat: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }

  // set flag values
  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[3];
  u_int flag = args[3];
  process_Flag_and_Mode_Args(args_map, flag, AT_SYMLINK_NOFOLLOW,
                             SYSCALL_FIELD_FLAG_AT_SYMLINK_NOFOLLOW);
  if (flag != 0) {
    std::cerr << "FChmodat: These flags are not processed/unknown->0";
    std::cerr << std::oct << flag << std::dec << std::endl;
  }
}
