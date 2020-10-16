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
 * This file implements all the functions in the DataSeriesOutputModule.hpp
 * header file.
 *
 * Read the DataSeriesOutputModule.hpp file for more information about this
 * class.
 */

#include "DataSeriesOutputModule.hpp"

void DataSeriesOutputModule::makeUtimensatArgsMap(void **args_map, long *args,
                                                  void **v_args) {
  static uint64_t access_time_Tfrac;
  static uint64_t mod_time_Tfrac;
  initArgsMap(args_map, "utimensat");

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  if (args[0] == AT_FDCWD) {
    args_map[SYSCALL_FIELD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY] = &true_;
    if (v_args[0] == NULL) {
      std::cerr
          << "Utimensat: Pathname is set as NULL and dirfd is set as AT_FDCWD!!"
          << std::endl;
    }
  } else {
    if (v_args[0] == NULL && (args[3] & AT_SYMLINK_NOFOLLOW)) {
      std::cerr << "Utimensat: Pathname is set as NULL and dirfd is not set as "
                   "AT_FDCWD and flag contains AT_SYMLINK_NOFOLLOW"
                << std::endl;
    }
  }

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_GIVEN_PATHNAME] = &v_args[0];
  }

  // If the timespec array is not NULL, set the corresponding values in the map
  if (v_args[1] != NULL) {
    struct timespec *ts = (struct timespec *)v_args[1];

    // Check for the special values UTIME_NOW and UTIME_OMIT
    if ((ts[0].tv_nsec == UTIME_NOW) || (ts[1].tv_nsec == UTIME_NOW))
      args_map[SYSCALL_FIELD_UTIME_NOW] = &true_;
    if ((ts[0].tv_nsec == UTIME_OMIT) || (ts[1].tv_nsec == UTIME_OMIT))
      args_map[SYSCALL_FIELD_UTIME_OMIT] = &true_;

    // Convert timespec arguments to Tfracs (uint64_t)
    access_time_Tfrac = timespec_to_Tfrac(ts[0]);
    mod_time_Tfrac = timespec_to_Tfrac(ts[1]);

    args_map[SYSCALL_FIELD_ACCESS_TIME] = &access_time_Tfrac;
    args_map[SYSCALL_FIELD_MOD_TIME] = &mod_time_Tfrac;
  }

  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[3];
  u_int flag = args[3];
  process_Flag_and_Mode_Args(args_map, flag, AT_SYMLINK_NOFOLLOW,
                             SYSCALL_FIELD_FLAG_SYMLINK_NOFOLLOW);
  if (flag != 0) {
    std::cerr << "Utimensat: These flags are not processed/unknown->"
              << std::hex << flag << std::dec << std::endl;
  }
}
