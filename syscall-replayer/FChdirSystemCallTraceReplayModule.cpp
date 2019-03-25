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
 * FChdirSystemCallTraceReplayModule header file.
 *
 * Read FChdirSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "FChdirSystemCallTraceReplayModule.hpp"

FChdirSystemCallTraceReplayModule::FChdirSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      descriptor_(series, "descriptor") {
  sys_call_name_ = "fchdir";
}

void FChdirSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, file_descriptor);

  syscall_logger_->log_info("traced fd(", file_descriptor, "), ",
                            "replayed fd(", replayed_fd, ")");
}

void FChdirSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, file_descriptor);

  if (fd == SYSCALL_SIMULATED) {
    replayed_ret_val_ = return_value();
  } else {
    replayed_ret_val_ = fchdir(fd);
  }
}

void FChdirSystemCallTraceReplayModule::prepareRow() {
  file_descriptor = descriptor_.val();
  SystemCallTraceReplayModule::prepareRow();
}
