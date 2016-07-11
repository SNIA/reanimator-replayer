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
 * This header file provides members and functions for implementing mknod
 * system call.
 *
 * MknodSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying mknod system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 *
 */
#ifndef MKNOD_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define MKNOD_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DS_FILE_TYPE_REG 0
#define DS_FILE_TYPE_CHR 1
#define DS_FILE_TYPE_BLK 2
#define DS_FILE_TYPE_FIFO 3
#define DS_FILE_TYPE_SOCK 4

class MknodSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  /* Mknod System Call Trace Fields in Dataseries file */
  Variable32Field given_pathname_;
  Int32Field mode_value_;
  ByteField type_;
  Int32Field dev_;

  /*
   * Print mknod sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will prepare things before replaying any
   * mknod system call. Right now it displays a starting
   * message.
   */
  void prepareForProcessing();

  /*
   * This function will gather arguments in the trace file
   * and replay a mknod system call with those arguments.
   */
  void processRow();

  /*
   * This function will do things that have be done
   * after finishing replaying all mknod system calls.
   * Now, it only displays an ending message.
   */
  void completeProcessing();

public:
  MknodSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   int warn_level_flag);

};

#endif /* MKNOD_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
