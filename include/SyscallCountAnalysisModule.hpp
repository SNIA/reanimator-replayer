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
 * This header represents an analysis module for counting system calls.
 *
 * USAGE
 * A main program can register this analysis module, which will then count the
 * occurrences of each system call.
 */
#ifndef SYSCALL_COUNT_ANALYSIS_MODULE_HPP
#define SYSCALL_COUNT_ANALYSIS_MODULE_HPP

#include <map>
#include "AnalysisModule.hpp"
#include "SystemCallTraceReplayModule.hpp"

class SyscallCountAnalysisModule : public AnalysisModule {
 protected:
  std::ostream& printGlobalMetrics(std::ostream& out) const;

  /**
   * Stores counts of each syscall.
   */
  std::map<std::string, uint64_t> analysisMap_;
 
 public:
  SyscallCountAnalysisModule() = default;

  /**
   * Add one to the count of the given syscall.
   */
  void considerSyscall(const SystemCallTraceReplayModule& module) override;

  /**
   * Print the counts for all syscalls seen.
   */
  std::ostream& printMetrics(std::ostream& out) const override;
};

#endif /* SYSCALL_COUNT_ANALYSIS_MODULE_HPP */
