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
 * ReadSystemCallTraceReplayModule header file
 *
 * Read ReadSystemCallTraceReplayModule.hpp for more information about this class. 
 */

#include "ReadSystemCallTraceReplayModule.hpp"

ReadSystemCallTraceReplayModule::ReadSystemCallTraceReplayModule(DataSeriesModule &source, 
								 bool verbose_flag, 
								 bool verify_flag, 
								 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  verify_(verify_flag),
  descriptor_(series, "descriptor"),
  data_read_(series, "data_read", Field::flag_nullable),
  bytes_requested_(series, "bytes_requested") {
  sys_call_ = "read";
}

void ReadSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Read System Call Replayer starts to replay...-----" << std::endl;
}

void ReadSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  int nbytes = bytes_requested_.val();
  if (verbose_) {
    std::cout << sys_call_ << ": ";
    std::cout.precision(23);
    std::cout << "time called(" << std::fixed << time_called() << "), ";
    std::cout << "descriptor:" << fd << "), ";
    std::cout << "data read(" << data_read_.val() << "), ";
    std::cout << "size(" << nbytes << ")\n";
  }
  char buffer[nbytes];
  int ret = read(fd, buffer, nbytes);
  compare_retval(ret);

  if (verify_ == true) {
    // Verify read data and data in the trace file are same
    if (memcmp(data_read_.val(), buffer, ret) != 0){
      // Data aren't same
      std::cerr << "Verification of data in read failed.\n";
      if (warn_level_ != DEFAULT_MODE) {
	std::cout << "time called:" << std::fixed << time_called() << std::endl;
	std::cout << "Captured read data is different from replayed read data" << std::endl;
	std::cout << "Captured read data: " << data_read_.val() << ", ";
	std::cout << "Replayed read data: " << buffer << std::endl;
	if (warn_level_ == ABORT_MODE ) {
	  abort();
	}
      }
    } else {
      if (verbose_) {
	std::cout << "Verification of data in read success.\n";
      }
    }
  }

  if (ret == -1) {
    perror("read");
  } else {
    if (verbose_) {
      std::cout << "read is executed successfully!" << std::endl;
    }
  }
}

void ReadSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Read System Call Replayer finished replaying...-----" << std::endl;
}
