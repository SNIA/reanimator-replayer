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
 * FsyncSystemCallTraceReplayModule header file.
 *
 * Read FsyncSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "FsyncSystemCallTraceReplayModule.hpp"

FsyncSystemCallTraceReplayModule::
FsyncSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor") {
  sys_call_name_ = "fsync";
}

void FsyncSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "descriptor(" << descriptor_.val() << ")";
}

void FsyncSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Fsync System Call Replayer starts to replay...-----"
	    << std::endl;
}

void FsyncSystemCallTraceReplayModule::processRow() {
  // Get actual file descriptor
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];

  replayed_ret_val_ = fsync(fd);
}

void FsyncSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Fsync System Call Replayer finished replaying...-----"
	    << std::endl;
}