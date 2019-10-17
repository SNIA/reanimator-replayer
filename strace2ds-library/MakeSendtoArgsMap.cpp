/*
 * Copyright (c) 2016-2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2018 Erez Zadok
 * Copyright (c) 2015-2018 Stony Brook University
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

void DataSeriesOutputModule::makeSendtoArgsMap(void **args_map, long *args,
                                               void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "sendto");
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_DATA_WRITTEN] = &v_args[0];
  args_map[SYSCALL_FIELD_BYTES_REQUESTED] = &args[2];
  args_map[SYSCALL_FIELD_FLAGS_VALUE] = &args[3];
  // Set individual send flag bits
  u_int flag = processSendFlags(args_map, args[3]);
  if (flag != 0) {
    std::cerr << "Sendto: These flag are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }
  args_map[SYSCALL_FIELD_SOCKADDR_BUFFER] = &v_args[1];
  args_map[SYSCALL_FIELD_SOCKADDR_LENGTH] = &args[5];
}
