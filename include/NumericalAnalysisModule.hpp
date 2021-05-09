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
 * This header represents an analysis module for numerical fields in system
 * calls.
 *
 * USAGE
 * A main program can register this analysis module, which will then calculate
 * minimum, maximum, and average elapsed time values for each system call.
 */
#ifndef NUMERICAL_ANALYSIS_MODULE_HPP
#define NUMERICAL_ANALYSIS_MODULE_HPP

#include <map>
#include <vector>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include "AnalysisModule.hpp"
#include "SystemCallTraceReplayModule.hpp"

using namespace boost::accumulators;

/**
 * Stores information about metrics for a given field.
 */
struct FieldStruct {
  FieldStruct();

  accumulator_set<uint64_t,
    stats<tag::mean,
          tag::variance,
          tag::median,
          tag::max,
          tag::tail_quantile<left>,
          tag::tail_quantile<right> >> acc;
};

struct SyscallStruct {
    SyscallStruct() = default;

    uint64_t count;
    std::map<std::string, FieldStruct> fields;
};

class NumericalAnalysisModule : public AnalysisModule {
 protected:

  std::ostream& printSyscallMetrics(std::ostream& out,
    const std::pair<std::string, SyscallStruct>&) const;

  std::map<std::string, SyscallStruct> syscalls_;
 
 public:
  NumericalAnalysisModule() = default;

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

#endif /* NUMERICAL_ANALYSIS_MODULE_HPP */
