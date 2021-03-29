#include "ReadSystemCallAnalysisModule.hpp"

using namespace std;
using boost::format;

ReadSystemCallAnalysisModule::ReadSystemCallAnalysisModule(
    DataSeriesModule &source)
    : RowAnalysisModule(source),
      rows_(0),
      min_time_elapsed_(numeric_limits<uint64_t>::max()),
      max_time_elapsed_(numeric_limits<uint64_t>::min()),
      average_time_elapsed_(0.0),
      time_called_(series, "time_called", Field::flag_nullable),
      time_returned_(series, "time_returned", Field::flag_nullable),
      syscall_name_("read")
    {
        // nothing to do
    }

ReadSystemCallAnalysisModule::~ReadSystemCallAnalysisModule() {
    // nothing to do
}

void ReadSystemCallAnalysisModule::processRow() {
    uint64_t time_called = time_called_.val();
    uint64_t time_returned = time_returned_.val();
    uint64_t time_elapsed = time_returned - time_called;

    min_time_elapsed_ = min(min_time_elapsed_, time_elapsed);
    max_time_elapsed_ = max(max_time_elapsed_, time_elapsed);
    average_time_elapsed_ = rows_ * (average_time_elapsed_ / (rows_ + 1)) +
        (time_elapsed / (rows_ + 1));
    ++rows_;

    /*
    cout << format("time_called: %u\n"
        "time_returned: %u\n"
        "time_elapsed: %u\n"
        "avg time: %d\n")   % time_called
                            % time_returned
                            % time_elapsed
                            % average_time_elapsed_;
    */
}

void ReadSystemCallAnalysisModule::printResult() {
    cout << format("Total %s syscalls: %u\n") % syscall_name_ % rows_;
    cout << format("\tMinimum elapsed time: %u\n"
        "\tMaximum elapsed time: %u\n"
        "\tAverage elapsed time: %d\n")   % min_time_elapsed_
                                        % max_time_elapsed_
                                        % average_time_elapsed_;
}
