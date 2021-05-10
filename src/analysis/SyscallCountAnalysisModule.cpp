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
 * This file implements all functions in the SyscallCountAnalysisModule header
 * file.
 *
 * Read SyscallCountAnalysisModule.hpp for more information about this class.
 */

#include "SyscallCountAnalysisModule.hpp"
#include <iostream>
#include <boost/format.hpp>

void SyscallCountAnalysisModule::considerSyscall(const SystemCallTraceReplayModule& module) {
    std::string sys_call_name = module.sys_call_name();
    analysisMap_[sys_call_name] += 1;
    totalCount_ += 1;
}

std::ostream& SyscallCountAnalysisModule::printMetrics(std::ostream& out) const {
    out << boost::format("=== Syscall Counts ===\n");
    printGlobalMetrics(out);
    return out;
}

std::ostream& SyscallCountAnalysisModule::printGlobalMetrics(std::ostream& out) const {
    out << boost::format("%-10s\t%s\t%s\n") % "System Call" % "Count" % "Proportion";
    for (const auto &am : analysisMap_) {
        double proportion = ((double) am.second / (double) totalCount_) * 100;
        out << boost::format("%-10s\t%d\t%f %%\n") % am.first % am.second % proportion;
    }

    return out;
}
