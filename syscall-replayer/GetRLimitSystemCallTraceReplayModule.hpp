/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions for implementing getrlimit
 * system call.
 *
 * GetRLimitSystemCallTraceReplayModule is a class/module that
 * has members and functions of replaying getrlimit system call.
 *
 * USAGE
 * A main program could initialize this object with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef GETRLIMIT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define GETRLIMIT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "BasicRLimitSystemCallTraceReplayModule.hpp"

class GetRLimitSystemCallTraceReplayModule
    : public BasicRLimitSystemCallTraceReplayModule {
  /**
   * This function will gather arguments in the trace file
   * and then replay getrlimit system call with those arguments.
   */
  void processRow();

 public:
  GetRLimitSystemCallTraceReplayModule(DataSeriesModule &source,
                                       bool verbose_flag, int warn_level_flag);

  /**
   * Verify to see if the resource limit on replaying
   * system is same as on the traced system.
   */
  void verifyResult();
};
#endif /* GETRLIMIT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
