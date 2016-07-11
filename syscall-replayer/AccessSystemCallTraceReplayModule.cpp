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
 * This file implements all the functions in the
 * AccessSystemCallTraceReplayModule header file
 *
 * Read AccessSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "AccessSystemCallTraceReplayModule.hpp"

AccessSystemCallTraceReplayModule::
AccessSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname"),
  mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "access";
}

void AccessSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "pathname(" << given_pathname_.val() << "), ";
  std::cout << "mode(" << mode_value_.val() << ")";
}

void AccessSystemCallTraceReplayModule::processRow() {
  const char *pathname = (char *)given_pathname_.val();
  int mode_value = mode_value_.val();

  // Replay the access system call
  replayed_ret_val_ = access(pathname, mode_value);
}
