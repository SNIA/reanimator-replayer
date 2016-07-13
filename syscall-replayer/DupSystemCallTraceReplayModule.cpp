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
 * DupSystemCallTraceReplayModule header file.
 *
 * Read DupSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "DupSystemCallTraceReplayModule.hpp"

DupSystemCallTraceReplayModule::
DupSystemCallTraceReplayModule(DataSeriesModule &source,
			       bool verbose_flag,
			       int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor") {
  sys_call_name_ = "dup";
}

void DupSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "descriptor(" << descriptor_.val() << ")";
}

void DupSystemCallTraceReplayModule::processRow() {
  // Get actual file descriptor
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];

  replayed_ret_val_ = dup(fd);

  /*
   * Map replayed duplicated file descriptor to traced duplicated
   * file descriptor
   */
  SystemCallTraceReplayModule::fd_map_[return_value()] = replayed_ret_val_;
}