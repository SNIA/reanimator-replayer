/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * LSeekSystemCallTraceReplayModule header file
 *
 * Read LSeekSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "SetRLimitSystemCallTraceReplayModule.hpp"

SetRLimitSystemCallTraceReplayModule::SetRLimitSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : BasicRLimitSystemCallTraceReplayModule(source, verbose_flag,
                                             warn_level_flag) {
  sys_call_name_ = "setrlimit";
}

void SetRLimitSystemCallTraceReplayModule::processRow() {
  rlim_t soft_limit = getSoftLimit();
  rlim_t hard_limit = getHardLimit();
  if (soft_limit == RLIM_INFINITY || hard_limit == RLIM_INFINITY) {
    int resource = getResource();
    struct rlimit rlim;
    rlim.rlim_cur = soft_limit;
    rlim.rlim_cur = hard_limit;
    replayed_ret_val_ = setrlimit(resource, &rlim);
  } else {
    if (verbose_mode() || !default_mode()) {
      syscall_logger_->log_warn(
          sys_call_name_,
          ": Set resource limit to value other than RLIM_INFINITY is \
        unsupported");
    }
  }
}
