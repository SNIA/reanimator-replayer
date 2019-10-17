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

uint64_t DataSeriesOutputModule::timespec_to_Tfrac(struct timespec ts) {
  double time_seconds = (double)ts.tv_sec + pow(10.0, -9) * ts.tv_nsec;
  uint64_t time_Tfracs = (uint64_t)(time_seconds * (((uint64_t)1) << 32));
  return time_Tfracs;
}

void DataSeriesOutputModule::makeCreatArgsMap(void **args_map, long *args,
                                              void **v_args) {
  initArgsMap(args_map, "creat");
  int mode_offset = 1;
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Creat: Pathname is set as NULL!!" << std::endl;
  }
  mode_t mode = processMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "Creat: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}
