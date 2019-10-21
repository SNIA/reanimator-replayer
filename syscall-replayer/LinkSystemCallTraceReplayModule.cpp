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
 * LinkSystemCallTraceReplayModule header file
 *
 * Read LinkSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "LinkSystemCallTraceReplayModule.hpp"

LinkSystemCallTraceReplayModule::LinkSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      given_oldpathname_(series, "given_oldpathname", Field::flag_nullable),
      given_newpathname_(series, "given_newpathname", Field::flag_nullable) {
  sys_call_name_ = "link";
}

void LinkSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("old path(", old_pathname, "), ", "new path(",
                            new_pathname, ")");
  delete[] new_pathname;
  delete[] old_pathname;
}

void LinkSystemCallTraceReplayModule::processRow() {
  // Replay the link system call
  replayed_ret_val_ = link(old_pathname, new_pathname);

  if (!verbose_mode()) {
    delete[] new_pathname;
    delete[] old_pathname;
  }
}

void LinkSystemCallTraceReplayModule::prepareRow() {
  auto old_pathBuf = reinterpret_cast<const char *>(given_oldpathname_.val());
  old_pathname = copyPath(old_pathBuf);

  auto new_pathBuf = reinterpret_cast<const char *>(given_newpathname_.val());
  new_pathname = copyPath(new_pathBuf);
  SystemCallTraceReplayModule::prepareRow();
}

LinkatSystemCallTraceReplayModule::LinkatSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : LinkSystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      old_descriptor_(series, "old_descriptor"),
      new_descriptor_(series, "new_descriptor"),
      flag_value_(series, "flag_value", Field::flag_nullable) {
  sys_call_name_ = "linkat";
}

void LinkatSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_old_fd =
      replayer_resources_manager_.get_fd(pid, old_descriptor_.val());
  int replayed_new_fd =
      replayer_resources_manager_.get_fd(pid, new_descriptor_.val());

  syscall_logger_->log_info(
      "traced old fd(", old_descriptor_.val(), "), ", "replayed old fd(",
      replayed_old_fd, "), ", "traced new fd(", new_descriptor_.val(), "), ",
      "replayed new fd(", replayed_new_fd, "), ", "old path(",
      given_oldpathname_.val(), "), ", "new path(", given_newpathname_.val(),
      "), ", "flags(", flag_value_.val(), ")");
}

void LinkatSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  int old_fd = replayer_resources_manager_.get_fd(pid, old_descriptor_.val());
  int new_fd = replayer_resources_manager_.get_fd(pid, new_descriptor_.val());
  const char *old_path_name = (const char *)given_oldpathname_.val();
  const char *new_path_name = (const char *)given_newpathname_.val();
  int flags = flag_value_.val();

  if ((old_fd == SYSCALL_SIMULATED && old_path_name != nullptr &&
       old_path_name[0] != '/') ||
      (new_fd == SYSCALL_SIMULATED && new_path_name != nullptr &&
       new_path_name[0] != '/')) {
    /*
     * Either old_fd or new_fd or both originated from a socket.
     * Linkat cannot be replayed.
     * Traced system call would have failed with ENOTDIR.
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }
  // Replay the linkat system call
  replayed_ret_val_ =
      linkat(old_fd, old_path_name, new_fd, new_path_name, flags);
}
