/*
 * Copyright (c) 2016-2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
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

void DataSeriesOutputModule::makeUmaskArgsMap(void **args_map,
					      long *args,
					      void **v_args) {
  initArgsMap(args_map, "umask");
  int mode_offset = 0;
  mode_t mode = processMode(args_map, args, mode_offset);
  if (mode != 0) {
    std::cerr << "Umask: These modes are not processed/unknown->0";
    std::cerr << std::oct << mode << std::dec << std::endl;
  }
}
