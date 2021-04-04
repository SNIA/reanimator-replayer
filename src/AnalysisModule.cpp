#include "AnalysisModule.hpp"

AnalysisModule::considerTimeElapsed(uint64_t time_elapsed, std::string syscall_name) {
    // todo
}

AnalysisModule::examineFriend(SystemCallTraceReplayModule& module) {
    std::cout << boost::format("Syscall %s has timeReturned %u\n")
        % module.sys_call_name_ % module.timeReturnedVal;
}
