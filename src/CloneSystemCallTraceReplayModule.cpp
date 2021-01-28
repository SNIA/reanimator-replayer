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
 * CloneSystemCallTraceReplayModule header file.
 *
 * Read CloneSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "CloneSystemCallTraceReplayModule.hpp"
#include <thread>
#include "tbb/atomic.h"
#include "tbb/concurrent_vector.h"

extern tbb::concurrent_vector<std::thread> threads;
extern void executionThread(int64_t threadID);
extern tbb::atomic<uint64_t> nThreads;
extern std::function<void(int64_t, SystemCallTraceReplayModule *)> setRunning;

CloneSystemCallTraceReplayModule::CloneSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      flag_value_(series, "flag_value", Field::flag_nullable),
      child_stack_address_(series, "child_stack_address"),
      parent_thread_id_(series, "parent_thread_id", Field::flag_nullable),
      child_thread_id_(series, "child_thread_id", Field::flag_nullable),
      new_tls_(series, "new_tls", Field::flag_nullable) {
  sys_call_name_ = "clone";
}

void CloneSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("flags(", boost::format("0x%02x") % flagVal, "), ",
                            "child stack address(", childStackAddrVal, "), ",
                            "parent thread id(", parentTIDVal, "), ",
                            "child thread id(", childTIDVal, "), ", "new tls(",
                            newTLSVal, ")");
}

void CloneSystemCallTraceReplayModule::processRow() {
  /*
   * Here, we will create a new file descriptor mapping for
   * the process created by clone. If the CLONE_FILES flag is not set,
   * then the cloned process will get its own file descriptor map copied
   * from that of the parent process. If that flag is set, then the cloned
   * process id will be mapped to the parent process id, and the two processes
   * will share a file descriptor table, as they would in the kernel.
   * NOTE: It is inappropriate to replay clone system call.
   * Hence we do not replay clone system call.
   */
  int flags = flagVal;
  bool shared_umask = false, shared_files = false;
  if ((flags & CLONE_FS) != 0 || (flags & CLONE_THREAD) != 0) {
    shared_umask = true;
  }
  if ((flags & CLONE_FILES) != 0 || (flags & CLONE_THREAD) != 0) {
    shared_files = true;
  }

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

void CloneSystemCallTraceReplayModule::prepareRow() {
  flagVal = flag_value_.val();
  childStackAddrVal = child_stack_address_.val();
  parentTIDVal = parent_thread_id_.val();
  childTIDVal = child_thread_id_.val();
  newTLSVal = new_tls_.val();
  SystemCallTraceReplayModule::prepareRow();
}
