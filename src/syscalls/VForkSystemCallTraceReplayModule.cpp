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
 * VForkSystemCallTraceReplayModule header file.
 *
 * Read VForkSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "VForkSystemCallTraceReplayModule.hpp"
#include <thread>
#include "tbb/atomic.h"
#include "tbb/concurrent_vector.h"

extern tbb::concurrent_vector<std::thread> threads;
extern void executionThread(int64_t threadID);
extern tbb::atomic<uint64_t> nThreads;
extern std::function<void(int64_t, SystemCallTraceReplayModule *)> setRunning;

VForkSystemCallTraceReplayModule::VForkSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag) {
  sys_call_name_ = "vfork";
}

void VForkSystemCallTraceReplayModule::print_specific_fields() {}

void VForkSystemCallTraceReplayModule::processRow() {
  /*
   * Here, we will create a new file descriptor mapping for
   * the process created by vfork.
   * NOTE: It is inappropriate to replay vfork system call.
   * Hence we do not replay vfork system call.
   * A call to vfork() is equivalent to calling clone(2) with flags
   * specified as: CLONE_VM | CLONE_VFORK | SIGCHLD
   */
  bool shared_umask = false, shared_files = false;
  pid_t ppid = executing_pid();
  pid_t pid = return_value();
  // Clone resources tables
  SystemCallTraceReplayModule::replayer_resources_manager_.clone_umask(
      ppid, pid, shared_umask);
  SystemCallTraceReplayModule::replayer_resources_manager_.clone_fd_table(
      ppid, pid, shared_files);

  nThreads++;
  setRunning(pid, nullptr);
  threads.emplace_back(executionThread, pid);
}

void VForkSystemCallTraceReplayModule::prepareRow() {
  SystemCallTraceReplayModule::prepareRow();
}
