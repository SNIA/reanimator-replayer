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
 * This file implements all the functions in the OpenSystemCallTraceReplayModule 
 * header file 
 *
 * Read OpenSystemCallTraceReplayModule.hpp for more information about this class.
 */

#include "OpenSystemCallTraceReplayModule.hpp"

OpenSystemCallTraceReplayModule::OpenSystemCallTraceReplayModule(DataSeriesModule &source,
								 bool verbose_flag, 
								 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname"),
  open_value_(series, "open_value", Field::flag_nullable),
  mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_ = "open";
}

void OpenSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Open System Call Replayer starts to replay...-----" << std::endl;
}

void OpenSystemCallTraceReplayModule::processRow() {
  const char *pathname = (char *)given_pathname_.val();
  int flags = open_value_.val();
  mode_t mode = mode_value_.val();
  int return_value = (int)return_value_.val();

  if (verbose_) {
    std::cout << "open: ";
    std::cout.precision(25);
    std::cout << "time called(" << std::fixed << time_called() << "), ";
    std::cout << "pathname(" << pathname << "), ";
    std::cout << "flags(" << flags << "), ";
    std::cout << "mode(" << mode << ")\n";
  }
  
  // replay the open system call
  int replay_ret = open(pathname, flags, mode);
  compare_retval(replay_ret);
  // Add a mapping from fd in trace file to actual replayed fd
  SystemCallTraceReplayModule::fd_map_[return_value] = replay_ret;

  if (replay_ret == -1) {
    perror(pathname);
  } else {
    if (verbose_) {
      std::cout << pathname << " is successfully opened..." << std::endl;
    }
  }
}

void OpenSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Open System Call Replayer finished replaying...-----" << std::endl;
}
