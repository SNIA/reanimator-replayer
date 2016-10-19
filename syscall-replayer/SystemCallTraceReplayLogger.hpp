/*
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Nina Brown
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Geoff Kuenning
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions for implementing logger
 * for System Call Trace Replay Module.
 *
 * SystemCallTraceReplayLogger is a class that has members and functions for
 * printing log messages during replaying.
 *
 * USAGE
 * A main program could initialize this class and call desired
 * function to add logs in the logger file.
 */

#ifndef SYSTEM_CALL_TRACE_REPLAY_LOGGER_HPP
#define SYSTEM_CALL_TRACE_REPLAY_LOGGER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <errno.h>

#define ERROR_MSG "ERROR"
#define WARN_MSG "WARN"
#define INFO_MSG "INFO"

#define TIMESTAMP_BUFFER_SIZE 20
#define NEWLINE "\n"

#define PRINT_LOG(msg, TYPE) (SystemCallTraceReplayLogger::getInstance())->logger_file_ << \
			     (SystemCallTraceReplayLogger::getInstance())->print_time() << \
			     "[" << TYPE << "]" << " " << __FILE__ << "(" << \
			     __LINE__ << ") " << msg << NEWLINE

#define LOG_ERR(msg) PRINT_LOG(msg, ERROR_MSG)
#define LOG_WARN(msg) PRINT_LOG(msg, WARN_MSG)
#define LOG_INFO(msg) PRINT_LOG(msg, INFO_MSG)

class SystemCallTraceReplayLogger {
private:
  // Constructor
  SystemCallTraceReplayLogger();

public:
  static std::ofstream logger_file_;
  static SystemCallTraceReplayLogger *logger;

  /*
   * This function initiaizes the single instance of the
   * SystemCallTraceReplayLogger class and opens the logger
   * file to write logs.
   */
  static void initialize(std::string log_filename);

  /*
   * This function returns the singleton instance of the
   * SystemCallTraceReplayLogger class.
   */
  static SystemCallTraceReplayLogger *getInstance();

  /*
   * This function closes the logger file.
   */
  void close_stream();

  /*
   * This function is used to print the current time to the log file
   * while appending logs to the log file.
   *
   * @return: returns buffer having timestamp in the format
   *          YYYY-MM-DD HH:MM:SS
   */
  char *print_time();
};

#endif /* SYSTEM_CALL_TRACE_REPLAY_LOGGER_HPP */
