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
 * This file implements all the functions in the CloseSystemCallTraceReplayModule
 * header file.
 *
 * Read OpenSystemCallTraceReplayModule.hpp for more information about this class.
 */

#include "CloseSystemCallTraceReplayModule.hpp"

CloseSystemCallTraceReplayModule::CloseSystemCallTraceReplayModule(DataSeriesModule &source,
								   bool verbose_flag, 
								   int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor") {
  sys_call_ = "close";
}

void CloseSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Close System Call Replayer starts to replay...-----" << std::endl;
}

void CloseSystemCallTraceReplayModule::processRow() {
  // Get actual file descriptor
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  if (verbose_) {
    std::cout << "close: ";
    std::cout.precision(25);
    std::cout << "time called(" << std::fixed << time_called() << "), ";
    std::cout << "descriptor(" << descriptor_.val() << ")\n";
  }
  
  int ret = close(fd);
  SystemCallTraceReplayModule::fd_map_.erase(descriptor_.val());
  compare_retval(ret);
  
  if (ret == -1) {
    perror("close");
  } else {
    if (verbose_) {
      std::cout << "fd " << descriptor_.val() << " is successfully closed..." << std::endl;
    }
  }
}

void CloseSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Close System Call Replayer finished replaying...-----" << std::endl;
}
