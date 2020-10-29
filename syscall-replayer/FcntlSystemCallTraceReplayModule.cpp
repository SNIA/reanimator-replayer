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
 * FcntlSystemCallTraceReplayModule header file
 *
 * Read FcntlSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "FcntlSystemCallTraceReplayModule.hpp"

FcntlSystemCallTraceReplayModule::FcntlSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      descriptor_(series, "descriptor"),
      command_value_(series, "command_value", Field::flag_nullable),
      argument_value_(series, "argument_value", Field::flag_nullable),
      lock_type_(series, "lock_type", Field::flag_nullable),
      lock_whence_(series, "lock_whence", Field::flag_nullable),
      lock_start_(series, "lock_start", Field::flag_nullable),
      lock_length_(series, "lock_length", Field::flag_nullable),
      lock_pid_(series, "lock_pid", Field::flag_nullable) {
  sys_call_name_ = "fcntl";
}

void FcntlSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, traced_fd);
  if ((command_val == F_SETLK) || (command_val == F_SETLKW) ||
      (command_val == F_GETLK)) {
    syscall_logger_->log_info(
        "traced fd(", traced_fd, "), ", "replayed fd(", replayed_fd, "), ",
        "command value(", command_val, "), ", "lock type(", lock_type_val,
        "), ", "lock whence(", lock_whence_val, "), ", "lock start(",
        lock_start_val, "), ", "lock length(", lock_length_val, "), ",
        "lock pid(", lock_pid_val, ")");
  } else {
    syscall_logger_->log_info("traced fd(", traced_fd, "), ", "replayed fd(",
                              replayed_fd, "), ", "command value(", command_val,
                              "), "
                              "argument value(",
                              arg_val, ")");
  }
}

void FcntlSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, traced_fd);
  int command = command_val;
  int argument = arg_val;
  struct flock lock;

  if (fd == SYSCALL_SIMULATED) {
    /*
    * FD for the Fcntl system call originated from a socket().
    * The system call will not be replayed.
    * Original return value will be returned.
    */
    replayed_ret_val_ = simulated_ret_val;
  }
  /*
   * Replay the fcntl system call If fcntl was passed a command that requires a
   * struct flock,
   * set the values of the flock and pass it to fcntl as the third argument.
   */
  if ((command == F_SETLK) || (command == F_SETLKW) || (command == F_GETLK)) {
    lock.l_type = lock_type_val;
    lock.l_whence = lock_whence_val;
    lock.l_start = lock_start_val;
    lock.l_len = lock_length_val;
    lock.l_pid = lock_pid_val;

    /*
     * If every value in the flock structure is set as 0, replay the system
     * call with NULL as the third argument
     */
    if ((lock.l_type == 0) && (lock.l_whence == 0) && (lock.l_start == 0) &&
        (lock.l_len == 0) && (lock.l_pid == 0)) {
      replayed_ret_val_ = fcntl(fd, command, NULL);
    } else {
      /*
       * If not, replay it with the corresponding flock structure as
       * the third argument
       */
      replayed_ret_val_ = fcntl(fd, command, &lock);
    }
  } else {
    // Otherwise, pass fcntl the argument value as the third argument.
    replayed_ret_val_ = fcntl(fd, command, argument);
  }

  /*
   * If command has F_DUPFD flag set, then map the returned file descriptor
   * value to the traced return value.
   */
  if (command == F_DUPFD || command == F_DUPFD_CLOEXEC) {
    // Get actual file descriptor
    pid_t pid = executing_pid();
    int fd_flags = replayer_resources_manager_.get_flags(pid, traced_fd);
    /*
     * The two file descriptors do not share file descriptor flags (the
     * close-on-exec flag),
     * unless F_DUPFD_CLOEXEC is set
     */
    int new_fd_flags = fd_flags & ~O_CLOEXEC;
    if (command == F_DUPFD_CLOEXEC) {
      new_fd_flags |= O_CLOEXEC;
    }
    replayer_resources_manager_.add_fd(pid, return_value(), replayed_ret_val_,
                                       new_fd_flags);
  } else if (command == F_SETFD) {
    if (argument == FD_CLOEXEC) {
      // Get actual file descriptor
      pid_t pid = executing_pid();
      replayer_resources_manager_.add_flags(pid, traced_fd, O_CLOEXEC);
    }
  }
}

void FcntlSystemCallTraceReplayModule::prepareRow() {
  traced_fd = descriptor_.val();
  command_val = command_value_.val();
  arg_val = argument_value_.val();
  simulated_ret_val = return_value_.val();
  lock_type_val = lock_type_.val();
  lock_whence_val = lock_whence_.val();
  lock_start_val = lock_start_.val();
  lock_length_val = lock_length_.val();
  lock_pid_val = lock_pid_.val();
  SystemCallTraceReplayModule::prepareRow();
}
