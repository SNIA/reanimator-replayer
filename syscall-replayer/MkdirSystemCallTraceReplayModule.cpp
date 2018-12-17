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
 * MkdirSystemCallTraceReplayModule header file
 *
 * Read MkdirSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "MkdirSystemCallTraceReplayModule.hpp"

MkdirSystemCallTraceReplayModule::
MkdirSystemCallTraceReplayModule(DataSeriesModule &source,
  bool verbose_flag,
  int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname"),
  mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "mkdir";
}

void MkdirSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", pathname, "), ", \
    "traced mode(", modeVal, "), ",
    "replayed mode(", get_mode(modeVal), ")");
}

void MkdirSystemCallTraceReplayModule::processRow() {
  // Replay the mkdir system call
  replayed_ret_val_ = mkdir(pathname, get_mode(modeVal));
  delete[] pathname;
}

void MkdirSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = new char[std::strlen(pathBuf)+1];
  std::strcpy(pathname, pathBuf);
  modeVal = mode_value_.val();
  SystemCallTraceReplayModule::prepareRow();
}

MkdiratSystemCallTraceReplayModule::
MkdiratSystemCallTraceReplayModule(DataSeriesModule &source,
  bool verbose_flag,
  int warn_level_flag):
  MkdirSystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor") {
  sys_call_name_ = "mkdirat";
}

void MkdiratSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), ",
    "replayed fd(", replayed_fd, "), ",
    "pathname(", given_pathname_.val(), "), ", \
    "traced mode(", mode_value_.val(), "), ",
    "replayed mode(", get_mode(mode_value_.val()), ")");
}

void MkdiratSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  int dirfd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  const char *pathname = (char *)given_pathname_.val();
  mode_t mode = get_mode(mode_value_.val());

  if (dirfd == SYSCALL_SIMULATED && pathname != NULL && pathname[0] != '/') {
    /*
     * dirfd originated from a socket, hence mkdirat cannot be replayed.
     * Traced system call would have failed with ENOTDIR.
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }
  // Replay the mkdirat system call
  replayed_ret_val_ = mkdirat(dirfd, pathname, mode);
}
