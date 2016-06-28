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
 * ReadlinkSystemCallTraceReplayModule header file
 *
 * Read ReadlinkSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "ReadlinkSystemCallTraceReplayModule.hpp"

ReadlinkSystemCallTraceReplayModule::
ReadlinkSystemCallTraceReplayModule(DataSeriesModule &source,
				    bool verbose_flag,
				    bool verify_flag,
				    int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  verify_(verify_flag),
  given_pathname_(series, "given_pathname"),
  link_value_(series, "link_value"),
  buffer_size_(series, "buffer_size", Field::flag_nullable) {
  sys_call_name_ = "readlink";
}

void ReadlinkSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Readlink System Call Replayer starts to replay...-----"
	    << std::endl;
}

void ReadlinkSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "link path(" << given_pathname_.val() << "), ";
  std::cout << "target path(" << (char*)link_value_.val() << "), " ;
  std::cout << "buffer size(" << buffer_size_.val() << "), " << std::endl;
}

void ReadlinkSystemCallTraceReplayModule::processRow() {
  char *pathname = (char*)given_pathname_.val();
  char nbytes = (int)buffer_size_.val();
  int return_value = (int)return_value_.val();
  char target_path[nbytes];

  // replay the readlink system call
  replayed_ret_val_ = readlink(pathname, target_path, nbytes);

  if (verify_ == true) {
    // Verify readlink buffer and buffer in the trace file are same
    if (memcmp(link_value_.val(), target_path, return_value) != 0){
      // Target path aren't same
      std::cerr << "Verification of path in readlink failed.\n";
      if (!default_mode()) {
	std::cout << "time called:" << std::fixed << time_called()
		  << std::endl;
	std::cout << "Captured readlink path is different from replayed\
		      readlink path" << std::endl;
	std::cout << "Captured readlink path: "
		  << (char*)link_value_.val() << ", ";
	std::cout << "Replayed readlink path: "
		  << (char*)target_path << std::endl;
	if (abort_mode()) {
	  abort();
	}
      }
    } else {
      if (verbose_mode()) {
	std::cout << "Verification of path in readlink success.\n";
      }
    }
  }
}

void ReadlinkSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Readlink System Call Replayer finished replaying...-----"
	    << std::endl;
}
