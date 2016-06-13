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
#include <errno.h>

#define DEFAULT_MODE 0
#define WARN_MODE    1
#define ABORT_MODE   2

class SystemCallTraceReplayModule : public RowAnalysisModule {
protected:
  std::string sys_call_name_;
  bool verbose_;
  int  warn_level_;
  DoubleField time_called_;
  Int64Field time_returned_;
  Int64Field time_recorded_;
  Int32Field executing_pid_;
  Int32Field errno_number_;
  Int64Field return_value_;
  Int64Field unique_id_;
  bool completed_;
  int replayed_ret_val_;
  /*
   * Print common and specific sys call field values in a nice format
   */
  void print_sys_call_fields();

  /*
   * Print common sys call field values in a nice format
   */
  void print_common_fields();

  /*
   * Print specific sys call field values in a nice format
   *
   * Note: child class should implement this function if it wants
   * print_sys_call_fields to print out specific sys call fields values.
   */
  virtual void print_specific_fields() = 0;

  /*
   * Compare return value of an operation in trace file and replayed operation
   * 
   * @param replayed_ret_val: return value of replayed operation
   */
  void compare_retval_and_errno();

  /*
   * This function will be called before replaying any corresponding 
   * system call.
   * Note: Child class should implement this function to do things
   * before a replayer execute corresponding system call
   */
  virtual void prepareForProcessing() = 0;

  /* 
   * This function is where all the replaying takes action. 
   * It will be called by execute() function for each record 
   * of a corresponding system call.
   * Note: Child class should implement this function to replay
   * a specific system call
   */
  virtual void processRow() = 0;

  /*
   * after_sys_call is called by execute() function.
   * This function gets called after a system call is replayed.
   * Currently, it prints system call fields if the replayer
   * is running in verbose mode.
   * Note: You can override this function to do things that
   * you want to do after replaying a system call.
   */
  virtual void after_sys_call();
  
  /* 
   * This function will be called after all system call 
   * operations are being replayed.
   * Note: Child class can implement this function to do things
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
   * Determine whether or not to replay in verbose mode
   *
   * @return: true if it is in verbose mode, false otherwise
   */
  bool verbose_mode() const;

  /*
   * Determine whether or not to replay in default mode
   *
   * @return: true if it is in default mode, false otherwise
   */
  bool default_mode() const;

  /*
   * Determine whether or not to replay in warn mode
   *
   * @return: true if it is in warn mode, false otherwise
   */
  bool warn_mode() const;

  /*
   * Determine whether or not to replay in abort mode
   *
   * @return: true if it is in abort mode, false otherwise
   */
  bool abort_mode() const;

  /*
   * Get the execution time of current system call record
   *
   * @return: corresponding time_called field of a record that
   *           extent series is pointing to.
   */
  double time_called() const;

  /*
   * Get the time the current system call returned according to the record
   *
   * @return: corresponding time_returned field of a record that
   *	       extent series is pointing to.
   */
  uint64_t time_returned() const;

  /*
   * Get the time the current system call record was written
   *
   * @return corresponding time_recorded field of a record that
   *	      extent series is pointing to.
   */
  uint64_t time_recorded() const;

  /*
   * Get the process id of current system call record
   *
   * @return: corresponding executing_pid field of a record that
   *	       extent series is pointing to.
   */
  int executing_pid() const;

  /*
   * Get the error number of current system call record
   *
   * @return: corresponding errno_number field of a record that
   *           extent series is pointing to.
   */
  int errno_number() const;

  /*
   * Get the return value of current system call record (not replayed ret value)
   *
   * @return: corresponding return value field of a record that
   *           extent series is pointing to.
   */
  int return_value() const;

  /*
   * Get the unique id of current system call record
   *
   * @return: corresponding unique id field of a record that
   *           extent series is pointing to.
   */
  int64_t unique_id() const;

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

  /*
   * Convert a time value stored in Tfrac units (2^32 Tfracs = 1 sec)
   * as a uint64_t to seconds (as a double)
   *
   * @return: the corresponding time value in seconds
   */
  double Tfrac_to_sec(uint64_t time);

};

#endif /* SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
