/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * Dup2SystemCallTraceReplayModule header file.
 *
 * Read Dup2SystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "Dup2SystemCallTraceReplayModule.hpp"

Dup2SystemCallTraceReplayModule::
Dup2SystemCallTraceReplayModule(DataSeriesModule &source,
				bool verbose_flag,
				int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  old_descriptor_(series, "old_descriptor"),
  new_descriptor_(series, "new_descriptor") {
  sys_call_name_ = "dup2";
}

void Dup2SystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_old_fd = replayer_resources_manager_.get_fd(pid, old_descriptor_.val());
  int replayed_new_fd = replayer_resources_manager_.get_fd(pid, new_descriptor_.val());

  syscall_logger_->log_info("traced old fd(", old_descriptor_.val(), "), ",
    "replayed old fd(", replayed_old_fd, "), ",
    "traced new fd(", new_descriptor_.val(), "), ",
    "replayed new fd(", replayed_new_fd, ")");
}

void Dup2SystemCallTraceReplayModule::processRow() {
  // Get actual file descriptor
  pid_t pid = executing_pid();
  int old_fd = replayer_resources_manager_.get_fd(pid, old_descriptor_.val());
  int old_fd_flags = replayer_resources_manager_.get_flags(pid, old_descriptor_.val());
  int new_fd = new_descriptor_.val();
  /*
   * The first is to duplicate onto a descriptor that's
   * currently open, usually as part of redirecting
   * stdout or stderr. We can't use new_fd in trace file
   * and need to replace that with our replayed fd.
   * Also, dup2 silently closes the file descriptor newfd if
   * it was previously open. Therefore, we need to
   * update our mapping.
   */
  if (replayer_resources_manager_.has_fd(pid, new_fd)) {
    new_fd = replayer_resources_manager_.get_fd(pid, new_fd);
    replayer_resources_manager_.remove_fd(pid, new_fd);
  } else {
    new_fd = replayer_resources_manager_.generate_unused_fd(pid);
  }

  // The two file descriptors do not share file descriptor flags (the close-on-exec flag).
  int new_fd_flags = old_fd_flags & ~O_CLOEXEC;
  replayed_ret_val_ = dup2(old_fd, new_fd);

  // Map replayed duplicated file descriptor to traced duplicated file descriptor
  replayer_resources_manager_.add_fd(pid, return_value(), replayed_ret_val_, new_fd_flags);
}
