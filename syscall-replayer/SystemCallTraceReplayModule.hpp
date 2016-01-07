/*
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides basic members and functions used internally
 * for implementing a specific system call.
 *
 * SystemCallTraceReplayerModule is an/a abstract class/basic module that 
 * has basic members and functions of a system call. It can be extended 
 * to implement any specific system call. For example, open, read, write, etc.
 * 
 * INITIALIZATION AND USAGE
 * As part of the abstraction design, implementing a module that replays 
 * a system call requires a class to extend this abstract class and implement 
 * abstract functions - prepareForProcessing(), processRow(), and completeProcessing().
 *
 */
#ifndef SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <DataSeries/RowAnalysisModule.hpp>

#include <string>
#include <map>

#define DEFAULT_MODE 0
#define WARN_MODE    1
#define ABORT_MODE   2

class SystemCallTraceReplayModule : public RowAnalysisModule {
protected:
  std::string sys_call_;
  bool verbose_;
  int  warn_level_;
  DoubleField time_called_;
  Int64Field return_value_;
  bool completed_;

  /* 
   * Compare return value of an operation in trace file and replayed operation
   * 
   * @param ret_val: return value of replayed operation
   */
  void compare_retval(int ret_val);

  /*
   * This function will be called before replaying any corresponding 
   * system call.
   * Note: Child class should implement this funciton to do things
   * before a replayer execute corresponding system call
   */
  virtual void prepareForProcessing() = 0;

  /* 
   * This function is where all the replaying takes action. 
   * It will be called by execute() function for each record 
   * of a corresponding system call.
   * Note: Child class should implement this funciton to replay
   * a specific system call
   */
  virtual void processRow() = 0;
  
  /* 
   * This function will be called after all system call 
   * operations are being replayed.
   * Note: Child class can implement this funciton to do things
   * after all system call operations are replayed
   *
   * @param ret_val: return value of replayed operation
   */
  virtual void completeProcessing() = 0;

public:
  // A mapping of file descriptors in the trace file to actual file descriptors
  static std::map<int, int> fd_map_;
  
  /*
   * Basic Constructor
   *
   * @param source: dataseries module that contains system call traces
   * @param verbose_flag: a flag to indicate to replay this module in 
   *                      verbose mode
   * @param warn_level_flag: a flag to indicate replaying mode
   *
   */  
  SystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag, int warn_level_flag);
  
  /*
   * Get the execution time of current system call record
   *
   * @return: corresponding time_called field of a record that
   *          extent series is pointing to.
   */
  double time_called() const;
  
  /*
   * This function overwrite getSharedExtent() in RowAnalysisModule.
   * It will find a extent test to see if current extent
   * has more records in it. If there are, that means not all
   * operations are replayed. Replayer still need to call execute()
   * to continue replaying.
   *
   * @return: a pointer to an extent
   */
  Extent::Ptr getSharedExtent();
  
  /*
   * This function will test to see if current extent
   * has more records in it. If there are, that means not all
   * operations are replayed. Replayer still need to call execute()
   * to continue replaying.
   *
   * @return: true indicates there are more records in the extent and 
   * false otherwise.
   */
  bool cur_extent_has_more_record();
  
  /*
   * This function will be called by a replayer to replay
   * one record of corresponding(based on class name) system call
   * Note: Replayer should call this function until 
   * there are no more trace record
   */
  void execute();

};

#endif /* SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
