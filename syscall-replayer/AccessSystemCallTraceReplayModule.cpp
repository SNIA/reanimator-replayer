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
 * AccessSystemCallTraceReplayModule header file
 *
 * Read AccessSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "AccessSystemCallTraceReplayModule.hpp"

AccessSystemCallTraceReplayModule::
AccessSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname"),
  mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "access";
}

void AccessSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", given_pathname_.val(), "), " \
    "traced mode(", mode_value_.val(), "), ",
    "replayed mode(", get_mode(mode_value_.val()), ")");
}

void AccessSystemCallTraceReplayModule::processRow() {
  const char *pathname = (char *)given_pathname_.val();
  int mode_value = get_mode(mode_value_.val());

  // Replay the access system call
  replayed_ret_val_ = access(pathname, mode_value);
}

FAccessatSystemCallTraceReplayModule::
FAccessatSystemCallTraceReplayModule(DataSeriesModule &source,
				     bool verbose_flag,
				     int warn_level_flag):
  AccessSystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor"),
  flags_value_(series, "flags_value", Field::flag_nullable) {
  sys_call_name_ = "faccessat";
}

void FAccessatSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), ",
    "replayed fd(", replayed_fd, ")",
    "pathname(", given_pathname_.val(), "), ",
    "traced mode(", mode_value_.val(), "), ",
    "replayed mode(", get_mode(mode_value_.val()), ")",
    "flag(", flags_value_.val(), ")");
}

void FAccessatSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  const char *pathname = (char *)given_pathname_.val();
  int mode = get_mode(mode_value_.val());
  int flags = flags_value_.val();

  if (replayed_fd == SYSCALL_SIMULATED) {
    /*
     * FD for the faccessat call originated from a socket().
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }

  // Replay the faccessat system call
  replayed_ret_val_ = faccessat(replayed_fd, pathname, mode, flags);
}
