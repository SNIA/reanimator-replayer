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
 * Read NumericalAnalysisModule.hpp for more information about this class.
 */

#include "NumericalAnalysisModule.hpp"
#include <iostream>
#include <unordered_map>
#include <boost/format.hpp>

// *** allow user to specify cache_size
FieldStruct::FieldStruct() 
    : acc{tag::tail<left>::cache_size = 100000,
          tag::tail<right>::cache_size = 100000} {
    // nothing to do
}

void NumericalAnalysisModule::considerSyscall(const SystemCallTraceReplayModule& module) {
    std::string sys_call_name = module.sys_call_name();

    auto& syscallStruct = syscalls_[sys_call_name];

    // make the "keys" constant enums and change syscallStruct.fields into an array
    // can use strace2ds-enums.h from the reanimator-library

    // do statistics on durations (elapsed time per syscall, deltas between
    // syscalls of the same type, deltas between immediate successive calls

    // ^ could make static variables (assuming the analysis is single-threaded)
    // to represent the previous call's time_called and time_returned

    // this could be new-people-work TODO-NEWPEOPLE
    syscallStruct.fields["time_called"].acc(module.time_called());
    syscallStruct.fields["time_returned"].acc(module.time_returned());
    syscallStruct.fields["time_recorded"].acc(module.time_recorded());
    syscallStruct.fields["executing_pid"].acc(module.executing_pid());
    syscallStruct.fields["errno_number"].acc(module.errno_number());
    syscallStruct.fields["return_value"].acc(module.return_value());

    ++syscallStruct.count;
}

std::ostream& NumericalAnalysisModule::printMetrics(std::ostream& out) const {
    out << boost::format("=== Numerical Analysis ===\n");

    for (auto& syscall : syscalls_) {
        printSyscallMetrics(out, syscall);
    }

    return out;
}

std::ostream& NumericalAnalysisModule::printSyscallMetrics(
    std::ostream& out,
    const std::pair<std::string, SyscallStruct>& syscall) const {

    std::string syscallName = syscall.first;
    auto& syscallStruct = syscall.second;
    out << boost::format("%s: (%d)\n") % syscallName % syscallStruct.count;
    // *** It would be nice to have the user specify which quantile they want
    // Good defaults are 0.95, 0.99, 0.999
    out << "Name,Mean,Median,Variance,Max,0.95 Quantile" << std::endl;
    for (auto& field : syscallStruct.fields) {
        std::string fieldName = field.first;
        auto& acc = field.second.acc;

        out << fieldName;

        // Print out statistics
        out << "," << mean(acc);
        out << "," << median(acc);
        out << "," << variance(acc);
        out << "," << max(acc);
        //if (syscall.second.count > 1)
            out << "," << quantile(acc, quantile_probability = 0.25);
        out << std::endl;

    }
    out << std::endl;
    return out;
}
