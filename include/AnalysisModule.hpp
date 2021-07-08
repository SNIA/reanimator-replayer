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
 * This header file describes the state of the current trace analysis.
 *
 * USAGE
 * A main program can calculate various metrics per system call in a trace
 * file, then update the global metrics in this class.
 */
#ifndef ANALYSIS_MODULE_HPP
#define ANALYSIS_MODULE_HPP

#include <unistd.h>
#include <sys/types.h>

// Forward declaration of replay module
class SystemCallTraceReplayModule;

class AnalysisModule {
 public:
  AnalysisModule() = default;
  virtual ~AnalysisModule() = default;

  /**
   * Takes the current trace replay module operating on a given system call and
   * analyzes the common fields using public getter functions. Must be
   * implemented by each subclass to perform specific analysis behavior.
   */
  virtual void considerSyscall(const SystemCallTraceReplayModule& module) = 0;

  /**
   * Output the metrics collected by the analysis module to a given stream.
   */
  virtual std::ostream& printMetrics(std::ostream& out) const = 0;
};

#endif /* ANALYSIS_MODULE_HPP */
