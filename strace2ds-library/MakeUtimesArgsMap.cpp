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

void DataSeriesOutputModule::makeUtimesArgsMap(void **args_map, long *args,
                                               void **v_args) {
  static uint64_t access_time_Tfrac;
  static uint64_t mod_time_Tfrac;

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Utimes: Pathname is set as NULL!!" << std::endl;
  }

  // If the timeval array is not NULL, set the corresponding values in the map
  if (v_args[1] != NULL) {
    struct timeval *tv = (struct timeval *)v_args[1];

    // Convert timeval arguments to Tfracs (uint64_t)
    access_time_Tfrac = timeval_to_Tfrac(tv[0]);
    mod_time_Tfrac = timeval_to_Tfrac(tv[1]);

    args_map[SYSCALL_FIELD_ACCESS_TIME] = &access_time_Tfrac;
    args_map[SYSCALL_FIELD_MOD_TIME] = &mod_time_Tfrac;
  }
}
