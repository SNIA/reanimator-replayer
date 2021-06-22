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
 * This header represents an analysis module for system call durations.
 *
 * USAGE
 * A main program can register this analysis module, which will then calculate
 * minimum, maximum, and average elapsed time values for each system call.
 */
#ifndef THREAD_WISE_DURATION_ANALYSIS_MODULE_HPP
#define THREAD_WISE_DURATION_ANALYSIS_MODULE_HPP

#include <map>
#include "AnalysisModule.hpp"
#include "SystemCallTraceReplayModule.hpp"

/**
 * Stores information about the elapsed times for a given syscall (or set of
 * syscalls) at a certain point in the analysis.
 */
struct ThreadWiseDurationAnalysisStruct {
  ThreadWiseDurationAnalysisStruct();

  uint64_t min_time_elapsed;
  uint64_t max_time_elapsed;
  uint64_t average_time_elapsed;
  uint64_t rows;
};

class ThreadWiseDurationAnalysisModule : public AnalysisModule {
 protected:
  uint64_t rollingAverage(uint64_t value, uint64_t old_average, uint64_t rows) const;
  std::ostream& printGlobalMetrics(std::ostream& out) const;
  std::ostream& printPerSyscallMetrics(std::ostream& out) const;

  /**
   * Update the min and max time_elapsed values by considering a new value
   * from a syscall.
   */
  void considerTimeElapsed(uint64_t time_elapsed, ThreadWiseDurationAnalysisStruct& analysis);

  std::map<std::string, ThreadWiseDurationAnalysisStruct> analysisMap_;
  ThreadWiseDurationAnalysisStruct global_metrics_;

 public:
  ThreadWiseDurationAnalysisModule() = default;

  /**
   * Calculate the elapsed time for the system call and update the minimum,
   * maximum, and average times globally and for the specific system call.
   */
  void considerSyscall(const SystemCallTraceReplayModule& module) override;

  /**
   * Print the minimum, maximum, and average elapsed times for each syscall
   * and for all syscalls together.
   */
  std::ostream& printMetrics(std::ostream& out) const override;
};

#endif /* THREAD_WISE_DURATION_ANALYSIS_MODULE_HPP */
