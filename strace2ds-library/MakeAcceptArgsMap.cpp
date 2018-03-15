/*
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements one of the functions in the DataSeriesOutputModule.hpp
 * header file.
 *
 * Read the DataSeriesOutputModule.hpp file for more information about this
 * class.
 */

#include "DataSeriesOutputModule.hpp"

void DataSeriesOutputModule::makeAcceptArgsMap(void **args_map,
					       long *args,
					       void **v_args) {
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_SOCKADDR_BUFFER] = &v_args[0];

  if(v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_SOCKADDR_LENGTH] = v_args[1];
  } else {
    args_map[SYSCALL_FIELD_SOCKADDR_LENGTH] = &v_args[1];
  }
}
