#include "AnalysisModule.hpp"


AnalysisStruct::AnalysisStruct() 
    : min_time_elapsed(0), max_time_elapsed(0), average_time_elapsed(0),
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
    analysis.max_time_elapsed = std::min(analysis.max_time_elapsed, time_elapsed);
    analysis.average_time_elapsed = 
        updateAverage(time_elapsed, analysis.average_time_elapsed,
                      analysis.rows);
    ++analysis.rows;
}


/*
AnalysisModule::examineFriend(SystemCallTraceReplayModule& module) {
    std::cout << boost::format("Syscall %s has timeReturned %u\n")
        % module.sys_call_name_ % module.timeReturnedVal;
}
*/
