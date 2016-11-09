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
#include <sstream>
#include <errno.h>

#define TIMESTAMP_BUFFER_SIZE 20

class SystemCallTraceReplayLogger {
private:
  std::ofstream logger_file_;

  /*
   * This function actually prints the log messgae to the
   * logger file
   */
  void print_logs(std::stringstream&& log_stream);

  /*
   * This function calls itself recursively to combine the arguments
   * in a stringstream.
   */
  template<typename First, typename...Rest>
  void print_logs(std::stringstream&& log_stream, First&& parm1, Rest&&...parm);

  /*
   * This function is used to print the current time to the log file
   * while appending logs to the log file.
   *
   * @return: returns buffer having timestamp in the format
   *          YYYY-MM-DD HH:MM:SS
   */
  std::string print_time();

public:
  /*
   * This is the constructor of SystemCallTraceReplayLogger
   * class which opens the logger file to write logs.
   */
  SystemCallTraceReplayLogger(std::string log_filename);

  /*
   * This function takes the variable number of arguments and prints
   * the info messages to the logger file.
   */
  template<typename...Args>
  void log_info(Args&&...args);

  /*
   * This function takes the variable number of arguments and prints
   * the error messages to the logger file.
   */
  template<typename...Args>
  void log_err(Args&&...args);

  /*
   * This function takes the variable number of arguments and prints
   * the warning messages to the logger file.
   */
  template<typename...Args>
  void log_warn(Args&&...args);

  /*
   * This function takes the variable number of arguments and prints
   * the debug messages to the logger file.
   */
  template<typename...Args>
  void log_debug(Args&&...args);

  // Destructor
  ~SystemCallTraceReplayLogger();
};

template<typename First, typename...Rest>
void SystemCallTraceReplayLogger::print_logs(std::stringstream&& log_stream,
					     First&& parm1,
					     Rest&&...parm) {
  log_stream << parm1;
  this->print_logs(std::forward<std::stringstream>(log_stream),
		   std::move(parm)...);
}

template<typename...Args>
void SystemCallTraceReplayLogger::log_info(Args&&...args) {
  this->logger_file_ << this->print_time() << "[INFO] ";
  std::stringstream log_stream;
  this->print_logs(std::forward<std::stringstream>(log_stream),
		   std::move(args)...);
  this->logger_file_ << std::endl;
}

template<typename...Args>
void SystemCallTraceReplayLogger::log_err(Args&&...args) {
  this->logger_file_ << this->print_time() << "[ERROR] ";
  std::stringstream log_stream;
  this->print_logs(std::forward<std::stringstream>(log_stream),
		   std::move(args)...);
  this->logger_file_ << std::endl;
}

template<typename...Args>
void SystemCallTraceReplayLogger::log_warn(Args&&...args) {
  this->logger_file_ << this->print_time() << "[WARN] ";
  std::stringstream log_stream;
  this->print_logs(std::forward<std::stringstream>(log_stream),
		   std::move(args)...);
  this->logger_file_ << std::endl;
}

template<typename...Args>
void SystemCallTraceReplayLogger::log_debug(Args&&...args) {
  this->logger_file_ << this->print_time() << "[DEBUG] ";
  std::stringstream log_stream;
  this->print_logs(std::forward<std::stringstream>(log_stream),
		   std::move(args)...);
  this->logger_file_ << std::endl;
}

#endif /* SYSTEM_CALL_TRACE_REPLAY_LOGGER_HPP */
