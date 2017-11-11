/*
 * Copyright (c) 2016 Nina Brown
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
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), ",
    "replayed fd(", replayed_fd, ")");
}

void FsyncSystemCallTraceReplayModule::processRow() {
  // Get actual file descriptor
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());

  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the fsync call originated from a socket().
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }

  replayed_ret_val_ = fsync(fd);
}
