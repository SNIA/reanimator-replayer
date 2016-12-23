#include <queue>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <DataSeries/PrefetchBufferModule.hpp>
#include <DataSeries/TypeIndexModule.hpp>

#include "OpenSystemCallTraceReplayModule.hpp"
#include "CloseSystemCallTraceReplayModule.hpp"
#include "ReadSystemCallTraceReplayModule.hpp"
#include "WriteSystemCallTraceReplayModule.hpp"
#include "LSeekSystemCallTraceReplayModule.hpp"
#include "AccessSystemCallTraceReplayModule.hpp"
#include "ChdirSystemCallTraceReplayModule.hpp"
#include "TruncateSystemCallTraceReplayModule.hpp"
#include "CreatSystemCallTraceReplayModule.hpp"
#include "LinkSystemCallTraceReplayModule.hpp"
#include "UnlinkSystemCallTraceReplayModule.hpp"
#include "SymlinkSystemCallTraceReplayModule.hpp"
#include "RmdirSystemCallTraceReplayModule.hpp"
#include "MkdirSystemCallTraceReplayModule.hpp"
#include "BasicStatSystemCallTraceReplayModule.hpp"
#include "BasicStatfsSystemCallTraceReplayModule.hpp"
#include "ReadlinkSystemCallTraceReplayModule.hpp"
#include "UtimeSystemCallTraceReplayModule.hpp"
#include "ChmodSystemCallTraceReplayModule.hpp"
#include "FChmodSystemCallTraceReplayModule.hpp"
#include "ChownSystemCallTraceReplayModule.hpp"
#include "ReadvSystemCallTraceReplayModule.hpp"
#include "WritevSystemCallTraceReplayModule.hpp"
#include "RenameSystemCallTraceReplayModule.hpp"
#include "FsyncSystemCallTraceReplayModule.hpp"
#include "MknodSystemCallTraceReplayModule.hpp"
#include "PipeSystemCallTraceReplayModule.hpp"
#include "DupSystemCallTraceReplayModule.hpp"
#include "Dup2SystemCallTraceReplayModule.hpp"
#include "FcntlSystemCallTraceReplayModule.hpp"
#include "ExitSystemCallTraceReplayModule.hpp"
#include "ExecveSystemCallTraceReplayModule.hpp"
#include "MmapSystemCallTraceReplayModule.hpp"
#include "MunmapSystemCallTraceReplayModule.hpp"
#include "GetdentsSystemCallTraceReplayModule.hpp"
#include "IoctlSystemCallTraceReplayModule.hpp"
#include "CloneSystemCallTraceReplayModule.hpp"
#include "VForkSystemCallTraceReplayModule.hpp"
#include "UmaskSystemCallTraceReplayModule.hpp"

// Define the static fd_map_ in SystemCallTraceReplayModule
std::map<int, int> SystemCallTraceReplayModule::fd_map_;
// Define the static replayer resources manager in SystemCallTraceReplayModule
ReplayerResourcesManager SystemCallTraceReplayModule::replayer_resources_manager_;
// Define the input file stream random_file_ in SystemCallTraceReplayModule
std::ifstream SystemCallTraceReplayModule::random_file_;
// Define the object of logger class in SystemCallTraceReplayModule
SystemCallTraceReplayLogger *SystemCallTraceReplayModule::syscall_logger_;

// Assert that only version 1 is allowed (at this point)
static const unsigned int supported_major_version = 1;
static const unsigned int supported_minor_version = 0;
