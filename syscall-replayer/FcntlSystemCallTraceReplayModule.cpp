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
 * FcntlSystemCallTraceReplayModule header file
 *
 * Read FcntlSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "FcntlSystemCallTraceReplayModule.hpp"

FcntlSystemCallTraceReplayModule::
FcntlSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
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
  if ((command_value_.val() == F_SETLK) ||
      (command_value_.val() == F_SETLKW) ||
      (command_value_.val() == F_GETLK)) {
    LOG_INFO("descriptor(" << descriptor_.val() << "), " \
      << "command value(" << command_value_.val() << "), " \
      << "lock type(" << lock_type_.val() << "), " \
      << "lock whence(" << lock_whence_.val() << "), " \
      << "lock start(" << lock_start_.val() << "), " \
      << "lock length(" << lock_length_.val() << "), " \
      << "lock pid(" << lock_pid_.val() << ")");
  } else {
    LOG_INFO("descriptor(" << descriptor_.val() << "), " \
      << "command value(" << command_value_.val() << "), " \
      << "argument value(" << argument_value_.val() << ")");
  }
}

void FcntlSystemCallTraceReplayModule::processRow() {
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  int command = command_value_.val();
  int argument = argument_value_.val();
  struct flock lock;

  /*
   * Replay the fcntl system call
   * If fcntl was passed a command that requires a struct flock,
   * set the values of the flock and pass it to fcntl as the third argument.
   */
  if ((command == F_SETLK) ||
      (command == F_SETLKW) ||
      (command == F_GETLK)) {
    lock.l_type = lock_type_.val();
    lock.l_whence = lock_whence_.val();
    lock.l_start = lock_start_.val();
    lock.l_len = lock_length_.val();
    lock.l_pid = lock_pid_.val();

    /*
     * If every value in the flock structure is set as 0, replay the system
     * call with NULL as the third argument
     */
    if ((lock.l_type == 0) &&
    	(lock.l_whence == 0) &&
    	(lock.l_start == 0) &&
    	(lock.l_len == 0) &&
    	(lock.l_pid == 0)) {
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
}
