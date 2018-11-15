/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions in order to replay
 * munmap system call.
 *
 * MunmapSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying munmap system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef MUNMAP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define MUNMAP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

class MunmapSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  // DataSeries Munmap System Call Trace Fields
  Int64Field start_address_;
  Int64Field length_;

  /**
   * Print munmap sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will simply return without replaying
   * munmap system call.
   */
  void processRow();

public:
  MunmapSystemCallTraceReplayModule(DataSeriesModule &source,
				    bool verbose_flag,
				    int warn_level_flag);
  SystemCallTraceReplayModule *move() {
    auto movePtr = new MunmapSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal, timeRecordedVal,
                       executingPidVal, errorNoVal, returnVal, replayerIndex);
    return movePtr;
  }
};

#endif /* MUNMAP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
