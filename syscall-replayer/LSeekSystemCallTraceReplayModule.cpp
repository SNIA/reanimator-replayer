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
 * Read LSeekSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "LSeekSystemCallTraceReplayModule.hpp"

LSeekSystemCallTraceReplayModule::
LSeekSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor"),
  offset_(series, "offset"),
  whence_(series, "whence") {
  sys_call_name_ = "lseek";
}

void LSeekSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("descriptor(", descriptor_.val(), "), ", \
    "offset(", offset_.val(), "), ", \
    "whence(", (unsigned) whence_.val(), ")");
}

void LSeekSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  long offset = offset_.val();
  uint8_t whence = whence_.val();

  // Replay
  replayed_ret_val_ = lseek(fd, offset, whence);
}
