/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * CloseSystemCallTraceReplayModule header file.
 *
 * Read CloseSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "CloseSystemCallTraceReplayModule.hpp"

CloseSystemCallTraceReplayModule::
CloseSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor") {
  sys_call_name_ = "close";
}

void CloseSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), ",
    "replayed fd(", replayed_fd, ")");
}

void CloseSystemCallTraceReplayModule::processRow() {
  // Get actual file descriptor
  int fd = replayer_resources_manager_.remove_fd(pid, descVal);

  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the close call originated from a socket().
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    return;
  }

  replayed_ret_val_ = close(fd);
}

void CloseSystemCallTraceReplayModule::prepareRow() {
  pid = executing_pid();
  replayed_ret_val_ = return_value_.val();
  descVal = descriptor_.val();
}
