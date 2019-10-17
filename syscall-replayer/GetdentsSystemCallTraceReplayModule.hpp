/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions for implementing getdents
 * system call.
 *
 * GetdentsSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying getdents system call.
 *
 * USAGE
 * A main program could initialize this object with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef GETDENTS_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define GETDENTS_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <dirent.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cerrno>
#include <string>
#include "SystemCallTraceReplayModule.hpp"

class GetdentsSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  bool verify_;
  // Getdents System Call Trace Fields in Dataseries file
  Int32Field descriptor_;
  Variable32Field dirent_buffer_;
  Int32Field count_;

  unsigned int traced_fd;
  struct dirent *dirent_buffer_val;
  unsigned int count_val;
  int64_t return_val;

  /**
   * Print getdents sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and then replay a getdents system call with those arguments.
   */
  void processRow() override;

  void printDirents(struct dirent *direntBuf) {
    for (int pos = 0; pos < return_val;) {
      auto direntRec =
          (struct dirent *)(reinterpret_cast<char *>(direntBuf) + pos);
      auto inode = direntRec->d_ino;
      auto type = *((reinterpret_cast<char *>(direntBuf)) + pos +
                    direntRec->d_reclen - 1);
      auto typeStr = getDirentTypeStr(type);
      auto offset = direntRec->d_off;
      auto recordLen = direntRec->d_reclen;
      auto direntName = direntRec->d_name;
      syscall_logger_->log_warn("inode: ", inode, " type: ", typeStr,
                                " off: ", offset, " record length: ", recordLen,
                                " name: ", direntName - 1);
      pos += recordLen;
    }
  }

  std::string getDirentTypeStr(char type) {
    switch (type) {
      case DT_REG: {
        return "regular";
      }
      case DT_DIR: {
        return "directory";
      }
      case DT_FIFO: {
        return "fifo";
      }
      case DT_SOCK: {
        return "socket";
      }
      case DT_LNK: {
        return "symlink";
      }
      case DT_BLK: {
        return "block dev";
      }
      case DT_CHR: {
        return "char dev";
      }
      default:
        return "unknown";
    }
  }

  bool compareResults(struct dirent *realExecution, int64_t sizeOfRealExecution,
                      struct dirent *replayed, int64_t sizeOfReplay);

 public:
  GetdentsSystemCallTraceReplayModule(DataSeriesModule &source,
                                      bool verbose_flag, bool verify_flag,
                                      int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr = new GetdentsSystemCallTraceReplayModule(
        source, verbose_, verify_, warn_level_);
    movePtr->setMove(traced_fd, dirent_buffer_val, count_val, return_val);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(int fd, struct dirent *direntBuffer, int count,
               int return_val_) {
    traced_fd = fd;
    dirent_buffer_val = direntBuffer;
    count_val = count;
    return_val = return_val_;
  }
  void prepareRow() override;
};
#endif /* READ_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
