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
 * This header file provides members and functions for implementing read
 * system call.
 *
 * ReadSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying read system call.
 *
 * USAGE
 * A main program could initialize this object with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef READ_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define READ_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <unistd.h>
#include "SystemCallTraceReplayModule.hpp"

class ReadSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  bool verify_;
  // Read System Call Trace Fields in Dataseries file
  Int32Field descriptor_;
  Variable32Field data_read_;
  Int64Field bytes_requested_;
  int traced_fd;
  int nbytes;
  char *buffer;
  char *dataReadBuf;

  /**
   * Print read sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and then replay an read system call with those arguments.
   */
  void processRow() override;

  void verifyRow();

 public:
  ReadSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                  bool verify_flag, int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr = new ReadSystemCallTraceReplayModule(source, verbose_,
                                                       verify_, warn_level_);
    movePtr->setMove(buffer, nbytes, traced_fd, dataReadBuf);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  inline void setMove(char *buf, int byte, int fd, char *verifyBuf) {
    buffer = buf;
    nbytes = byte;
    traced_fd = fd;
    dataReadBuf = verifyBuf;
  }
  void prepareRow() override;
};

class PReadSystemCallTraceReplayModule
    : public ReadSystemCallTraceReplayModule {
 protected:
  // PRead System Call Trace Fields in Dataseries file
  Int64Field offset_;
  off_t off;

  /**
   * Print pread sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and then replay an pread system call with those arguments.
   */
  void processRow() override;

 public:
  PReadSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   bool verify_flag, int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr = new PReadSystemCallTraceReplayModule(source, verbose_,
                                                        verify_, warn_level_);
    movePtr->setMove(buffer, nbytes, traced_fd, off, dataReadBuf);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  inline void setMove(char *buf, int byte, int fd, off_t offset,
                      char *verifyBuf) {
    ReadSystemCallTraceReplayModule::setMove(buf, byte, fd, verifyBuf);
    off = offset;
  }
  void prepareRow() override;
};

class MmapPReadSystemCallTraceReplayModule
    : public PReadSystemCallTraceReplayModule {
 protected:
  Int64Field address_;
  uint64_t ptr;

  /**
   * Print pread sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and then replay an pread system call with those arguments.
   */
  void processRow() override;

 public:
  MmapPReadSystemCallTraceReplayModule(DataSeriesModule &source,
                                       bool verbose_flag, bool verify_flag,
                                       int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr = new MmapPReadSystemCallTraceReplayModule(
        source, verbose_, verify_, warn_level_);
    movePtr->setMove(buffer, nbytes, traced_fd, off, dataReadBuf, ptr);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  inline void setMove(char *buf, int byte, int fd, off_t offset,
                      char *verifyBuf, uint64_t pointer) {
    PReadSystemCallTraceReplayModule::setMove(buf, byte, fd, offset, verifyBuf);
    ptr = pointer;
  }
  void prepareRow() override;
};

#endif /* READ_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
