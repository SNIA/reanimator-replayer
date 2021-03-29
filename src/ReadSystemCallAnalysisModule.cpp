#include "ReadSystemCallAnalysisModule.hpp"

ReadSystemCallAnalysisModule::ReadSystemCallAnalysisModule(
    DataSeriesModule &source)
    : RowAnalysisModule(source),
      rows_(0),
      time_called_(series, "time_called", Field::flag_nullable),
      time_returned_(series, "time_returned", Field::flag_nullable)
    {
        // nothing to do
    }

ReadSystemCallAnalysisModule::~ReadSystemCallAnalysisModule() {
    // nothing to do
}

void ReadSystemCallAnalysisModule::processRow() {
    std::cout << "process row " << rows_ << std::endl;
    ++rows_;
}

void ReadSystemCallAnalysisModule::printResult() {
    std::cout << "Total rows: " << rows_ << std::endl;
}
