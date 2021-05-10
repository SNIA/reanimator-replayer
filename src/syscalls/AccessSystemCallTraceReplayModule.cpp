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
 * AccessSystemCallTraceReplayModule header file
 *
 * Read AccessSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "AccessSystemCallTraceReplayModule.hpp"
#include <cstring>
#include <memory>

AccessSystemCallTraceReplayModule::AccessSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      given_pathname_(series, "given_pathname", Field::flag_nullable),
      mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "access";
}

void AccessSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", pathname,
                            "), "
                            "traced mode(",
                            mode_value, "), ", "replayed mode(",
                            get_mode(mode_value), ")");
  // delete[] pathname;
}

void AccessSystemCallTraceReplayModule::processRow() {
  // Replay the access system call
  replayed_ret_val_ = access(pathname, get_mode(mode_value));

  if (!verbose_mode()) {
    delete[] pathname;
  }
}

void AccessSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = new char[std::strlen(pathBuf) + 1];
  std::strcpy(pathname, pathBuf);
  mode_value = mode_value_.val();
  SystemCallTraceReplayModule::prepareRow();
}

FAccessatSystemCallTraceReplayModule::FAccessatSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : AccessSystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      descriptor_(series, "descriptor"),
      flags_value_(series, "flags_value", Field::flag_nullable) {
  sys_call_name_ = "faccessat";
}

void FAccessatSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info(
      "traced fd(", descriptor_.val(), "), ", "replayed fd(", replayed_fd, ")",
      "pathname(", given_pathname_.val(), "), ", "traced mode(",
      mode_value_.val(), "), ", "replayed mode(", get_mode(mode_value_.val()),
      ")", "flag(", flags_value_.val(), ")");
}

void FAccessatSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  const char *pathname = (char *)given_pathname_.val();
  int mode = get_mode(mode_value_.val());
  int flags = flags_value_.val();

  if (replayed_fd == SYSCALL_SIMULATED && pathname != nullptr &&
      pathname[0] != '/') {
    /*
     * replayed_fd originated from a socket, hence faccessat cannot be replayed.
     * Traced system call would have failed with ENOTDIR.
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }

  // Replay the faccessat system call
  replayed_ret_val_ = faccessat(replayed_fd, pathname, mode, flags);
}
