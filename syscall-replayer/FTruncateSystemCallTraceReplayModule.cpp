/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * FTruncateSystemCallTraceReplayModule header file
 *
 * Read FTruncateSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "FTruncateSystemCallTraceReplayModule.hpp"

FTruncateSystemCallTraceReplayModule::FTruncateSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      descriptor_(series, "descriptor"),
      truncate_length_(series, "truncate_length") {
  sys_call_name_ = "ftruncate";
}

void FTruncateSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), ",
                            "replayed fd(", getReplayedFD(), "), ", "length(",
                            truncate_length_.val(), ")");
}

int FTruncateSystemCallTraceReplayModule::getReplayedFD() {
  // Get replaying file descriptor.
  pid_t pid = executing_pid();
  return replayer_resources_manager_.get_fd(pid, descriptor_.val());
}

void FTruncateSystemCallTraceReplayModule::processRow() {
  int fd = getReplayedFD();
  int64_t length = truncate_length_.val();
  // Replay ftruncate system call
  replayed_ret_val_ = ftruncate(fd, length);
}
