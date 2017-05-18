/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Nina Brown
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Geoff Kuenning
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the SystemCallTraceReplayLogger
 * header file.
 *
 * Read SystemCallTraceReplayLogger.hpp for more information about this class.
 */

#include "SystemCallTraceReplayLogger.hpp"

SystemCallTraceReplayLogger::SystemCallTraceReplayLogger(std::string log_filename) {
  // Open log file in append mode
  this->logger_file_.open(log_filename.c_str(),
			  std::ios_base::app | std::ios_base::out);
  if (!(this->logger_file_.is_open()) && !log_filename.empty()) {
    std::cerr << "Unable to open log file" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void SystemCallTraceReplayLogger::print_logs(std::stringstream&& log_stream) {
  this->logger_file_ << log_stream.str();
}

std::string SystemCallTraceReplayLogger::format_time() {
  static char buffer[TIMESTAMP_BUFFER_SIZE];
  time_t rawtime;
  struct tm *curr_time;

  time(&rawtime);
  curr_time = localtime(&rawtime);

  strftime(buffer, TIMESTAMP_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", curr_time);
  return std::string(buffer);
}

SystemCallTraceReplayLogger::~SystemCallTraceReplayLogger() {
  // Close the log file
  logger_file_.close();
}
