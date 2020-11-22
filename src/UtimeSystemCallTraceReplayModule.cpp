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
 * UtimeSystemCallTraceReplayModule header file
 *
 * Read UtimeSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "UtimeSystemCallTraceReplayModule.hpp"

UtimeSystemCallTraceReplayModule::UtimeSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      verify_(verify_flag),
      /*
       * Pathname is nullable for utimensat, but not for utime and utimes.
       * However, making it nullable here does not affect the performance
       * of utime and utimes as long as it is non-nullable in the
       * strace2ds library.
       */
      given_pathname_(series, "given_pathname", Field::flag_nullable),
      access_time_(series, "access_time", Field::flag_nullable),
      mod_time_(series, "mod_time", Field::flag_nullable) {
  sys_call_name_ = "utime";
}

void UtimeSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info(
      "given_pathname(", given_pathname_.val(), "), ", "access_time(",
      boost::format(DEC_PRECISION) % Tfrac_to_sec(access_time_.val()), "), ",
      "mod_time(", boost::format(DEC_PRECISION) % Tfrac_to_sec(mod_time_.val()),
      ")");
}

void UtimeSystemCallTraceReplayModule::processRow() {
  // Get replaying file given_pathname.
  struct utimbuf utimebuf;

  // Replay the utime system call.
  if (access_t != -1 && mod_t != -1) {
    /*
     * If access_time and mod_time are both null, then assume the utimbuf
     * is NULL.
     */
    if (verify_) {
      syscall_logger_->log_info(
          "Utime was passed NULL as its second argument.",
          " It will assign the current time to the file's",
          " access_time and mod_time.");
    }
    replayed_ret_val_ = utime(pathname, nullptr);
  } else {
    utimebuf.actime = Tfrac_to_sec(access_t);
    utimebuf.modtime = Tfrac_to_sec(mod_t);

    replayed_ret_val_ = utime(pathname, &utimebuf);
  }
}

void UtimeSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = copyPath(pathBuf);

  if (access_time_.isNull()) {
    access_t = -1;
  } else {
    access_t = access_time_.val();
  }

  if (mod_time_.isNull()) {
    mod_t = -1;
  } else {
    mod_t = mod_time_.val();
  }
  SystemCallTraceReplayModule::prepareRow();
}

UtimesSystemCallTraceReplayModule::UtimesSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : UtimeSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                       warn_level_flag) {
  sys_call_name_ = "utimes";
}

void UtimesSystemCallTraceReplayModule::processRow() {
  // Get replaying file given_pathname and make timeval array.
  struct timeval tv[2];
  auto pathname = reinterpret_cast<const char *>(given_pathname_.val());

  // Replay the utimes system call.
  if ((access_time_.isNull()) && (mod_time_.isNull())) {
    /*
     * If access_time and mod_time are both null, then assume the timeval
     * array is NULL.
     */
    if (verify_) {
      syscall_logger_->log_info(
          "Utimes was passed NULL as its second argument.",
          " It will assign the current time to the file's",
          " access_time and mod_time.");
    }
    replayed_ret_val_ = utimes(pathname, nullptr);
  } else {
    struct timeval tv_access_time = Tfrac_to_timeval(access_time_.val());
    struct timeval tv_mod_time = Tfrac_to_timeval(mod_time_.val());
    tv[0] = tv_access_time;
    tv[1] = tv_mod_time;

    replayed_ret_val_ = utimes(pathname, tv);
  }
}

UtimensatSystemCallTraceReplayModule::UtimensatSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : UtimeSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                       warn_level_flag),
      descriptor_(series, "descriptor"),
      flag_value_(series, "flag_value", Field::flag_nullable) {
  sys_call_name_ = "utimensat";
}

void UtimensatSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info(
      "traced fd(", descriptor_.val(), "), ", "replayed fd(", replayed_fd,
      "), ", "given_pathname(", given_pathname_.val(), "), ", "access_time(",
      boost::format(DEC_PRECISION) % Tfrac_to_sec(access_time_.val()), "), ",
      "mod_time(", boost::format(DEC_PRECISION) % Tfrac_to_sec(mod_time_.val()),
      "), ", "flags(", flag_value_.val(), ")");
}

void UtimensatSystemCallTraceReplayModule::processRow() {
  // Get replaying file given_pathname and make timespec array.
  pid_t pid = executing_pid();
  int dirfd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  struct timespec ts[2];
  const char *pathname;
  if (given_pathname_.isNull()) {
    pathname = nullptr;
  } else {
    pathname = reinterpret_cast<const char *>(given_pathname_.val());
  }
  int flags = flag_value_.val();

  if (dirfd == SYSCALL_SIMULATED && pathname != nullptr && pathname[0] != '/') {
    /*
     * dirfd originated from a socket, hence utimensat cannot be replayed.
     * Traced system call would have failed with ENOTDIR.
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }

  // Replay the utimensat system call.
  if ((access_time_.isNull()) && (mod_time_.isNull())) {
    /*
     * If access_time and mod_time are both null, then assume the timespec
     * array is NULL.
     */
    if (verify_) {
      syscall_logger_->log_info(
          "Utimensat was passed NULL as its third argument.",
          " It will assign the current time to the file's",
          " access_time and mod_time.");
    }
    /*
     * XXX: The glibc wrapper for utimensat will give the error EINVAL
     * if the pathname is NULL, while, for the system call itself,
     * NULL is a valid pathname in some situations.  Calling the wrapper
     * resulted in errors replaying a cp -a command, so we call the
     * utimensat system call directly. - Nina
     */
    replayed_ret_val_ = syscall(SYS_utimensat, dirfd, pathname, NULL, flags);
  } else {
    struct timespec ts_access_time = Tfrac_to_timespec(access_time_.val());
    struct timespec ts_mod_time = Tfrac_to_timespec(mod_time_.val());
    ts[0] = ts_access_time;
    ts[1] = ts_mod_time;

    replayed_ret_val_ = syscall(SYS_utimensat, dirfd, pathname, ts, flags);
  }
}
