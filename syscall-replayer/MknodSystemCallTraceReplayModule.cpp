/*
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * MknodSystemCallTraceReplayModule header file
 *
 * Read MknodSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "MknodSystemCallTraceReplayModule.hpp"

MknodSystemCallTraceReplayModule::
MknodSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname"),
  mode_value_(series, "mode_value", Field::flag_nullable),
  type_(series, "type"),
  dev_(series, "dev", Field::flag_nullable) {
  sys_call_name_ = "mknod";
}

void MknodSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "pathname(" << given_pathname_.val() << "), ";
  std::cout << "file type(";

  // Decode the file type field from the encoding specified by SNIA
  switch (type_.val()) {
  case 0:
    std::cout << "regular";
    break;
  case 1:
    std::cout << "character special";
    break;
  case 2:
    std::cout << "block special";
    break;
  case 3:
    std::cout << "FIFO";
    break;
  case 4:
    std::cout << "socket";
    break;
  }
  std::cout << "), ";
  std::cout << "mode(" << mode_value_.val() << "), ";
  std::cout << "dev(" << dev_.val() << ")";
}

void MknodSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Mknod System Call Replayer starts to replay...-----"
	    << std::endl;
}

void MknodSystemCallTraceReplayModule::processRow() {
  const char *pathname = (char *)given_pathname_.val();
  mode_t mode = mode_value_.val();
  dev_t dev = dev_.val();

  // replay the mknod system call
  replayed_ret_val_ = mknod(pathname, mode, dev);
}

void MknodSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Mknod System Call Replayer finished replaying...-----"
	    << std::endl;
}
