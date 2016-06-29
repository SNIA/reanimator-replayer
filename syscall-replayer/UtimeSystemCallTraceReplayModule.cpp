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
 * UtimeSystemCallTraceReplayModule header file
 *
 * Read UtimeSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "UtimeSystemCallTraceReplayModule.hpp"

UtimeSystemCallTraceReplayModule::
UtimeSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname"),
  access_time_(series, "access_time"),
  mod_time_(series, "mod_time") {
  sys_call_name_ = "utime";
}

void UtimeSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "given_pathname(" << given_pathname_.val() << ")," << std::endl;
  std::cout << "access_time(" << Tfrac_to_sec(access_time_.val())
	    << ")," << std::endl;
  std::cout << "mod_time(" << Tfrac_to_sec(mod_time_.val()) << ")";
}

void UtimeSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Utime system call replayer starts to replay...-----"
	    << std::endl;
}

void UtimeSystemCallTraceReplayModule::processRow() {
  // Get replaying file given_pathname.
  struct utimbuf utimebuf;
  const char *pathname = (char *)given_pathname_.val();
  utimebuf.actime = Tfrac_to_sec(access_time_.val());
  utimebuf.modtime = Tfrac_to_sec(mod_time_.val());

  // Replay the utime system call.
  replayed_ret_val_ = utime(pathname, &utimebuf);
}

void UtimeSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Utime system call replayer finished replaying...-----"
	    << std::endl;
}
