/*
 * Copyright (c) 2016 Nina Brown
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
 * FChmodSystemCallTraceReplayModule header file
 *
 * Read FChmodSystemCallTraceReplayModule.hpp for more information about this
 * class.
 */

#include "FChmodSystemCallTraceReplayModule.hpp"

FChmodSystemCallTraceReplayModule::FChmodSystemCallTraceReplayModule(
						   DataSeriesModule &source,
						   bool verbose_flag,
						   int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor"),
  mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "fchmod";
}

void FChmodSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "descriptor(" << descriptor_.val() << "), ";
  std::cout << "mode(" << mode_value_.val() << ")";
}

void FChmodSystemCallTraceReplayModule::processRow() {
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  mode_t mode = mode_value_.val();

  // Replay the fchmod system call
  replayed_ret_val_ = fchmod(fd, mode);
}
