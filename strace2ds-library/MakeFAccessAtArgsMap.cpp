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

void DataSeriesOutputModule::makeFAccessatArgsMap(void **args_map, long *args,
                                                  void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "faccessat");
  u_int mode_offset = 2;

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  } else {
    std::cerr << "FAccessat: Pathname is set as NULL!!" << std::endl;
  }

  // Map the individual mode fields
  mode_t mode = processAccessMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "FAccessat: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }

  args_map[SYSCALL_FIELD_FLAGS_VALUE] = &args[3];
  // Map the inividual flag values
  u_int flag = processFAccessatFlags(args_map, args[3]);
  if (flag != 0) {
    std::cerr << "FAccessat: These flags are not processed/unknown->"
              << std::hex << flag << std::dec << std::endl;
  }
}

u_int DataSeriesOutputModule::processFAccessatFlags(void **args_map,
                                                    u_int faccessat_flags) {
  /*
   * Process each individual faccessat flag bit that has been set
   * in the argument faccessat_flags.
   */
  // set eaccess flag
  process_Flag_and_Mode_Args(args_map, faccessat_flags, AT_EACCESS,
                             SYSCALL_FIELD_FLAGS_AT_EACCESS);
  // set symlink nofollow flag
  process_Flag_and_Mode_Args(args_map, faccessat_flags, AT_SYMLINK_NOFOLLOW,
                             SYSCALL_FIELD_FLAGS_AT_SYMLINK_NOFOLLOW);

  /*
   * Return remaining faccessat flags so that caller can
   * warn of unknown flags if the faccessat_flags is not set
   * as zero.
   */
  return faccessat_flags;
}

/*
 * This function unwraps the mode value passed as an argument to the
 * Access system call, which has different modes than Open and Mkdir.
 *
 * @param args_map: stores mapping of <field, value> pairs.
 *
 * @param args: represents complete arguments of the actual system call.
 *
 * @param mode_offset: represents index of mode value in the actual
 *		       system call.
 */
mode_t DataSeriesOutputModule::processAccessMode(void **args_map, long *args,
                                                 u_int mode_offset) {
  // Save the mode argument with mode_value field in the map
  args_map[SYSCALL_FIELD_MODE_VALUE] = &args[mode_offset];
  mode_t mode = args[mode_offset];

  // set read permission bit
  process_Flag_and_Mode_Args(args_map, mode, R_OK, SYSCALL_FIELD_MODE_READ);
  // set write permission bit
  process_Flag_and_Mode_Args(args_map, mode, W_OK, SYSCALL_FIELD_MODE_WRITE);
  // set execute permission bit
  process_Flag_and_Mode_Args(args_map, mode, X_OK, SYSCALL_FIELD_MODE_EXECUTE);
  // set existence bit
  process_Flag_and_Mode_Args(args_map, mode, F_OK, SYSCALL_FIELD_MODE_EXIST);

  /*
   * Return remaining unprocessed modes so that caller can warn
   * of unknown modes if the mode value is not set as zero.
   */
  return mode;
}
