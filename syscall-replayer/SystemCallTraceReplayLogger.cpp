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
 * This file implements all the functions in the SystemCallTraceReplayLogger
 * header file.
 *
 * Read SystemCallTraceReplayLogger.hpp for more information about this class.
 */

#include "SystemCallTraceReplayLogger.hpp"

SystemCallTraceReplayLogger* SystemCallTraceReplayLogger::logger = NULL;
std::ofstream SystemCallTraceReplayLogger::logger_file_;

SystemCallTraceReplayLogger::SystemCallTraceReplayLogger() {
}

void SystemCallTraceReplayLogger::initialize(std::string log_filename) {
  if (logger == NULL) {
    // Create a new instance of logger class.
    logger = new SystemCallTraceReplayLogger();
    // Open the logger file
    logger_file_.open(log_filename.c_str());
    if (logger_file_.is_open() < 0) {
      std::cerr << "Unable to open log file" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

void SystemCallTraceReplayLogger::close_stream() {
  // Close the log file
  logger_file_.close();
}

SystemCallTraceReplayLogger *SystemCallTraceReplayLogger::getInstance() {
  return logger;
}

char *SystemCallTraceReplayLogger::print_time() {
  static char buffer[TIMESTAMP_BUFFER_SIZE];
  time_t rawtime;
  struct tm *curr_time;

  time(&rawtime);
  curr_time = localtime(&rawtime);

  strftime(buffer, TIMESTAMP_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", curr_time);
  return buffer;
}
