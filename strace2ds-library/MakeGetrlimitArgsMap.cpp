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

void DataSeriesOutputModule::makeGetrlimitArgsMap(void **args_map, long *args,
                                                  void **v_args) {
  args_map[SYSCALL_FIELD_RESOURCE_VALUE] = &args[0];
  /*
   * TODO: The correct value of args_map["resource"] should be 0 if resource is
   * RLIMIT_AS, 1 if it is RLIMIT_CORE, 2 if it is RLIMIT_CPU, so and so forth.
   * Currently, we don't do this. We simply assume that resource is same
   * across different platforms.
   */
  args_map[SYSCALL_FIELD_RESOURCE] = &args[0];
  if (v_args[0] != NULL) {
    struct rlimit *rlim = (struct rlimit *)v_args[0];
    args_map[SYSCALL_FIELD_RESOURCE_SOFT_LIMIT] = &rlim->rlim_cur;
    args_map[SYSCALL_FIELD_RESOURCE_HARD_LIMIT] = &rlim->rlim_max;
  } else {
    std::cerr << "Getrlimit: Struct rlimit is set as NULL!!" << std::endl;
  }
}
