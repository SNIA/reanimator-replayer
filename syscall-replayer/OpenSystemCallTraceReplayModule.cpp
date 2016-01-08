/*
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
 * This file implements all the functions in the OpenSystemCallTraceReplayModule 
 * header file 
 *
 * Read OpenSystemCallTraceReplayModule.hpp for more information about this class.
 */

#include "OpenSystemCallTraceReplayModule.hpp"

OpenSystemCallTraceReplayModule::OpenSystemCallTraceReplayModule(DataSeriesModule &source,
								 bool verbose_flag, 
								 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname(series, "given_pathname"),
  flag_read_only(series, "flag_read_only"),
  flag_write_only(series, "flag_write_only"),
  flag_read_and_write(series, "flag_read_and_write"),
  flag_append(series, "flag_append"),
  flag_async(series, "flag_async"),
  flag_close_on_exec(series, "flag_close_on_exec"),
  flag_create(series, "flag_create"),
  flag_direct(series, "flag_direct"),
  flag_directory(series, "flag_directory"),
  flag_exclusive(series, "flag_exclusive"),
  flag_largefile(series, "flag_largefile"),
  flag_no_access_time(series, "flag_no_access_time"),
  flag_no_controlling_terminal(series, "flag_no_controlling_terminal"),
  flag_no_follow(series, "flag_no_follow"),
  flag_no_blocking_mode(series, "flag_no_blocking_mode"),
  flag_no_delay(series, "flag_no_delay"),
  flag_synchronous(series, "flag_synchronous"),
  flag_truncate(series, "flag_truncate"),
  mode_R_user(series, "mode_R_user"),
  mode_W_user(series, "mode_W_user"),
  mode_X_user(series, "mode_X_user"),
  mode_R_group(series, "mode_R_group"),
  mode_W_group(series, "mode_W_group"),
  mode_X_group(series, "mode_X_group"),
  mode_R_others(series, "mode_R_others"),
  mode_W_others(series, "mode_W_others"),
  mode_X_others(series, "mode_X_others") {
  sys_call_ = "open";
}

int OpenSystemCallTraceReplayModule::getFlags() {
  // Add all the access flags.
  int access_mode_sum = flag_read_only.val() + flag_write_only.val() + flag_read_and_write.val();
  // Test to see if multiple access flags are set.
  if (access_mode_sum != 1) {
    /*
     * Error since it is only allowed to set one of 
     * three access modes: O_RDONLY, O_WRONLY, or O_RDWR
     */
    std::cerr << "Error: Multiple access modes are set" 
	      << "(Only one access mode O_RDONLY, O_WRONLY, or O_RDWR can be set)."
	      << std::endl;
    return -1;
  }
  int flags = O_RDONLY;
  if (flag_read_only.val() == 1) {
    flags = O_RDONLY;
  } else if (flag_write_only.val() == 1) {
    flags = O_WRONLY;
  } else if (flag_read_and_write.val() == 1) {
    flags = O_RDWR;
  }
  // Find additional flags and bitwise-or those flags.
  if (flag_append.val() == 1) {
    flags |= O_APPEND;
  }
  if (flag_async.val() == 1) {
    flags |= O_ASYNC;
  }
  if (flag_close_on_exec.val() == 1) {
    flags |= O_CLOEXEC;
  }
  if (flag_create.val() == 1) {
    flags |= O_CREAT;
  }
  if (flag_direct.val() == 1) {
    flags |= O_DIRECT;
  }
  if (flag_directory.val() == 1) {
    flags |= O_DIRECTORY;
  }
  if (flag_exclusive.val() == 1) {
    flags |= O_EXCL;
  }
  if (flag_largefile.val() == 1) {
    flags |= O_LARGEFILE;
  }
  if (flag_no_access_time.val() == 1) {
    flags |= O_NOATIME;
  }
  if (flag_no_controlling_terminal.val() == 1) {
    flags |= O_NOCTTY;
  }
  if (flag_no_follow.val() == 1) {
    flags |= O_NOFOLLOW;
  }
  if (flag_no_blocking_mode.val() == 1) {
    flags |= O_NONBLOCK;
  }
  if (flag_no_delay.val() == 1) {
    flags |= O_NDELAY;
  }
  if (flag_synchronous.val() == 1) {
    flags |= O_SYNC;
  }
  if (flag_truncate.val() == 1) {
    flags |= O_TRUNC;
  }
  return flags;
}

mode_t OpenSystemCallTraceReplayModule::getMode() {
  mode_t mode = 0;
  /* 
   * Find what modes were in the trace file
   * and bitwise-or them
   */
  if (mode_R_user.val() == 1) {
    mode |= S_IRUSR;
  }
  if (mode_W_user.val() == 1) {
    mode |= S_IWUSR;
  }
  if (mode_X_user.val() == 1) {
    mode |= S_IXUSR;
  }
  if (mode_R_group.val() == 1) {
    mode |= S_IRGRP;
  }
  if (mode_W_group.val() == 1) {
    mode |= S_IWGRP;
  }
  if (mode_X_group.val() == 1) {
    mode |= S_IXGRP;
  }
  if (mode_R_others.val() == 1) {
    mode |= S_IROTH;
  }
  if (mode_W_others.val() == 1) {
    mode |= S_IWOTH;
  }
  if (mode_X_others.val() == 1) {
    mode |= S_IXOTH;
  }    
  /*
    Keep the following code in the comment for now.
    May use it for debugging purpose
    std::cout << "mode_R_user: " << mode_R_user.val() << std::endl;
    std::cout << "mode_W_user: " << mode_W_user.val() << std::endl;
    std::cout << "mode_X_user: " << mode_X_user.val() << std::endl;
    std::cout << "mode_R_group: " << mode_R_group.val() << std::endl;
    std::cout << "mode_W_group: " << mode_W_group.val() << std::endl;
    std::cout << "mode_X_group: " << mode_X_group.val() << std::endl;
    std::cout << "mode_R_others: " << mode_R_others.val() << std::endl;
    std::cout << "mode_W_others: " << mode_W_others.val() << std::endl;
    std::cout << "mode_X_others: " << mode_X_others.val() << std::endl;
  */
  return mode;
}

void OpenSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Open System Call Replayer starts to replay...-----" << std::endl;
}

void OpenSystemCallTraceReplayModule::processRow() {
  char *pathname = (char *)given_pathname.val();
  int flags = getFlags();
  // Check to see if this operation has a valid flag.
  if (flags == -1) {
    std::cout << given_pathname.val() << " is NOT successfully opened." << std::endl;
    return;
  }
  mode_t mode = getMode();
  int return_value = (int)return_value_.val();

  if (verbose_) {
    std::cout << "open: ";
    std::cout.precision(25);
    std::cout << "time called(" << std::fixed << time_called() << "), ";
    std::cout << "pathname(" << pathname << "), ";
    std::cout << "flags(" << flags << "), ";
    std::cout << "mode(" << mode << ")\n";
  }
  
  // replay the open system call
  int replay_ret = open(pathname, flags, mode);
  compare_retval(replay_ret);
  // Add a mapping from fd in trace file to actual replayed fd
  SystemCallTraceReplayModule::fd_map_[return_value] = replay_ret;

  if (replay_ret == -1) {
    perror(pathname);
  } else {
    if (verbose_) {
      std::cout << given_pathname.val() << " is successfully opened..." << std::endl;
    }
  }
}

void OpenSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Open System Call Replayer finished replaying...-----" << std::endl;
}
