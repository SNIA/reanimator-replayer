/*
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

void DataSeriesOutputModule::makeSocketpairArgsMap(void **args_map,
						   long *args,
						   void **v_args) {
  static int sv[2];

  args_map[SYSCALL_FIELD_DOMAIN] = &args[0];
  args_map[SYSCALL_FIELD_TYPE] = &args[1];
  args_map[SYSCALL_FIELD_PROTOCOL] = &args[2];

  if (v_args[0] != NULL) {
    sv[0] = ((int *) v_args[0])[0];
    sv[1] = ((int *) v_args[0])[1];
  } else {
    /*
     * In the case of a NULL socket array, set
     * read_descriptor and write_descriptor equal to 0.
     */
    sv[0] = 0;
    sv[1] = 0;
    std::cerr << "Socketpair: Socket array is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_READ_DESCRIPTOR] = &sv[0];
  args_map[SYSCALL_FIELD_WRITE_DESCRIPTOR] = &sv[1];
}
