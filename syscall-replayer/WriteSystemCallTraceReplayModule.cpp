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
 * WriteSystemCallTraceReplayModule header file
 *
 * Read WriteSystemCallTraceReplayModule.hpp for more information about this class. 
 */

#include "WriteSystemCallTraceReplayModule.hpp"

WriteSystemCallTraceReplayModule::WriteSystemCallTraceReplayModule(DataSeriesModule &source,
								   bool verbose_flag,
								   bool verify_flag,
								   int warn_level_flag,
								   std::string pattern_data):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  verify_(verify_flag),
  pattern_data_(pattern_data),
  descriptor_(series, "descriptor"),
  data_written_(series, "data_written", Field::flag_nullable),
  bytes_requested_(series, "bytes_requested") {
  sys_call_ = "write";
}

void WriteSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Write System Call Replayer starts to replay...-----" << std::endl;
  if (pattern_data_ == "random") {
    // Pattern is random, so open urandom device
    random_file_.open("/dev/urandom");
    if (!random_file_.is_open()) {
      std::cerr << "Unable to open file '/dev/urandom/'.\n";
      exit(EXIT_FAILURE);
    }
  }
}

void WriteSystemCallTraceReplayModule::processRow() {
  size_t nbytes = (size_t)bytes_requested_.val();
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  char *buffer;

  // Check to see if write data was in DS and user didn't specify pattern                                           
  if (data_written_.isNull() && pattern_data_.empty()) {
    // Let's write zeros.                                                                                           
    pattern_data_ = "0x0";
  }

  // Check to see if user wants to use pattern
  if (!pattern_data_.empty()) {
    buffer = new char[nbytes];
    if (pattern_data_ == "random") {
      random_file_.read(buffer, nbytes);
    } else {
      int pattern_hex;
      std::stringstream pattern_stream;
      pattern_stream << std::hex << pattern_data_;
      pattern_stream >> pattern_hex;
      memset(buffer, pattern_hex, nbytes);
    }
  } else {
    buffer = (char *)data_written_.val();
  }

  if (verbose_) {
    std::cout << sys_call_ << ": ";
    std::cout.precision(25);
    std::cout << "time called(" << std::fixed << time_called() << "),";
    std::cout << "descriptor(" << descriptor_.val() << "),";
    std::cout << "data(" << buffer << "),";
    std::cout << "nbytes(" << nbytes << ")" << std::endl;
  }

  int ret = write(fd, buffer, nbytes);
  compare_retval(ret);
  
  // Free buffer
  if (!pattern_data_.empty()){
    delete[] buffer;
  }

  if (ret == -1) {
    perror("write");
  } else {
    if (verbose_) {
      std::cout << "write is successfully replayed\n";
    }
  }
}

void WriteSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Write System Call Replayer finished replaying...-----" << std::endl;
  if (pattern_data_ == "random") {
    // Close the urandom device
    random_file_.close();
  }
}
