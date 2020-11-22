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
 * This header file provides members and functions in order to replay
 * exit system call.
 *
 * ExitSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying exit system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef EXIT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define EXIT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

class ExitSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 private:
  // DataSeries Exit System Call Trace Fields
  Int32Field exit_status_;
  BoolField generated_;

  int32_t exitStat;
  bool generated;

  /**
   * Print exit sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will simply return without replaying
   * exit system call.
   */
  void processRow() override;

 public:
  ExitSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                  int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr =
        new ExitSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(exitStat, generated);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(int32_t exitStatus, bool isGenerated) {
    exitStat = exitStatus;
    generated = isGenerated;
  }
  void prepareRow() override;
};

#endif /* EXIT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
