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
 * OpenSystemCallTraceReplayModule header file
 *
 * Read OpenSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "OpenSystemCallTraceReplayModule.hpp"
#include <cstring>
#include <memory>

static char path_print[256];
// #define WEBSERVER_TESTING

OpenSystemCallTraceReplayModule::OpenSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      given_pathname_(series, "given_pathname", Field::flag_nullable),
      open_value_(series, "open_value", Field::flag_nullable),
      mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "open";
}

void OpenSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", path_print, "), flags(", flags, "),",
                            "traced mode(", modeVal, "), ", "replayed mode(",
                            get_mode(modeVal), ")");
}

void OpenSystemCallTraceReplayModule::processRow() {
  // replay the open system call
  replayed_ret_val_ = open(pathname, flags, get_mode(modeVal));
  if (traced_fd <= -1 && replayed_ret_val_ != -1) {
    /*
     * Original system open failed, but replay system succeeds.
     * Therefore, we will close the replayed fd.
     */
    close(replayed_ret_val_);
  } else {
#ifdef WEBSERVER_TESTING
    if (replayer_resources_manager_.has_fd(executingPidVal, traced_fd)) {
      replayer_resources_manager_.remove_fd(executingPidVal, traced_fd);
    }
#endif
    /*
     * Even if traced fd is valid, but replayed fd is -1,
     * we will still add the entry and replay it.
     * Add a mapping from fd in trace file to actual replayed fd
     */
    replayer_resources_manager_.add_fd(executingPidVal, traced_fd,
                                       replayed_ret_val_, flags);
  }
  if (verbose_mode()) {
    strcpy(path_print, pathname);
  }
  delete[] pathname;
}

void OpenSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = copyPath(pathBuf);
  flags = open_value_.val();
  modeVal = mode_value_.val();
  traced_fd = reinterpret_cast<int64_t>(return_value_.val());
  SystemCallTraceReplayModule::prepareRow();
}

OpenatSystemCallTraceReplayModule::OpenatSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : OpenSystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      descriptor_(series, "descriptor") {
  sys_call_name_ = "openat";
}

void OpenatSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info(
      "traced fd(", descriptor_.val(), "), ", "replayed fd(", replayed_fd,
      "), ", "pathname(", given_pathname_.val(), "), flags(", open_value_.val(),
      "), ", "traced mode(", mode_value_.val(), "), ", "replayed mode(",
      get_mode(mode_value_.val()), ")");
}

void OpenatSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  int dirfd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  const char *pathname = reinterpret_cast<const char *>(given_pathname_.val());
  int flags = open_value_.val();
  mode_t mode = get_mode(mode_value_.val());
  int64_t traced_fd = reinterpret_cast<int64_t>(return_value_.val());

  if (dirfd == SYSCALL_SIMULATED && pathname != nullptr && pathname[0] != '/') {
    /*
     * dirfd originated from a socket, hence openat cannot be replayed.
     * Traced system call would have failed with ENOTDIR.
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }

  // replay the openat system call
  replayed_ret_val_ = openat(dirfd, pathname, flags, mode);
  if (traced_fd == -1 && replayed_ret_val_ != -1) {
    /*
     * Original system open failed, but replay system succeeds.
     * Therefore, we will close the replayed fd.
     */
    close(replayed_ret_val_);
  } else {
    /*
     * Even if traced fd is valid, but replayed fd is -1,
     * we will still add the entry and replay it.
     * Add a mapping from fd in trace file to actual replayed fd
     */
    replayer_resources_manager_.add_fd(pid, traced_fd, replayed_ret_val_,
                                       flags);
  }
}
