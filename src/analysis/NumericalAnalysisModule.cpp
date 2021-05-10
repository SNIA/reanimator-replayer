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

// TODO-NEWPEOPLE: When finding the tail quantiles, we need to keep a cache of
// some of the elements. We should allow the user to somehow specify the cache
// size.
FieldStruct::FieldStruct() 
    : acc{tag::tail<left>::cache_size = 100000,
          tag::tail<right>::cache_size = 100000} {
    // nothing to do
}

void NumericalAnalysisModule::considerSyscall(const SystemCallTraceReplayModule& module) {
    std::string sys_call_name = module.sys_call_name();

    auto& syscallStruct = syscalls_[sys_call_name];

    // TODO-NEWPEOPLE: This is an inefficient approach. Currently,
    // syscallStruct.fields is a map whose keys are strings and whose values are
    // FieldStructs. However, this means that whenever we want to access one of
    // those field structs, we must perform a map lookup. A better solution
    // would be to make syscallStruct.fields an array of FieldStructs. We can
    // index into the array using enums (which are just named constants). For
    // example, TIME_CALLED might be 0, so we could access that field using
    // syscallStruct.fields[TIME_CALLED]. We already have a numbered list of
    // fields in enum form. Look in the file strace2ds-enums.h in
    // reanimator-library.
    syscallStruct.fields["time_called"].acc(module.time_called());
    syscallStruct.fields["time_returned"].acc(module.time_returned());
    syscallStruct.fields["time_recorded"].acc(module.time_recorded());
    syscallStruct.fields["executing_pid"].acc(module.executing_pid());
    syscallStruct.fields["errno_number"].acc(module.errno_number());
    syscallStruct.fields["return_value"].acc(module.return_value());

    // TODO-NEWPEOPLE: It's not particularly useful to record time_called and
    // time_returned. We'd much rather record elapsed time (which would be their
    // difference). It would also be nice to record the deltas between syscalls
    // of the same type (time between writes, say) and deltas between immediate
    // successive calls (time between syscall 1 and 2). Assuming the analysis is
    // single-threaded, you could keep static variables for the previously seen
    // system calls' time_called.

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
    // TODO-NEWPEOPLE: The user should be able to specify which quantile they
    // care about. Good defaults are 0.95, 0.99, and 0.999
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
        // TODO-NEWPEOPLE: There is an unfortunate bug here where boost crashes
        // if the accumulator `acc` only contains 1 element.
        if (syscall.second.count > 1)
            out << "," << quantile(acc, quantile_probability = 0.25);
        out << std::endl;

    }
    out << std::endl;
    return out;
}
