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
 * ExeveSystemCallTraceReplayModule header file.
 *
 * Read ExecveSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "ExecveSystemCallTraceReplayModule.hpp"
#include <sys/socket.h>
#include <sys/types.h>

ExecveSystemCallTraceReplayModule::ExecveSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      given_pathname_(series, "given_pathname", Field::flag_nullable),
      continuation_number_(series, "continuation_number"),
      argument_(series, "argument", Field::flag_nullable),
      environment_(series, "environment", Field::flag_nullable) {
  sys_call_name_ = "execve";
}

void ExecveSystemCallTraceReplayModule::print_sys_call_fields() {
  print_specific_fields();
}

void ExecveSystemCallTraceReplayModule::print_specific_fields() {
  for (auto envPair : environmentVariables) {
    syscall_logger_->log_info("argument(", envPair.second, ")", " environment(",
                              envPair.first, ")");
    delete[] envPair.second;
    delete[] envPair.first;
  }
  print_common_fields();
}

void ExecveSystemCallTraceReplayModule::processRow() {
  /*
   * NOTE: It is not appropriate to replay execve system call.
   * Hence we do not replay execve system call. However, we still need
   * to update fd manager.
   */
  // Get all traced fds in this process
  std::unordered_set<int> traced_fds =
      replayer_resources_manager_.get_all_traced_fds(executingPidVal);
  for (int traced_fd : traced_fds) {
    int flags =
        replayer_resources_manager_.get_flags(executingPidVal, traced_fd);
    int replayed_fd =
        replayer_resources_manager_.get_fd(executingPidVal, traced_fd);
    /*
     * Check to see if fd has O_CLOEXEC flag set.
     * If the FD_CLOEXEC bit is set, the file descriptor will automatically
     * be closed during a successful execve(2). (If the execve(2) fails, the
     * file descriptor
     * is left open.)  If the FD_CLOEXEC bit is not set, the file descriptor
     * will
     * remain open across an execve(2).
     */
    if (((flags & O_CLOEXEC) != 0) && retVal >= 0) {
      replayer_resources_manager_.remove_fd(executingPidVal, traced_fd);
    }

    if (replayed_fd == SYSCALL_SIMULATED) {
      if (((flags & SOCK_CLOEXEC) != 0) || ((flags & 0x80000) != 0)) {
        replayer_resources_manager_.remove_fd(executingPidVal, traced_fd);
      }
    }
  }

  return;
}

void ExecveSystemCallTraceReplayModule::prepareRow() {
  int count = 1;
  continuation_num = continuation_number_.val();
  retVal = return_value();

  // Save the position of the first record
  const void *first_record_pos = series.getCurPos();

  // Count the number of rows processed
  while (continuation_num >= 0 && series.morerecords()) {
    continuation_num = continuation_number_.val();
    if (verbose_) {
      char *environmentVal = nullptr, *argumentVal = nullptr;
      if (continuation_num > 0) {
        if (!environment_.isNull()) {
          auto envBuf = reinterpret_cast<const char *>(environment_.val());
          environmentVal = copyPath(envBuf);
        }
        if (!argument_.isNull()) {
          auto argBuf = reinterpret_cast<const char *>(environment_.val());
          argumentVal = copyPath(argBuf);
        }
        environmentVariables.emplace_back(environmentVal, argumentVal);
      }
    }
    ++series;
    ++count;
  }

  // Set the number of record processed
  rows_per_call_ = count;

  // Again, set the pointer to the first record
  series.setCurPos(first_record_pos);
  SystemCallTraceReplayModule::prepareRow();
  timeReturnedVal = timeRecordedVal;
}
