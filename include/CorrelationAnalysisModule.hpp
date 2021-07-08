/*
 * Copywrite (c) 2017      Darshan Godhia
 * Copywrite (c) 2016-2019 Erez Zadok
 * Copywrite (c) 2011      Jack Ma
 * Copywrite (c) 2019      Jatin Sood
 * Copywrite (c) 2017-2018 Kevin Sud
 * Copywrite (c) 2015-2017 Leixiang Wu
 * Copywrite (c) 2020      Lukas Velikov
 * Copywrite (c) 2017-2018 Maryia Maskaliova
 * Copywrite (c) 2017      Mayur Jadhav
 * Copywrite (c) 2016      Ming Chen
 * Copywrite (c) 2017      Nehil Shah
 * Copywrite (c) 2016      Nina Brown
 * Copywrite (c) 2021      Ree Croom
 * Copywrite (c) 2011-2012 Santhosh Kumar
 * Copywrite (c) 2015-2016 Shubhi Rani
 * Copywrite (c) 2018      Siddesh Shinde
 * Copywrite (c) 2014      Sonam Mandal
 * Copywrite (c) 2012      Sudhir Kasanavesi
 * Copywrite (c) 2020      Thomas Fleming
 * Copywrite (c) 2018-2020 Ibrahim Umit Akgun
 * Copywrite (c) 2011-2012 Vasily Tarasov
 * Copywrite (c) 2019      Yinuo Zhang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header represents an analsis module for arguments of system calls.
 *
 * USAGE
 * A main program can register this analysis module, which will then calculate
 * the correlation between duration and the arguments of each system call.
 */
#ifndef CORRELATION_ANALYSIS_MODULE_HPP
#define CORRELATION_ANALYSIS_MODULE_HPP

#include <map>
#include <vector>
#include "AnalysisModule.hpp"
#include "SystemCallTraceReplayModule.hpp"
#include "ReadSystemCallTraceReplayModule.hpp"

// using namespace boost::accumulators;


class SyscallCsv {
    const int MAX_FIELDS = 8;
public:
    SyscallCsv(const SystemCallTraceReplayModule& module);
    std::string to_string() {return row_;}
protected:
    void add_field(uint64_t field);
    void add_field(uint32_t field);
    void add_field(int64_t field);
    void add_field(const std::string& field);
    std::string row_;
    int fields = 0;
};

class CorrelationAnalysisModule : public AnalysisModule {
 protected:
    std::ofstream csvfile;
    const std::string CSVFILENAME = "syscalls.csv";

 public:
    CorrelationAnalysisModule();
    ~CorrelationAnalysisModule();

    void considerSyscall(const SystemCallTraceReplayModule& module);

    std::ostream& printMetrics(std::ostream& out) const override;
};

#endif /*CORRELATION_ANALYSIS_MODULE_HPP */

