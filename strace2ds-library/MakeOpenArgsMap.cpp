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

void DataSeriesOutputModule::makeOpenArgsMap(void **args_map, long *args,
                                             void **v_args) {
  int offset = 0;

  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "open");

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Open: Pathname is set as NULL!!" << std::endl;
  }

  /* Setting flag values */
  args_map[SYSCALL_FIELD_OPEN_VALUE] = &args[offset + 1];
  u_int flag = processOpenFlags(args_map, args[offset + 1]);
  if (flag != 0) {
    std::cerr << "Open: These flag are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }

  /*
   * If only, open is called with 3 arguments, set the corresponding
   * mode value and mode bits as True.
   */
  if (args[offset + 1] & O_CREAT) {
    mode_t mode = processMode(args_map, args, offset + 2);
    if (mode != 0) {
      std::cerr << "Open: These modes are not processed/unknown->0";
      std::cerr << std::oct << mode << std::dec << std::endl;
    }
  }
}
