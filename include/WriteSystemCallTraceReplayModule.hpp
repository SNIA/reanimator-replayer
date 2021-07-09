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
 * This header file provides members and functions for implementing write
 * system call.
 *
 * WriteSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying write system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef WRITE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define WRITE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

class WriteSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  bool verify_;
  std::string pattern_data_;
  // Write System Call Trace Fields in Dataseries file
  Int32Field descriptor_;
  Variable32Field data_written_;
  Int64Field bytes_requested_;
  char *data_buffer;
  size_t nbytes;
  int traced_fd;

  /**
   * Print write sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * or create our own arguments (for example, pattern),
   * then replay an write system call with those arguments.
   */
  void processRow() override;

 public:
  WriteSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   bool verify_flag, int warn_level_flag,
                                   std::string pattern_data);
  SystemCallTraceReplayModule *move() override {
    auto movePtr = new WriteSystemCallTraceReplayModule(
        source, verbose_, verify_, warn_level_, pattern_data_);
    movePtr->setMove(data_buffer, nbytes, traced_fd);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(char *buf, int byte, int fd) {
    data_buffer = buf;
    nbytes = byte;
    traced_fd = fd;
  }
  void prepareRow() override;
  uint64_t count_arg() const;
};

class PWriteSystemCallTraceReplayModule
    : public WriteSystemCallTraceReplayModule {
 protected:
  // PWrite System Call Trace Fields in Dataseries file
  Int64Field offset_;
  off_t off;
  /**
   * Print pwrite sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and then replay an pwrite system call with those arguments.
   */
  void processRow() override;

 public:
  PWriteSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                    bool verify_flag, int warn_level_flag,
                                    std::string pattern_data);
  SystemCallTraceReplayModule *move() override {
    auto movePtr = new PWriteSystemCallTraceReplayModule(
        source, verbose_, verify_, warn_level_, pattern_data_);
    movePtr->setMove(data_buffer, nbytes, traced_fd, off);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  inline void setMove(char *buf, int byte, int fd, off_t offset) {
    WriteSystemCallTraceReplayModule::setMove(buf, byte, fd);
    off = offset;
  }
  void prepareRow() override;
};

class MmapPWriteSystemCallTraceReplayModule
    : public PWriteSystemCallTraceReplayModule {
 public:
  MmapPWriteSystemCallTraceReplayModule(DataSeriesModule &source,
                                        bool verbose_flag, bool verify_flag,
                                        int warn_level_flag,
                                        std::string pattern_data);
};
#endif /* WRITE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
