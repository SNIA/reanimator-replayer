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

void DataSeriesOutputModule::makeMknodatArgsMap(void **args_map, long *args,
                                                void **v_args) {
  static int32_t dev;
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  initArgsMap(args_map, "mknodat");
  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "Mknodat: Pathname is set as NULL!!" << std::endl;
  }

  mode_t mode = processMode(args_map, args, 2);

  /* Can reuse processMknodType since the mode values are same */
  mode = processMknodType(args_map, mode);

  if (mode != 0) {
    std::cerr << "Mknodat: These modes are not processed/unknown: ";
    std::cerr << std::oct << mode << std::dec << ". " << std::endl;
  }

  if ((args[2] & S_IFCHR) || (args[2] & S_IFBLK)) {
    dev = (int32_t)args[3];
    args_map[SYSCALL_FIELD_DEV] = &dev;
  }
}

mode_t DataSeriesOutputModule::processMknodType(void **args_map, mode_t mode) {
  static u_int type;

  /*
   * Check for each file type.  If the mode is equal to the value for that
   * file type, set type to the encoding value specified by SNIA:
   * Regular = 0
   * Character special = 1
   * Block special = 2
   * FIFO = 3
   * Socket = 4
   */
  if ((S_ISREG(mode)) || (mode == 0)) {
    type = DS_FILE_TYPE_REG;
    mode &= ~S_IFREG;
  } else if (S_ISCHR(mode)) {
    type = DS_FILE_TYPE_CHR;
    mode &= ~S_IFCHR;
  } else if (S_ISBLK(mode)) {
    type = DS_FILE_TYPE_BLK;
    mode &= ~S_IFBLK;
  } else if (S_ISFIFO(mode)) {
    type = DS_FILE_TYPE_FIFO;
    mode &= ~S_IFIFO;
  } else if (S_ISSOCK(mode)) {
    type = DS_FILE_TYPE_SOCK;
    mode &= ~S_IFSOCK;
  }
  args_map[SYSCALL_FIELD_TYPE] = &type;

  /*
   * Return remaining unprocessed modes so that caller can warn
   * of unknown modes if the mode value is not set as zero.
   */
  return mode;
}
