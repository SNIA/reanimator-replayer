/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * UnlinkSystemCallTraceReplayModule header file
 *
 * Read UnlinkSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "UnlinkSystemCallTraceReplayModule.hpp"

UnlinkSystemCallTraceReplayModule::UnlinkSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      given_pathname_(series, "given_pathname", Field::flag_nullable) {
  sys_call_name_ = "unlink";
}

void UnlinkSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", pathname, ")");
}

void UnlinkSystemCallTraceReplayModule::processRow() {
  // Replay the unlink system call
  replayed_ret_val_ = unlink(pathname);
  delete[] pathname;
}

void UnlinkSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = copyPath(pathBuf);
  SystemCallTraceReplayModule::prepareRow();
}

UnlinkatSystemCallTraceReplayModule::UnlinkatSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : UnlinkSystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      descriptor_(series, "descriptor"),
      flag_value_(series, "flag_value", Field::flag_nullable) {
  sys_call_name_ = "unlinkat";
}

void UnlinkatSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), ",
                            "replayed fd(", replayed_fd, "), ", "pathname(",
                            given_pathname_.val(), "), ", "flags(",
                            flag_value_.val(), ")");
}

void UnlinkatSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  pid_t pid = executing_pid();
  int dirfd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  auto path = reinterpret_cast<const char *>(given_pathname_.val());
  int flags = flag_value_.val();

  if (dirfd == SYSCALL_SIMULATED && path != nullptr && path[0] != '/') {
    /*
     * dirfd originated from a socket, hence unlinkat cannot be replayed.
     * Traced system call would have failed with ENOTDIR.
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }
  // Replay the unlinkat system call
  replayed_ret_val_ = unlinkat(dirfd, path, flags);
}
