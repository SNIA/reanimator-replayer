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
 * Dup2SystemCallTraceReplayModule header file.
 *
 * Read Dup2SystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "Dup2SystemCallTraceReplayModule.hpp"

Dup2SystemCallTraceReplayModule::Dup2SystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      old_descriptor_(series, "old_descriptor"),
      new_descriptor_(series, "new_descriptor") {
  sys_call_name_ = "dup2";
}

void Dup2SystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_old_fd =
      replayer_resources_manager_.get_fd(pid, old_file_descriptor);
  int replayed_new_fd =
      replayer_resources_manager_.get_fd(pid, new_file_descriptor);

  syscall_logger_->log_info("traced old fd(", old_file_descriptor, "), ",
                            "replayed old fd(", replayed_old_fd, "), ",
                            "traced new fd(", new_file_descriptor, "), ",
                            "replayed new fd(", replayed_new_fd, ")");
}

void Dup2SystemCallTraceReplayModule::processRow() {
  // Get actual file descriptor
  pid_t pid = executing_pid();
  int old_fd = replayer_resources_manager_.get_fd(pid, old_file_descriptor);
  int old_fd_flags =
      replayer_resources_manager_.get_flags(pid, old_file_descriptor);
  int new_fd = new_file_descriptor;
  int replayed_new_fd = SYSCALL_SIMULATED;

  /*
   *
   * To support simulation in cases of FDs originating from sockets,
   * there are 6 possible non failure cases.
   *
   * +-----------+------------------+--------+
   * |  old_fd   |      new_fd      | action |
   * +-----------+------------------+--------+
   * | SIMULATED | SIMULATED        |      1 |
   * | SIMULATED | Valid & Open     |      1 |
   * | SIMULATED | Valid & Not open |      1 |
   * | Valid     | SIMULATED        |      2 |
   * | Valid     | Valid & Open     |      3 |
   * | Valid     | Valid & Not open |      4 |
   * +-----------+------------------+--------+
   *
   * Actions:
   * 1. We need to simulate dup2 on SYSCALL_SIMULATED as old_fd.
   * If new_fd existed in fd_map, we remove it; since dup2 silently closes
   * any open new_fd.
   * Whatever the value of new_fd, we add a mapping of
   * new_fd <-> SYSCALL_SIMULATED to fd_map.
   * We do not generate an fd, irrespective of the new_fd value.
   *
   * 2. Dup2 has to silently close new_fd and reuse it.
   * We do remove SYSCALL_SIMULATED from fd_map, but cannot reuse
   * SYSCALL_SIMULATED.
   * Dup2 needs to be replayed here. Hence we generate an fd, then
   * add new_fd <-> generated fd to fd_map.
   *
   * 3. Here, new_fd exists in fd_map, so first remove it.
   * Reuse the new_fd mapping to replay dup2 (this avoids an expensive
   * call to generate fd).
   * Re-insert new_fd mapping in the fd_map.
   *
   * 4. Here, new_fd does not exists in fd_map.
   * Generate an unused fd. Replay dup2, and insert
   * new_fd <-> generated fd into fd_map.
   */

  // In all 4 actions above, if new_fd exists in fd_map, we remove it.
  if (replayer_resources_manager_.has_fd(pid, new_fd)) {
    replayed_new_fd = replayer_resources_manager_.get_fd(pid, new_fd);
    replayer_resources_manager_.remove_fd(pid, new_fd);
  }

  // The two file descriptors do not share file descriptor flags (the
  // close-on-exec flag).
  int new_fd_flags = old_fd_flags & ~O_CLOEXEC;

  if (old_fd == SYSCALL_SIMULATED) {
    /*
     * old_fd originated from a socket() system call.
     * Add SYSCALL_SIMULATED as replayed_new_fd in the mapping.
     * Return the replayed return value.
     * Action: 1
     */

    replayed_ret_val_ = return_value();
    replayer_resources_manager_.add_fd(pid, replayed_ret_val_,
                                       SYSCALL_SIMULATED, new_fd_flags);
    return;
  }

  if (replayed_new_fd == SYSCALL_SIMULATED) {
    // Action: 2, 4
    replayed_new_fd = replayer_resources_manager_.generate_unused_fd(pid);
  }

  replayed_ret_val_ = dup2(old_fd, replayed_new_fd);

  // Map replayed duplicated file descriptor to traced duplicated file
  // descriptor
  replayer_resources_manager_.add_fd(pid, return_value(), replayed_ret_val_,
                                     new_fd_flags);
}

void Dup2SystemCallTraceReplayModule::prepareRow() {
  old_file_descriptor = old_descriptor_.val();
  new_file_descriptor = new_descriptor_.val();
  SystemCallTraceReplayModule::prepareRow();
}
