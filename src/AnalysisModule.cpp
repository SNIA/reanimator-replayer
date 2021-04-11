#include "AnalysisModule.hpp"
#include <iostream>
#include <boost/format.hpp>


AnalysisStruct::AnalysisStruct()
    : min_time_elapsed(-1), max_time_elapsed(0), average_time_elapsed(0),
      rows(0) {
    // nothing to do
}

uint64_t updateAverage(uint64_t value, uint64_t old_average, uint64_t rows) {
    return rows * (old_average / (rows + 1)) + (value / (rows + 1));
}

void AnalysisModule::considerTimeElapsed(uint64_t time_elapsed, std::string syscall_name) {
    AnalysisStruct& syscall_specific_analysis_struct = analysisMap_[syscall_name];
    considerTimeElapsedInternal(time_elapsed, syscall_specific_analysis_struct);
    considerTimeElapsedInternal(time_elapsed, global_metrics_);
}

void AnalysisModule::considerTimeElapsedInternal(uint64_t time_elapsed, AnalysisStruct& analysis) {
    analysis.min_time_elapsed = std::min(analysis.min_time_elapsed, time_elapsed);
    analysis.max_time_elapsed = std::max(analysis.max_time_elapsed, time_elapsed);
    analysis.average_time_elapsed = 
        updateAverage(time_elapsed, analysis.average_time_elapsed,
                      analysis.rows);
    ++analysis.rows;
}

void AnalysisModule::printGlobalMetrics() {
    std::cout << boost::format("Global Metrics:\n");
    std::cout << boost::format("Min Syscall Time Elapsed (ns): %u\n") % global_metrics_.min_time_elapsed;
    std::cout << boost::format("Max Syscall Time Elapsed (ns): %u\n") % global_metrics_.max_time_elapsed;
    std::cout << boost::format("Average Syscall Time Elapsed (ns): %u\n\n") % global_metrics_.average_time_elapsed;
}

void AnalysisModule::printPerSyscallMetrics() {
    for (const auto &am : analysisMap_) {
        AnalysisStruct a = am.second;
        std::cout << boost::format("Metrics For %s:\n") % am.first;
        std::cout << boost::format("Min Syscall Time Elapsed (ns): %u\n") % a.min_time_elapsed;
        std::cout << boost::format("Max Syscall Time Elapsed (ns): %u\n") % a.max_time_elapsed;
        std::cout << boost::format("Average Syscall Time Elapsed (ns): %u\n\n") % a.average_time_elapsed;
    }
}
