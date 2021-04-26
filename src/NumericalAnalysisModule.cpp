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

FieldStruct::FieldStruct()
    : values{} {
    // nothing to do
}

void NumericalAnalysisModule::considerSyscall(const SystemCallTraceReplayModule& module) {
    std::string sys_call_name = module.sys_call_name();

    auto& syscallStruct = syscalls_[sys_call_name];

    syscallStruct.fields["time_called"].values.push_back(module.time_called());
    syscallStruct.fields["time_returned"].values.push_back(module.time_returned());
    syscallStruct.fields["time_recorded"].values.push_back(module.time_recorded());
    syscallStruct.fields["executing_pid"].values.push_back(module.executing_pid());
    syscallStruct.fields["errno_number"].values.push_back(module.errno_number());
    syscallStruct.fields["return_value"].values.push_back(module.return_value());
}

std::ostream& NumericalAnalysisModule::printMetrics(std::ostream& out) const {
    out << boost::format("=== Numerical Analysis ===\n");

    for (auto& syscall : syscalls_) {
        out << boost::format("%s:\n") % syscall.first;
        for (auto& field : syscall.second.fields) {
            out << boost::format("\t%s: ") % field.first;
            for (auto& val : field.second.values) {
                out << val << " ";
            }
            out << std::endl;
        }
        out << std::endl;
    }

    return out;
}
