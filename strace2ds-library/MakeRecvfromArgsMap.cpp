/*
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2018 Erez Zadok
 * Copyright (c) 2015-2018 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements the recvfrom function in the DataSeriesOutputModule.hpp 
 * header file.
 *
 * Read the DataSeriesOutputModule.hpp file for more information about this
 * class.
 */

#include "DataSeriesOutputModule.hpp"

void DataSeriesOutputModule::makeRecvfromArgsMap(void **args_map,
						 long *args,
						 void **v_args) {
  initArgsMap(args_map, "recvfrom");
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_DATA_READ] = &v_args[0];
  args_map[SYSCALL_FIELD_BYTES_REQUESTED] = &args[2];
  args_map[SYSCALL_FIELD_FLAGS_VALUE] = &args[3];
  // Set individual recvfrom flag bits
  u_int flag = processRecvFlags(args_map,args[3]);
  if (flag != 0) {
    std::cerr << "Recvfrom: These flags are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }
  args_map[SYSCALL_FIELD_SOCKADDR_LENGTH] = &v_args[1];
}
