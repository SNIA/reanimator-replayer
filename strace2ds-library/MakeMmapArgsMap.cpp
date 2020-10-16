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

void DataSeriesOutputModule::makeMmapArgsMap(void **args_map, long *args,
                                             void **v_args) {
  // Initialize all non-nullable boolean fields to False.
  initArgsMap(args_map, "mmap");

  args_map[SYSCALL_FIELD_START_ADDRESS] = &args[0];
  args_map[SYSCALL_FIELD_LENGTH] = &args[1];

  args_map[SYSCALL_FIELD_PROTECTION_VALUE] = &args[2];
  // Set individual mmap protection bits
  u_int prot_flags = processMmapProtectionArgs(args_map, args[2]);
  if (prot_flags != 0) {
    std::cerr << "Mmap: These protection flags are not processed/unknown->0x";
    std::cerr << std::hex << prot_flags << std::dec << std::endl;
  }

  args_map[SYSCALL_FIELD_FLAGS_VALUE] = &args[3];
  // Set individual mmap flag bits
  u_int flag = processMmapFlags(args_map, args[3]);
  if (flag != 0) {
    std::cerr << "Mmap: These flag are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }

  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[4];
  args_map[SYSCALL_FIELD_OFFSET] = &args[5];
}

u_int DataSeriesOutputModule::processMmapProtectionArgs(void **args_map,
                                                        u_int mmap_prot_flags) {
  /*
   * Process each individual mmap protection bit that has been set
   * in the argument mmap_prot_flags.
   */
  // set exec protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_EXEC,
                             SYSCALL_FIELD_PROTECTION_EXEC);
  // set read protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_READ,
                             SYSCALL_FIELD_PROTECTION_READ);
  // set write protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_WRITE,
                             SYSCALL_FIELD_PROTECTION_WRITE);
  // set none protection flag
  process_Flag_and_Mode_Args(args_map, mmap_prot_flags, PROT_NONE,
                             SYSCALL_FIELD_PROTECTION_NONE);

  /*
   * Return remaining mmap protection flags so that caller can
   * warn of unknown flags if the mmap_prot_flags is not set
   * as zero.
   */
  return mmap_prot_flags;
}

u_int DataSeriesOutputModule::processMmapFlags(void **args_map,
                                               u_int mmap_flags) {
  /*
   * Process each individual mmap flag bit that has been set
   * in the argument mmap_flags.
   */
  // set mmap fixed flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_FIXED,
                             SYSCALL_FIELD_FLAG_FIXED);
  // set mmap shared flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_SHARED,
                             SYSCALL_FIELD_FLAG_SHARED);
  // set mmap private flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_PRIVATE,
                             SYSCALL_FIELD_FLAG_PRIVATE);
  // set mmap 32bit flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_32BIT,
                             SYSCALL_FIELD_FLAG_32BIT);
  // set mmap anonymous flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_ANONYMOUS,
                             SYSCALL_FIELD_FLAG_ANONYMOUS);
  // set mmap denywrite flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_DENYWRITE,
                             SYSCALL_FIELD_FLAG_DENYWRITE);
  // set mmap executable flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_EXECUTABLE,
                             SYSCALL_FIELD_FLAG_EXECUTABLE);
  // set mmap file flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_FILE,
                             SYSCALL_FIELD_FLAG_FILE);
  // set mmap grows_down flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_GROWSDOWN,
                             SYSCALL_FIELD_FLAG_GROWS_DOWN);
  // set mmap huge TLB flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_HUGETLB,
                             SYSCALL_FIELD_FLAG_HUGE_TLB);
  // set mmap locked flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_LOCKED,
                             SYSCALL_FIELD_FLAG_LOCKED);
  // set mmap non-blocking flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_NONBLOCK,
                             SYSCALL_FIELD_FLAG_NON_BLOCK);
  // set mmap no reserve flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_NORESERVE,
                             SYSCALL_FIELD_FLAG_NO_RESERVE);
  // set mmap populate flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_POPULATE,
                             SYSCALL_FIELD_FLAG_POPULATE);
  // set mmap stack flag
  process_Flag_and_Mode_Args(args_map, mmap_flags, MAP_STACK,
                             SYSCALL_FIELD_FLAG_STACK);

  /*
   * Return remaining mmap flags so that caller can
   * warn of unknown flags if the mmap_flags is not set
   * as zero.
   */
  return mmap_flags;
}
