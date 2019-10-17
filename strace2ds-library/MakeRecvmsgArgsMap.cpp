/*
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2018 Erez Zadok
 * Copyright (c) 2015-2018 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements the recvmsg function in the DataSeriesOutputModule.hpp
 * header file.
 *
 * Read the DataSeriesOutputModule.hpp file for more information about this
 * class.
 */

#include "DataSeriesOutputModule.hpp"

void DataSeriesOutputModule::makeRecvmsgArgsMap(void **args_map, long *args,
                                                void **v_args) {
  int iov_number = -1;
  if (v_args[0] != NULL) {
    iov_number = *(int *)v_args[0];
  }

  /*
   * iov_number equals to '-1' denotes the first record for the
   * recvmsg system call. For first record, we save the file
   * descriptor, iov_number and total number of bytes
   * requested and do not set the iov_data_read bytes.
   */
  if (iov_number == -1) {
    initArgsMap(args_map, "recvmsg");
    args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
    args_map[SYSCALL_FIELD_FLAGS_VALUE] = &args[2];
    // Set individual recvmsg flag bits
    u_int flag = processRecvFlags(args_map, args[2]);
    if (flag != 0) {
      std::cerr << "Recvmsg: These flags are not processed/unknown->0x";
      std::cerr << std::hex << flag << std::dec << std::endl;
    }
    if (v_args[0] != NULL && v_args[1] != NULL) {
      args_map[SYSCALL_FIELD_IOV_NUMBER] = v_args[0];
      args_map[SYSCALL_FIELD_BYTES_REQUESTED] = v_args[1];
    } else {
      std::cerr << "Recvmsg: struct msghdr buffer is set as NULL!!"
                << std::endl;
    }
  } else {
    /*
     * For rest of the records, we do not save file descriptor and
     * flags fields. We only save the iov_number, bytes_requested
     * and iov_data_read fields.
     */
    args_map[SYSCALL_FIELD_IOV_NUMBER] = v_args[0];
    args_map[SYSCALL_FIELD_BYTES_REQUESTED] = v_args[1];
    args_map[SYSCALL_FIELD_IOV_DATA_READ] = &v_args[2];
  }
}
