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
 * mmap system call.
 *
 * MmapSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying mmap system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef MMAP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define MMAP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

class MmapSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 private:
  // DataSeries Mmap System Call Trace Fields
  Int64Field start_address_;
  Int64Field length_;
  Int32Field protection_value_;
  Int32Field flags_value_;
  Int32Field descriptor_;
  Int64Field offset_;

  int64_t startAddress;
  int64_t sizeOfMap;
  int32_t protectionVal;
  int32_t flagsVal;
  int32_t descriptorVal;
  int64_t offsetVal;
  uint64_t mmapReturnVal;

  /**
   * Print mmap sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will simply return without replaying
   * mmap system call.
   */
  void processRow() override;

 public:
  MmapSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                  int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr =
        new MmapSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(startAddress, sizeOfMap, protectionVal, flagsVal,
                     descriptorVal, offsetVal, mmapReturnVal);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }

  void setMove(int64_t startAddr, int64_t lengthOfMap, int32_t protection,
               int32_t flag, int32_t desc, int64_t offset, uint64_t mretval) {
    startAddress = startAddr;
    sizeOfMap = lengthOfMap;
    protectionVal = protection;
    flagsVal = flag;
    descriptorVal = desc;
    offsetVal = offset;
    mmapReturnVal = mretval;
  }

  void prepareRow() override;
};

#endif /* MMAP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
