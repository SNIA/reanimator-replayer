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
 * This file implements all functions in the DurationAnalysisModule header file.
 *
 * Read DurationAnalysisModule.hpp for more information about this class.
 */

#include "DurationAnalysisModule.hpp"
#include <iostream>
#include <boost/format.hpp>

DurationAnalysisStruct::DurationAnalysisStruct()
    : min_time_elapsed(-1), max_time_elapsed(0), average_time_elapsed(0),
      rows(0) {
    // nothing to do
}

void DurationAnalysisModule::considerSyscall(const SystemCallTraceReplayModule& module) {
    std::string sys_call_name = module.sys_call_name();

    if (module.isTimeable()) {
        uint64_t time_elapsed = module.time_returned() - module.time_called();
        auto& syscallAnalysisStruct = analysisMap_[sys_call_name];
        considerTimeElapsed(time_elapsed, syscallAnalysisStruct);
        considerTimeElapsed(time_elapsed, global_metrics_);
    } else {
        std::cerr << boost::format("<Duration Analysis> Untracked syscall %s is not timeable\n")
                     % sys_call_name;
    }
}

void DurationAnalysisModule::considerTimeElapsed(uint64_t time_elapsed, DurationAnalysisStruct& analysis) {
    analysis.min_time_elapsed = std::min(analysis.min_time_elapsed, time_elapsed);
    analysis.max_time_elapsed = std::max(analysis.max_time_elapsed, time_elapsed);
    analysis.average_time_elapsed =
        rollingAverage(time_elapsed, analysis.average_time_elapsed,
                      analysis.rows);
    ++analysis.rows;
}

uint64_t DurationAnalysisModule::rollingAverage(uint64_t value, uint64_t old_average, uint64_t rows) const {
    return ((rows * old_average) + value) / (rows + 1);
}

std::ostream& DurationAnalysisModule::printMetrics(std::ostream& out) const {
    out << boost::format("=== Duration Analysis ===\n");
    printPerSyscallMetrics(out);
    out << boost::format("\n");
    printGlobalMetrics(out);
    return out;
}

std::ostream& DurationAnalysisModule::printGlobalMetrics(std::ostream& out) const {
    out << boost::format("Global Metrics:\n");
    out << boost::format("Min Syscall Time Elapsed (ns): %u\n") % global_metrics_.min_time_elapsed;
    out << boost::format("Max Syscall Time Elapsed (ns): %u\n") % global_metrics_.max_time_elapsed;
    out << boost::format("Average Syscall Time Elapsed (ns): %u\n\n") % global_metrics_.average_time_elapsed;
    return out;
}

std::ostream& DurationAnalysisModule::printPerSyscallMetrics(std::ostream& out) const {
    out << boost::format("%-10s\t%s\t%s\t%s\n") % "System Call" % "Min (ns)" % "Max (ns)" % "Average (ns)";
    for (const auto &am : analysisMap_) {
        auto& a = am.second;
        out << boost::format("%-10s\t%-10u\t%-10u\t%u\n") % am.first % a.min_time_elapsed % a.max_time_elapsed % a.average_time_elapsed;
    }
    return out;
}

