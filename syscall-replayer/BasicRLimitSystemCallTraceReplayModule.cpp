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
 * GetRLimitSystemCallTraceReplayModule header file
 *
 * Read BaicRLimitSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "BasicRLimitSystemCallTraceReplayModule.hpp"

BasicRLimitSystemCallTraceReplayModule::BasicRLimitSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      resource_value_(series, "resource_value", Field::flag_nullable),
      resource_soft_limit_(series, "resource_soft_limit"),
      resource_hard_limit_(series, "resource_hard_limit") {}

void BasicRLimitSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("resource value(", resource_value_.val(), "), ",
                            "resource soft limit(", resource_soft_limit_.val(),
                            "), ", "resource hard limit(",
                            resource_hard_limit_.val(), ")");
}

int BasicRLimitSystemCallTraceReplayModule::getResource() {
  int returnVal = reinterpret_cast<int>(resource_value_.val());
  return returnVal;
}

rlim_t BasicRLimitSystemCallTraceReplayModule::getSoftLimit() {
  rlim_t returnVal = reinterpret_cast<long>(resource_soft_limit_.val());
  return returnVal;
}

rlim_t BasicRLimitSystemCallTraceReplayModule::getHardLimit() {
  rlim_t returnVal = reinterpret_cast<long>(resource_hard_limit_.val());
  return returnVal;
}
