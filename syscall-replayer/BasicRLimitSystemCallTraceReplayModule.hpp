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
 * This header file provides basic members and functions for implementing
 * resource related
 * system call.
 *
 * BasicRLimitSystemCallTraceReplayerModule is a class/module that
 * has basic members and functions of replaying resource related system call.
 *
 * USAGE
 * A class could extends this class to implement a resource related system
 * call
 */
#ifndef BASICRLIMIT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define BASICRLIMIT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <sys/resource.h>
#include <sys/time.h>

#include "SystemCallTraceReplayModule.hpp"

class BasicRLimitSystemCallTraceReplayModule
    : public SystemCallTraceReplayModule {
 protected:
  // rlimit System Call Trace Fields in Dataseries file
  Int32Field resource_value_;
  Int64Field resource_soft_limit_;
  Int64Field resource_hard_limit_;

  /**
   * Get resource
   *
   * @return: resource
   */
  int getResource();

  /**
   * Get soft limit
   *
   * @return: soft limit
   */
  rlim_t getSoftLimit();

  /**
   * Get hard limit (ceiling for rlim_cur)
   *
   * @return: hard limit
   */
  rlim_t getHardLimit();

  /**
   * Print getrlimit sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and then replay resource related system call with those arguments.
   */
  void processRow() override = 0;

 public:
  BasicRLimitSystemCallTraceReplayModule(DataSeriesModule &source,
                                         bool verbose_flag,
                                         int warn_level_flag);
};
#endif /* BASICRLIMIT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
