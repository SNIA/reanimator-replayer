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
  std::cout << "given_pathname(" << given_pathname_.val() << "), ";
  std::cout << "access_time(" << Tfrac_to_sec(access_time_.val()) << "), ";
  std::cout << "mod_time(" << Tfrac_to_sec(mod_time_.val()) << ")";
}

void UtimeSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Utime System Call Replayer starts to replay...-----"
	    << std::endl;
}

void UtimeSystemCallTraceReplayModule::processRow() {
  // Get replaying file given_pathname.
  struct utimbuf utimebuf;
  const char *pathname = (char *) given_pathname_.val();
  utimebuf.actime = Tfrac_to_sec(access_time_.val());
  utimebuf.modtime = Tfrac_to_sec(mod_time_.val());

  // Replay the utime system call.
  if ((access_time_.val() == 0) && (mod_time_.val() == 0))
    // If access_time and mod_time are both 0, then assume the utimbuf is NULL.
    replayed_ret_val_ = utime(pathname, NULL);
  else
    replayed_ret_val_ = utime(pathname, &utimebuf);
}

void UtimeSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Utime System Call Replayer finished replaying...-----"
	    << std::endl;
}

UtimesSystemCallTraceReplayModule::
UtimesSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  int warn_level_flag):
  UtimeSystemCallTraceReplayModule(source, verbose_flag, warn_level_flag) {
  sys_call_name_ = "utimes";
}

void UtimesSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Utimes System Call Replayer starts to replay...-----"
	    << std::endl;
}

void UtimesSystemCallTraceReplayModule::processRow() {
  // Get replaying file given_pathname and make timeval array.
  struct timeval tv[2];
  const char *pathname = (char *) given_pathname_.val();

  /*
   * If the recorded error was EINVAL (invalid argument), don't initialize
   * the timeval array, in order to replicate the recorded behavior.
   */
  if (errno_number_.val() != 22) {
    struct timeval tv_access_time = Tfrac_to_timeval(access_time_.val());
    struct timeval tv_mod_time = Tfrac_to_timeval(mod_time_.val());
    tv[0] = tv_access_time;
    tv[1] = tv_mod_time;
  }

  // Replay the utimes system call.
  if ((access_time_.val() == 0) && (mod_time_.val() == 0))
    /*
     * If access_time and mod_time are both 0, then assume the timeval
     * array is NULL.
     */
    replayed_ret_val_ = utimes(pathname, NULL);
  else
    replayed_ret_val_ = utimes(pathname, tv);
}

void UtimesSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Utimes System Call Replayer finished replaying...-----"
	    << std::endl;
}
