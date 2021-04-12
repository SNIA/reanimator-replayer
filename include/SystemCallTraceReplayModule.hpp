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
 * This header file provides basic members and functions used internally
 * for implementing a specific system call.
 *
 * SystemCallTraceReplayerModule is an abstract class that has basic members
 * and functions of a system call.  It can be extended to implement any
 * specific system call.  For example, open, read, write, etc.
 *
 * INITIALIZATION AND USAGE
 * As part of the abstraction design, implementing a module that replays
 * a system call requires a class to extend this abstract class and implement
 * abstract function - processRow().
 */
#ifndef SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <DataSeries/RowAnalysisModule.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include "AnalysisModule.hpp"
#include "ReplayerResourcesManager.hpp"
#include "SystemCallTraceReplayLogger.hpp"
#include "strace2ds.h"

#define DEFAULT_MODE 0
#define WARN_MODE 1
#define ABORT_MODE 2
#define SYSCALL_FAILURE ((-1))
#define SYSCALL_SIMULATED ((-2))
/*
 * DEC_PRECISION specifies the format for printing precision of decimal
 * values upto 25 decimal places in logger file.
 */
#define DEC_PRECISION "%.25f"

class SystemCallTraceReplayModule : public RowAnalysisModule {
 protected:
  std::string sys_call_name_;
  bool verbose_;
  int warn_level_;
  Int64Field time_called_;
  Int64Field time_returned_;
  Int64Field time_recorded_;
  Int32Field executing_pid_;
  Int32Field errno_number_;
  Int64Field return_value_;
  Int64Field unique_id_;
  int rows_per_call_;  // It stores the number of rows processed per system
                       // call.
  int replayed_ret_val_;

  int64_t uniqueIdVal;
  int64_t timeCalledVal;
  int64_t timeReturnedVal;
  int64_t timeRecordedVal;
  int64_t executingPidVal;
  int errorNoVal;
  int64_t returnVal;

  int64_t replayerIndex;

  /**
   * Print common and specific sys call field values in a nice format
   */
  virtual void print_sys_call_fields();

  /**
   * Print common sys call field values in a nice format
   */
  void print_common_fields();

  /**
   * Print specific sys call field values in a nice format
   *
   * Note: child class should implement this function if it wants
   * print_sys_call_fields to print out specific sys call fields values.
   */
  virtual void print_specific_fields() = 0;

  /**
   * Compare return value of an operation in trace file and replayed operation
   *
   * @param replayed_ret_val: return value of replayed operation
   */
  void compare_retval_and_errno();

  /**
   * This function is where all the replaying takes action.
   * It will be called by execute() function for each record
   * of a corresponding system call.
   * Note: Child class should implement this function to replay
   * a specific system call
   */
  void processRow() override = 0;

  /**
   * This function will be called after processRow() of correponding
   * system call is called. It calls the after_sys_call() which does
   * the post checking of each record processed.  Finally it increments
   * the pointer in the Extent Series by the number of rows processed
   * per system call.
   */

  void completeProcessing() override;

  /**
   * after_sys_call is called by completeProcessing() function.
   * This function gets called after a system call is replayed.
   * Currently, it prints system call fields if the replayer
   * is running in verbose mode.
   * Note: You can override this function to do things that
   * you want to do after replaying a system call.
   */
  virtual void after_sys_call();

  /**
   * This function is a helper function that masks mode value argument
   * of a system call because we are managing our own umask values.
   * This means that all system calls that have mode_val will
   * use this function to get the mode value that needs to be passed
   * to system call, syscall ex: mkdir, open, etc.
   * The conversion logic:
   * 1. getting the current PID umask value
   * 2. ~umask AND mode bits
   * Return ~umask AND mode bits
   */
  mode_t get_mode(mode_t mode);

 public:
  // A resource manager for umask and file descriptors
  static ReplayerResourcesManager replayer_resources_manager_;
  // An input file stream for reading random data from /dev/urandom
  static std::ifstream random_file_;
  // An object of logger class
  static SystemCallTraceReplayLogger *syscall_logger_;

  /**
   * Basic Constructor
   *
   * @param source: dataseries module that contains system call traces
   * @param verbose_flag: a flag to indicate to replay this module in
   *                      verbose mode
   * @param warn_level_flag: a flag to indicate replaying mode
   *
   */
  SystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                              int warn_level_flag);

  /**
   * Determine whether or not to replay in verbose mode
   *
   * @return: true if it is in verbose mode, false otherwise
   */
  bool verbose_mode() const;

  /**
   * Determine whether or not to replay in default mode
   *
   * @return: true if it is in default mode, false otherwise
   */
  bool default_mode() const;

  /**
   * Determine whether or not to replay in warn mode
   *
   * @return: true if it is in warn mode, false otherwise
   */
  bool warn_mode() const;

  /**
   * Determine whether or not to replay in abort mode
   *
   * @return: true if it is in abort mode, false otherwise
   */
  bool abort_mode() const;

  /**
   * Get the system call name of current system call record
   *
   * @return: a string that represents the system call name.
   */
  std::string sys_call_name() const;

  /**
   * Get the execution time of current system call record
   *
   * @return: corresponding time_called field of a record that
   *          extent series is pointing to.
   */
  uint64_t time_called() const;

  /**
   * Get the time the current system call returned according to the record
   *
   * @return: corresponding time_returned field of a record that
   *	       extent series is pointing to.
   */
  uint64_t time_returned() const;

  /**
   * Get the time the current system call record was written
   *
   * @return corresponding time_recorded field of a record that
   *	      extent series is pointing to.
   */
  uint64_t time_recorded() const;

  /**
   * Get the process id of current system call record
   *
   * @return: corresponding executing_pid field of a record that
   *	       extent series is pointing to.
   */
  uint32_t executing_pid() const;

  /**
   * Get the error number of current system call record
   *
   * @return: corresponding errno_number field of a record that
   *          extent series is pointing to.
   */
  int errno_number() const;

  /**
   * Get the return value of current system call record
   * (not replayed ret value)
   *
   * @return: corresponding return value field of a record that
   *          extent series is pointing to.
   */
  int64_t return_value() const;

  /**
   * Get the unique id of current system call record
   *
   * @return: corresponding unique id field of a record that
   *          extent series is pointing to.
   */
  int64_t unique_id() const;

  /**
   * This function overwrite getSharedExtent() in RowAnalysisModule.
   * It will find a extent test to see if current extent
   * has more records in it.  If there are, that means not all
   * operations are replayed.  Replayer still need to call execute()
   * to continue replaying.
   *
   * @return: a pointer to an extent
   */
  Extent::Ptr getSharedExtent() override;

  /**
   * This function will test to see if current extent
   * has more records in it.  If there are, that means not all
   * operations are replayed.  Replayer still need to call execute()
   * to continue replaying.
   *
   * @return: true indicates there are more records in the extent and
   * false otherwise.
   */
  bool cur_extent_has_more_record();

  /**
   * is_version_compatible() determines whether current extent is
   * compatible with major_v.minor_v. Return true if current extent
   * has version x.y and y <= minor_v and x == major_v.
   *
   * @return: true indicates current extent is compatible with version
   * major_v.minor_v.
   */
  bool is_version_compatible(unsigned int major_v, unsigned int minor_v);

  /**
   * This function will be called by a replayer to replay
   * one record of corresponding(based on class name) system call
   * Note: Replayer should call this function until
   * there are no more trace record
   */
  void execute();

  /**
   * This function will be called by a replayer to analyze one record of
   * corresponding (based on class name) system call
   * Note: Replayer should call this function until there are no more trace
   * record
   */
  void analyze(AnalysisModule &module);

  /**
   * Displays fun facts about system calls
   */
  void displayAnalysisResults(AnalysisModule &module);

  /**
   * Perform analysis on the Dataseries extent for this system call.
   *
   * Called instead of processRow() when analyzing a trace.
   */
  virtual void analyzeRow(AnalysisModule &module);

  /**
   * Convert a time value stored in nanosecond units (10^9 nsec = 1 sec)
   * (as a uint64_t) to seconds (as a double)
   *
   * @return: the corresponding time value in seconds
   */
  double nsec_to_sec(uint64_t time);

  /**
   * Convert a time value stored in Tfrac units (2^32 Tfracs = 1 sec)
   * (as a uint64_t) to seconds (as a double)
   *
   * @return: the corresponding time value in seconds
   */
  double Tfrac_to_sec(uint64_t time);

  /**
   * Convert a time value stored in Tfrac units (2^32 Tfracs = 1 sec)
   * (as a uint64_t) to a struct timeval (for the utimes system call)
   *
   * @return: the corresponding timeval
   */
  struct timeval Tfrac_to_timeval(uint64_t time);

  /**
   * Convert a time value stored in Tfrac units (2^32 Tfracs = 1 sec)
   * (as a uint64_t) to a struct timespec (for the utimensat system call)
   *
   * @return: the corresponding timespec
   */
  struct timespec Tfrac_to_timespec(uint64_t time);

  /**
   * For system calls such as write, pwrite and writev, replayer has
   * an option to fill buffers with zeros, any pattern or random values.
   * If pattern is set as random, this function will randomly generate
   * numbers using standard rand() and srand() library functions to
   * fill the buffer.
   */
  char *random_fill_buffer(char *buffer, size_t nbytes);

  /**
   * Some system calls such as _exit, execve, mmap and munmap are not
   * appropriate to replay. So we do not replay in our replayer.
   *
   * @return: returns true if the system call is replayable, else it
   *	      returns false.
   */
  bool isReplayable();

  /**
   * Some system calls such as _exit, execve, mmap and munmap have garbage
   * time_returned values, so we do not want to analyze their durations.
   *
   * @return: returns true if the system call is timeable, else it returns
   *        false.
   */
  bool isTimeable() const;

  virtual void prepareRow();
  void setCommon(int64_t id, int64_t called, int64_t returned, int64_t recorded,
                 int64_t pid, int error, int ret, int64_t index) {
    uniqueIdVal = id;
    timeCalledVal = called;
    timeReturnedVal = returned;
    timeRecordedVal = recorded;
    executingPidVal = pid;
    errorNoVal = error;
    returnVal = ret;
    replayerIndex = index;
  }
  virtual SystemCallTraceReplayModule *move() { return nullptr; }

  inline char *copyPath(const char *source) {
    auto path_size = std::strlen(source) + 1;
    auto new_path = new char[path_size];
    std::strncpy(new_path, source, path_size);
    return new_path;
  }

  int64_t getReplayerIndex() { return replayerIndex; }

  void setReplayerIndex(int64_t idx) { replayerIndex = idx; }
};

#endif /* SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
