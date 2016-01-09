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
 * LSeekSystemCallTraceReplayModule header file
 *
 * Read LSeekSystemCallTraceReplayModule.hpp for more information about this class. 
 */

#include "LSeekSystemCallTraceReplayModule.hpp"

LSeekSystemCallTraceReplayModule::LSeekSystemCallTraceReplayModule(DataSeriesModule &source,
								   bool verbose_flag,
								   int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor"),
  offset_(series, "offset"),
  whence_(series, "whence") {
  sys_call_ = "lseek";
}

void LSeekSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----lseek system call replayer starts to replay...-----" << std::endl;
}

void LSeekSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  long offset = offset_.val();
  uint8_t whence = whence_.val();
  if (verbose_) {
    std::cout << sys_call_ << ": ";
    std::cout.precision(23);
    std::cout << "time called(" << std::fixed << time_called() << "), ";
    std::cout << "descriptor(" << fd << "), ";
    std::cout << "offset(" << offset << "), ";
    std::cout << "whence(" << (int)whence << ")." << std::endl;
  }
  // Replay
  int ret = lseek(fd, offset, whence);
  compare_retval(ret);

  if (ret == -1) {
    perror("lseek");
  } else {
    if (verbose_) {
      std::cout << "lseek was executed successfully!" << std::endl;
    }
  }
}

void LSeekSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----lseek system call replayer finished replaying...-----" << std::endl;
}
