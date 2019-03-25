#include <queue>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <DataSeries/PrefetchBufferModule.hpp>
#include <DataSeries/TypeIndexModule.hpp>

#include "AccessSystemCallTraceReplayModule.hpp"
#include "BasicStatSystemCallTraceReplayModule.hpp"
#include "BasicStatfsSystemCallTraceReplayModule.hpp"
#include "ChdirSystemCallTraceReplayModule.hpp"
#include "ChmodSystemCallTraceReplayModule.hpp"
#include "ChownSystemCallTraceReplayModule.hpp"
#include "ChrootSystemCallTraceReplayModule.hpp"
#include "CloneSystemCallTraceReplayModule.hpp"
#include "CloseSystemCallTraceReplayModule.hpp"
#include "CreatSystemCallTraceReplayModule.hpp"
#include "Dup2SystemCallTraceReplayModule.hpp"
#include "DupSystemCallTraceReplayModule.hpp"
#include "ExecveSystemCallTraceReplayModule.hpp"
#include "ExitSystemCallTraceReplayModule.hpp"
#include "FChdirSystemCallTraceReplayModule.hpp"
#include "FChmodSystemCallTraceReplayModule.hpp"
#include "FTruncateSystemCallTraceReplayModule.hpp"
#include "FallocateSystemCallTraceReplayModule.hpp"
#include "FcntlSystemCallTraceReplayModule.hpp"
#include "FdatasyncSystemCallTraceReplayModule.hpp"
#include "FsyncSystemCallTraceReplayModule.hpp"
#include "GetdentsSystemCallTraceReplayModule.hpp"
#include "IoctlSystemCallTraceReplayModule.hpp"
#include "LSeekSystemCallTraceReplayModule.hpp"
#include "LinkSystemCallTraceReplayModule.hpp"
#include "MkdirSystemCallTraceReplayModule.hpp"
#include "MknodSystemCallTraceReplayModule.hpp"
#include "MmapSystemCallTraceReplayModule.hpp"
#include "MunmapSystemCallTraceReplayModule.hpp"
#include "OpenSystemCallTraceReplayModule.hpp"
#include "PipeSystemCallTraceReplayModule.hpp"
#include "ReadSystemCallTraceReplayModule.hpp"
#include "ReadaheadSystemCallTraceReplayModule.hpp"
#include "ReadlinkSystemCallTraceReplayModule.hpp"
#include "ReadvSystemCallTraceReplayModule.hpp"
#include "RenameSystemCallTraceReplayModule.hpp"
#include "RmdirSystemCallTraceReplayModule.hpp"
#include "SetxattrSystemCallTraceReplayModule.hpp"
#include "SocketSystemCallTraceReplayModule.hpp"
#include "SymlinkSystemCallTraceReplayModule.hpp"
#include "TruncateSystemCallTraceReplayModule.hpp"
#include "UmaskSystemCallTraceReplayModule.hpp"
#include "UnlinkSystemCallTraceReplayModule.hpp"
#include "UtimeSystemCallTraceReplayModule.hpp"
#include "VForkSystemCallTraceReplayModule.hpp"
#include "WriteSystemCallTraceReplayModule.hpp"
#include "WritevSystemCallTraceReplayModule.hpp"

// Define the static replayer resources manager in SystemCallTraceReplayModule
ReplayerResourcesManager
    SystemCallTraceReplayModule::replayer_resources_manager_;
// Define the input file stream random_file_ in SystemCallTraceReplayModule
std::ifstream SystemCallTraceReplayModule::random_file_;
// Define the object of logger class in SystemCallTraceReplayModule
SystemCallTraceReplayLogger *SystemCallTraceReplayModule::syscall_logger_;

// Assert that only version 1 is allowed (at this point)
static const unsigned int supported_major_version = 1;
static const unsigned int supported_minor_version = 0;

/*
 * This number is the number of system calls that are need to replayed
 * in order to invoke next fd scanning done to verify the state
 * of resource manager. Ex: 10 means that replayer will scan all fds
 * for every 10 system calls that are replayed.
 */
#define SCAN_FD_FREQUENCY 100000
