/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
 */

#include <queue>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <DataSeries/PrefetchBufferModule.hpp>
#include <DataSeries/TypeIndexModule.hpp>

#include "Accept4SystemCallTraceReplayModule.hpp"
#include "AcceptSystemCallTraceReplayModule.hpp"
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
#include "Dup3SystemCallTraceReplayModule.hpp"
#include "DupSystemCallTraceReplayModule.hpp"
#include "EPollCreateSystemCallTraceReplayModule.hpp"
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
#include "SocketPairSystemCallTraceReplayModule.hpp"
#include "SocketSystemCallTraceReplayModule.hpp"
#include "SymlinkSystemCallTraceReplayModule.hpp"
#include "TruncateSystemCallTraceReplayModule.hpp"
#include "UmaskSystemCallTraceReplayModule.hpp"
#include "UnlinkSystemCallTraceReplayModule.hpp"
#include "UtimeSystemCallTraceReplayModule.hpp"
#include "VForkSystemCallTraceReplayModule.hpp"
#include "WriteSystemCallTraceReplayModule.hpp"
#include "WritevSystemCallTraceReplayModule.hpp"

#include "AnalysisModule.hpp"
#include "DurationAnalysisModule.hpp"
#include "ThreadWiseDurationAnalysisModule.hpp"
#include "SyscallCountAnalysisModule.hpp"
#include "NumericalAnalysisModule.hpp"

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
