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
 * GetRLimitSystemCallTraceReplayModule header file
 *
 * Read GetRLimitSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "GetRLimitSystemCallTraceReplayModule.hpp"

GetRLimitSystemCallTraceReplayModule::GetRLimitSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : BasicRLimitSystemCallTraceReplayModule(source, verbose_flag,
                                             warn_level_flag),
      verify_(verify_flag) {
  sys_call_name_ = "getrlimit";
}

void GetRLimitSystemCallTraceReplayModule::processRow() {
  int resource = getResource();
  struct rlimit rlim;
  replayed_ret_val_ = getrlimit(resource, &rlim);
  if (verify_) {
    GetRLimitSystemCallTraceReplayModule::verifyResult(statfs_buf);
  }
}

void GetRLimitSystemCallTraceReplayModule::verifyResult(struct rlimit *rlim) {
  rlim_t soft_limit = getSoftLimit();
  rlim_t hard_limit = getSoftLimit();

  // Verify rlim struct contents in the trace file are same
  if (soft_limit != rlim->rlim_cur || soft_limit != rlim->rlim_max) {
    // Not same
    if (verbose_mode()) {
      syscall_logger_->log_err("Verification of ", sys_call_name_,
                               " struct rlimit content failed.");
    }
    if (!default_mode()) {
      syscall_logger_->log_warn(
          "time called:",
          boost::format(DEC_PRECISION) % Tfrac_to_sec(time_called()),
          "Captured ", sys_call_name_,
          " struct rlimit content is different from replayed ", sys_call_name_,
          " content");
      syscall_logger_->log_warn(
          "Captured soft resource limit: ", soft_limit, ", ",
          "Replayed system soft resource limit: ", rlim->rlim_cur);
      syscall_logger_->log_warn(
          "Captured hard resource limit: ", hard_limit, ", ",
          "Replayed system hard resource limit: ", rlim->rlim_max);
      if (abort_mode()) {
        abort();
      }
    }
  } else {
    if (verbose_mode()) {
      syscall_logger_->log_info("Verification of ", sys_call_name_,
                                " struct rlimit succeeded.");
    }
  }
}
