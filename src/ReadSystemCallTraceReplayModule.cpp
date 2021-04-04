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
 * This file implements all the functions in the
 * ReadSystemCallTraceReplayModule header file
 *
 * Read ReadSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "ReadSystemCallTraceReplayModule.hpp"
#include "VirtualAddressSpace.hpp"

ReadSystemCallTraceReplayModule::ReadSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      verify_(verify_flag),
      descriptor_(series, "descriptor"),
      data_read_(series, "data_read", Field::flag_nullable),
      bytes_requested_(series, "bytes_requested") {
  sys_call_name_ = "read";
}

void ReadSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, traced_fd);
  syscall_logger_->log_info("traced fd(", traced_fd, "), ", "replayed fd(",
                            replayed_fd, "), ",
                            //    "data read(", dataReadBuf, "), ",
                            "bytes requested(", nbytes, ")");
}

void ReadSystemCallTraceReplayModule::verifyRow() {
  if (verify_) {
    if (dataReadBuf == nullptr) {
      if (replayed_ret_val_ == return_value()) {
        syscall_logger_->log_info(
            "Verification of data because of not captured in read success.");
      }
    }

    // Verify read data and data in the trace file are same
    if (dataReadBuf != NULL && replayed_ret_val_ >= 0 &&
        memcmp(dataReadBuf, buffer, replayed_ret_val_) != 0) {
      // Data aren't same
      syscall_logger_->log_info("Verification of data in read failed. retval:",
                                replayed_ret_val_);
      if (verbose_mode()) {
        syscall_logger_->log_info(
            "time called:",
            boost::format(DEC_PRECISION) % Tfrac_to_sec(time_called()),
            " Captured read data is different from replayed read data");
        syscall_logger_->log_info("Captured read data: ", dataReadBuf, ", ",
                                  "Replayed read data: ", std::string(buffer));
        if (abort_mode()) {
          abort();
        }
      }
    } else {
      if (verbose_mode()) {
        syscall_logger_->log_info(
            "Verification of data comparison in read success.");
      }
    }
    delete[] dataReadBuf;
  }

  delete[] buffer;
}

void ReadSystemCallTraceReplayModule::processRow() {
  auto replayed_fd =
      replayer_resources_manager_.get_fd(executingPidVal, traced_fd);
  if (replayed_fd == SYSCALL_SIMULATED) {
    /*
     * FD for the read call originated from an AF_UNIX socket().
     * The system call will not be replayed.
     * Original return value will be returned.
     */
    return;
  }
  // Replay read system call as normal.
  replayed_ret_val_ = read(replayed_fd, buffer, nbytes);

  verifyRow();
}

void ReadSystemCallTraceReplayModule::analyzeRow() {
  std::cout << boost::format("nbytes is %d\n"
                             "time elapsed: %d\n") % nbytes
                                                 % (timeReturnedVal - timeCalledVal);
}

void ReadSystemCallTraceReplayModule::prepareRow() {
  traced_fd = descriptor_.val();
  nbytes = bytes_requested_.val();
  replayed_ret_val_ = return_value_.val();
  buffer = new char[nbytes];

  if (verify_) {
    if (replayed_ret_val_ > 0) {
      auto dataBuf = reinterpret_cast<const char *>(data_read_.val());
      dataReadBuf = new char[replayed_ret_val_];
      std::memcpy(dataReadBuf, dataBuf, replayed_ret_val_);
    } else {
      dataReadBuf = nullptr;
    }
  }
  SystemCallTraceReplayModule::prepareRow();
}

PReadSystemCallTraceReplayModule::PReadSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : ReadSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                      warn_level_flag),
      offset_(series, "offset") {
  sys_call_name_ = "pread";
}

void PReadSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, traced_fd);
  syscall_logger_->log_info(
      "traced fd(", traced_fd, "), ", "replayed fd(", replayed_fd, "), ",
      // "data read(", data_read_.val(),
      "), ", "bytes requested(", nbytes, "), ", "offset(", off, ")");
}

void PReadSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, traced_fd);

  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the PRead system call originated from a socket().
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value();
    return;
  }

  replayed_ret_val_ = pread(fd, buffer, nbytes, off);

  verifyRow();
}

void PReadSystemCallTraceReplayModule::prepareRow() {
  off = offset_.val();
  ReadSystemCallTraceReplayModule::prepareRow();
}

MmapPReadSystemCallTraceReplayModule::MmapPReadSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : PReadSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                       warn_level_flag),
      address_(series, "address") {
  sys_call_name_ = "mmappread";
}

void MmapPReadSystemCallTraceReplayModule::prepareRow() {
  ptr = address_.val();
  PReadSystemCallTraceReplayModule::prepareRow();
}

void MmapPReadSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, traced_fd);
  syscall_logger_->log_info(
      "address(", boost::format("0x%02x") % ptr, ") ", "traced fd(", traced_fd,
      "), ", "replayed fd(", replayed_fd, "), ",
      // "data read(", data_read_.val(),
      "), ", "bytes requested(", nbytes, "), ", "offset(", off, ")");
}

void MmapPReadSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, traced_fd);

  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the PRead system call originated from a socket().
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value();
    return;
  }

  auto areas = VM_manager::getInstance()->get_VM_area(pid)->find_VM_node(
      reinterpret_cast<void *>(ptr), 8);

  for (auto vnode : *areas) {
    if (vnode == NULL) continue;
    auto offset_ptr =
        ptr - reinterpret_cast<uint64_t>(vnode->traced_start_address);
    syscall_logger_->log_info(
        "trace start addr (",
        boost::format("0x%02x") % vnode->traced_start_address, ") ptr (",
        boost::format("0x%02x") % ptr, ") offset_ptr(", offset_ptr, ")");
    auto replayed_ptr =
        reinterpret_cast<uint64_t>(vnode->replayed_start_address) + offset_ptr;
    auto read_ptr = reinterpret_cast<uint64_t *>(replayed_ptr);
    __attribute__((unused)) auto test = *read_ptr;
    auto verify_ptr =
        reinterpret_cast<uint64_t>(vnode->replayed_start_address) + off;
    std::memcpy(buffer, reinterpret_cast<void *>(verify_ptr),
                replayed_ret_val_);
    verifyRow();
  }
}
