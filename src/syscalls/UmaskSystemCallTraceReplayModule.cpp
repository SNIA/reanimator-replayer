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
 * UmaskSystemCallTraceReplayModule header file
 *
 * Read UmaskSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "UmaskSystemCallTraceReplayModule.hpp"

UmaskSystemCallTraceReplayModule::UmaskSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "umask";
}

void UmaskSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("mode(", mode, ")");
}

void UmaskSystemCallTraceReplayModule::processRow() {
  // Replay umask by updating umask table.
  SystemCallTraceReplayModule::replayer_resources_manager_.set_umask(
      executingPidVal, mode);
  // Always succeed since umask always succeeds.
  replayed_ret_val_ = 0;
}

void UmaskSystemCallTraceReplayModule::prepareRow() {
  mode = mode_value_.val();
  SystemCallTraceReplayModule::prepareRow();
}
