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
 * Dup3SystemCallTraceReplayModule header file.
 *
 * Read Dup3SystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "Dup3SystemCallTraceReplayModule.hpp"

Dup3SystemCallTraceReplayModule::Dup3SystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      old_descriptor_(series, "old_descriptor"),
      new_descriptor_(series, "new_descriptor"),
      flags_(series, "flags") {
  sys_call_name_ = "dup3";
}

void Dup3SystemCallTraceReplayModule::print_specific_fields() {
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

void Dup3SystemCallTraceReplayModule::processRow() {
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
   * 1. We need to simulate dup3 on SYSCALL_SIMULATED as old_fd.
   * If new_fd existed in fd_map, we remove it; since dup3 silently closes
   * any open new_fd.
   * Whatever the value of new_fd, we add a mapping of
   * new_fd <-> SYSCALL_SIMULATED to fd_map.
   * We do not generate an fd, irrespective of the new_fd value.
   *
   * 2. Dup3 has to silently close new_fd and reuse it.
   * We do remove SYSCALL_SIMULATED from fd_map, but cannot reuse
   * SYSCALL_SIMULATED.
   * Dup3 needs to be replayed here. Hence we generate an fd, then
   * add new_fd <-> generated fd to fd_map.
   *
   * 3. Here, new_fd exists in fd_map, so first remove it.
   * Reuse the new_fd mapping to replay dup3 (this avoids an expensive
   * call to generate fd).
   * Re-insert new_fd mapping in the fd_map.
   *
   * 4. Here, new_fd does not exists in fd_map.
   * Generate an unused fd. Replay dup3, and insert
   * new_fd <-> generated fd into fd_map.
   */

  // In all 4 actions above, if new_fd exists in fd_map, we remove it.
  if (replayer_resources_manager_.has_fd(pid, new_fd)) {
    replayed_new_fd = replayer_resources_manager_.get_fd(pid, new_fd);
    replayer_resources_manager_.remove_fd(pid, new_fd);
  }

  // The two file descriptors do not share file descriptor flags (the
  // close-on-exec flag) in dup2 O_CLOEXEC disables this is the only difference
  int new_fd_flags = old_fd_flags;

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

  replayed_ret_val_ = dup3(old_fd, replayed_new_fd, flags);

  // Map replayed duplicated file descriptor to traced duplicated file
  // descriptor
  replayer_resources_manager_.add_fd(pid, return_value(), replayed_ret_val_,
                                     new_fd_flags);
}

void Dup3SystemCallTraceReplayModule::prepareRow() {
  old_file_descriptor = old_descriptor_.val();
  new_file_descriptor = new_descriptor_.val();
  flags = flags_.val();
  SystemCallTraceReplayModule::prepareRow();
}
