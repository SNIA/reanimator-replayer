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

SyscallCsv::SyscallCsv(const SystemCallTraceReplayModule& module) {
    const std::string& name = module.sys_call_name();
    add_field(name);
    add_field(module.return_value());
    if (module.isTimeable()) {
        add_field(module.time_returned() - module.time_called());
    } else {
        add_field("");
    }
    add_field(module.time_called());
    add_field(module.executing_pid());
    if (name == "read") {
        const ReadSystemCallTraceReplayModule &read_module =
            (ReadSystemCallTraceReplayModule&)module;
        add_field(read_module.bytes_requested());
    }
    while (fields < MAX_FIELDS) {
        add_field("");
    }
    row_ += "\n";
}

void SyscallCsv::add_field(uint64_t field) {
    add_field(std::to_string(field));
}

void SyscallCsv::add_field(uint32_t field) {
    add_field(std::to_string(field));
}

void SyscallCsv::add_field(int64_t field) {
    add_field(std::to_string(field));
}

void SyscallCsv::add_field(const std::string& field) {
    fields++;
    if (row_.empty()) {
        row_ = field;
    } else {
        row_ += ',' + field;
    }
}

CorrelationAnalysisModule::CorrelationAnalysisModule() {
    csvfile = std::ofstream(CSVFILENAME);
}

void CorrelationAnalysisModule::considerSyscall(
        const SystemCallTraceReplayModule& module) {
    SyscallCsv row(module);
    csvfile << row.to_string();
}

CorrelationAnalysisModule::~CorrelationAnalysisModule() {
    csvfile.close();
}

std::ostream& CorrelationAnalysisModule::printMetrics(std::ostream& out) const {
    out << "=== Duration Analysis ===\n";
    out << "Wrote to " << CSVFILENAME << " for further analysis\n";
    return out;
}
