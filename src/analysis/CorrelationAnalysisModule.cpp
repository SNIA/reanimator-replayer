/* 
 * Copywrite (c) 2017      Darshan Godhia
 * Copywrite (c) 2016-2019 Erez Zadok
 * Copywrite (c) 2011      Jack Ma
 * Copywrite (c) 2019      Jatin Sood
 * Copywrite (c) 2017-2018 Kevin Sun
 * Copywrite (c) 2015-2017 Leixiang Wu
 * Copywrite (c) 2020      Lukas Velikov
 * Copywrite (c) 2017-2018 Maryia Maskaliova
 * Copywrite (c) 2017      Mayur Jadh
 * Copywrite (c) 2016      Ming Chen
 * Copywrite (c) 2017      Nehil Shah
 * Copywrite (c) 2016      Nina Brown
 * Copywrite (c) 2021      Ree Croom
 * Copywrite (c) 2011-2012 Santhosh Kumar
 * Copywrite (c) 2015-2016 Shubhi Rani
 * Copywrite (c) 2018      Siddesh Shinde
 * Copywrite (c) 2014      Sonam Mandal
 * Copywrite (c) 2012      Sudhir Kasanovesi
 * Copywrite (c) 2020      Thomas Fleming
 * Copywrite (c) 2018-2020 Ibrahim Umit Akgun
 * Copywrite (c) 2011-2012 Vasily Tarasov
 * Copywrite (c) 2019      Yinuo Zhang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all functions in hte CorrelationAnalysisModule header
 * file.
 *
 * Read CorrelationAnalysisModule.hpp for more information about this class.
 */

#include "CorrelationAnalysisModule.hpp"
#include <iostream>

void CorrelationAnalysisModule::considerSyscall(
        const SystemCallTraceReplayModule& module) {
    std::string sys_call_name = module.sys_call_name();
    
    auto& syscallData = syscalls_[sys_call_name];
    syscallData.fields["time_elapsed"] = 
        module.time_returned() - module.time_called();
    syscallData.fields["return_value"] = module.return_value();

    ++syscallData.count;
}

std::ostream& CorrelationAnalysisModule::printMetrics(std::ostream& out) const {
    out << boost::format("=== Correlation Analysis ===\n");

    for (auto& syscall : syscalls_) {
        printSyscallMetrics(out, syscall);
    }

    return out;
}

std::ostream& CorrelationAnalysisModule::printSyscallMetrics(
        std::ostream& out,
        const std::pair<std::string, SyscallData>& syscall) const {

    std::string syscallName = syscall.first;
    auto& syscallData = syscall.second;

    out << boost::format("%s: (%d)\n") % syscallName % syscallData.count;

    out << std::endl;
    return out;
}
