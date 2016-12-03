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
 * Dup2SystemCallTraceReplayModule header file.
 *
 * Read Dup2SystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "Dup2SystemCallTraceReplayModule.hpp"

Dup2SystemCallTraceReplayModule::
Dup2SystemCallTraceReplayModule(DataSeriesModule &source,
				bool verbose_flag,
				int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  old_descriptor_(series, "old_descriptor"),
  new_descriptor_(series, "new_descriptor") {
  sys_call_name_ = "dup2";
}

void Dup2SystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("old descriptor(", old_descriptor_.val(), "), ", \
    "new descriptor(", new_descriptor_.val(), ")");
}

void Dup2SystemCallTraceReplayModule::processRow() {
  // Get actual file descriptor
  int old_fd = SystemCallTraceReplayModule::fd_map_[old_descriptor_.val()];
  int new_fd = new_descriptor_.val();

  replayed_ret_val_ = dup2(old_fd, new_fd);

  /*
   * Map replayed duplicated file descriptor to traced duplicated
   * file descriptor
   */
  SystemCallTraceReplayModule::fd_map_[return_value()] = replayed_ret_val_;
}
