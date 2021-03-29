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
 * This header file provides members and functions for implementing read
 * system call.
 *
 * ReadSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying read system call.
 *
 * USAGE
 * A main program could initialize this object with a dataseries file
 * and call execute() function until all extents are processed.
 * 
 * TODO: Update this header
 */
#ifndef READ_SYSTEM_CALL_ANALYSIS_MODULE_HPP
#define READ_SYSTEM_CALL_ANALYSIS_MODULE_HPP

#include <unistd.h>
#include <DataSeries/DataSeriesModule.hpp>
#include <DataSeries/RowAnalysisModule.hpp>
#include <DataSeries/TypeIndexModule.hpp>

class ReadSystemCallAnalysisModule : public RowAnalysisModule {
  public:
    ReadSystemCallAnalysisModule() = delete;
    ReadSystemCallAnalysisModule(DataSeriesModule &source);
    virtual ~ReadSystemCallAnalysisModule();

    //void prepareRow() override;
    void processRow() override;
    void printResult() override;
  private:
    uint64_t rows_;
    uint64_t min_time_elapsed_;
    uint64_t max_time_elapsed_;
    double average_time_elapsed_;
    Int64Field time_called_;
    Int64Field time_returned_;
    std::string syscall_name_;
};

#endif /* READ_SYSTEM_CALL_ANALYSIS_MODULE_HPP */
