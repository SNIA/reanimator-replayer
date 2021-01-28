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
 * FChmodSystemCallTraceReplayModule header file
 *
 * Read FChmodSystemCallTraceReplayModule.hpp for more information about this
 * class.
 */

#include "FChmodSystemCallTraceReplayModule.hpp"

FChmodSystemCallTraceReplayModule::FChmodSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      descriptor_(series, "descriptor"),
      mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "fchmod";
}

void FChmodSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, traced_fd);

  syscall_logger_->log_info("traced fd(", traced_fd, "), ", "replayed fd(",
                            replayed_fd, "), ", "traced mode(", mode_value, ")",
                            "replayed mode(", get_mode(mode_value), ")");
}

void FChmodSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, traced_fd);
  mode_t mode = get_mode(mode_value);

  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the fchmod call originated from a socket().
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value();
    return;
  }
  // Replay the fchmod system call
  replayed_ret_val_ = fchmod(fd, mode);
}

void FChmodSystemCallTraceReplayModule::prepareRow() {
  traced_fd = descriptor_.val();
  mode_value = mode_value_.val();
  SystemCallTraceReplayModule::prepareRow();
}

FChmodatSystemCallTraceReplayModule::FChmodatSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : FChmodSystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      given_pathname_(series, "given_pathname", Field::flag_nullable),
      flag_value_(series, "flag_value", Field::flag_nullable) {
  sys_call_name_ = "fchmodat";
}

void FChmodatSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());

  syscall_logger_->log_info(
      "traced fd(", descriptor_.val(), "), ", "replayed fd(", replayed_fd,
      "), ", "given_pathname(", given_pathname_.val(), "), ", "traced mode(",
      mode_value_.val(), "), ", "replayed mode(", get_mode(mode_value_.val()),
      "), ", "flag(", flag_value_.val(), ")");
}

void FChmodatSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  const char *pathname = reinterpret_cast<const char *>(given_pathname_.val());
  mode_t mode = get_mode(mode_value_.val());
  int flags = flag_value_.val();

  if (fd == SYSCALL_SIMULATED && pathname != nullptr && pathname[0] != '/') {
    /*
     * fd originated from a socket, hence fchmodat cannot be replayed.
     * Traced system call would have failed with ENOTDIR.
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }
  // Replay the fchmodat system call
  replayed_ret_val_ = fchmodat(fd, pathname, mode, flags);
}
