/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
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
  return (int)(resource_value_.val());
}

rlim_t BasicRLimitSystemCallTraceReplayModule::getSoftLimit(){
    return (rlim_t)(resource_soft_limit_.val())}

rlim_t BasicRLimitSystemCallTraceReplayModule::getHardLimit() {
  return (rlim_t)(resource_hard_limit_.val())
}
