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

void DataSeriesOutputModule::makeSendArgsMap(void **args_map, long *args,
                                             void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "send");
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_DATA_WRITTEN] = &v_args[0];
  args_map[SYSCALL_FIELD_BYTES_REQUESTED] = &args[2];
  args_map[SYSCALL_FIELD_FLAGS_VALUE] = &args[3];
  // Set individual send flag bits
  u_int flag = processSendFlags(args_map, args[3]);
  if (flag != 0) {
    std::cerr << "Send: These flag are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }
}

u_int DataSeriesOutputModule::processSendFlags(void **args_map,
                                               u_int send_flags) {
  /*
   * Process each individual send flag bit that has been set
   * in the argument send_flags.
   */
  // set send confirm flag
  process_Flag_and_Mode_Args(args_map, send_flags, MSG_CONFIRM,
                             SYSCALL_FIELD_FLAG_CONFIRM);
  // set send dontroute flag
  process_Flag_and_Mode_Args(args_map, send_flags, MSG_DONTROUTE,
                             SYSCALL_FIELD_FLAG_DONTROUTE);
  // set send dontwait flag
  process_Flag_and_Mode_Args(args_map, send_flags, MSG_DONTWAIT,
                             SYSCALL_FIELD_FLAG_DONTWAIT);
  // set send eor flag
  process_Flag_and_Mode_Args(args_map, send_flags, MSG_EOR,
                             SYSCALL_FIELD_FLAG_EOR);
  // set send more flag
  process_Flag_and_Mode_Args(args_map, send_flags, MSG_MORE,
                             SYSCALL_FIELD_FLAG_MORE);
  // set send nosignal flag
  process_Flag_and_Mode_Args(args_map, send_flags, MSG_NOSIGNAL,
                             SYSCALL_FIELD_FLAG_NOSIGNAL);
  // set send oob flag
  process_Flag_and_Mode_Args(args_map, send_flags, MSG_OOB,
                             SYSCALL_FIELD_FLAG_OOB);
  /*
   * Return remaining send flags so that caller can
   * warn of unknown flags if the send_flags is not set
   * as zero.
   */
  return send_flags;
}
