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
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SystemCallTraceReplayer is a program that is designed for user to
 * replay system calls in DataSeries. It currently supports replaying
 * open, close, read, write, and lseek and has various options to modify the
 * behavior of this program.
 *
 * USAGE
 * ./system-call-replayer <-vwp> <input files>
 * ./system-call-replayer --analysis <input files>
 */

#include "SystemCallTraceReplayer.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include "tbb/atomic.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_priority_queue.h"
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_vector.h"
#include "tbb/task_group.h"

// #define PROFILE_ENABLE

#ifdef PROFILE_ENABLE
#define PROFILE_START(start_id)                                \
  std::chrono::high_resolution_clock::time_point t##start_id = \
      std::chrono::high_resolution_clock::now();

#define PROFILE_END(start_id, end_id, acc)                                 \
  std::chrono::high_resolution_clock::time_point t##end_id =               \
      std::chrono::high_resolution_clock::now();                           \
  acc += std::chrono::duration_cast<std::chrono::nanoseconds>(t##end_id -  \
                                                              t##start_id) \
             .count();

#define PROFILE_SAMPLE(id) t##id = std::chrono::high_resolution_clock::now();

#define PROFILE_PRINT(str, acc) std::cerr << str << acc / 1000000 << "\n";
#else
#define PROFILE_START(start_id)
#define PROFILE_END(start_id, end_id, acc)
#define PROFILE_SAMPLE(id)
#define PROFILE_PRINT(str, acc)
#endif

/**
 * min heap uses this function to sort elements in the tree.
 * The sorting key is unique id.
 */
struct CompareByUniqueID {
  bool operator()(SystemCallTraceReplayModule *m1,
                  SystemCallTraceReplayModule *m2) const {
    return (m1->unique_id() >= m2->unique_id());
  }
};

typedef tbb::concurrent_hash_map<int64_t, SystemCallTraceReplayModule *>
    RunningSyscallTable;
typedef tbb::concurrent_priority_queue<SystemCallTraceReplayModule *,
                                       CompareByUniqueID>
    ExecutionHeap;

tbb::atomic<uint64_t> *numberOfSyscalls;
tbb::atomic<bool> *finishedModules;

/*
   * Define a min heap that stores each module. The heap is ordered
   * by unique_id field.
   */
std::unordered_map<int64_t, ExecutionHeap> executionHeaps;
auto initExecutionHeap = [](int64_t tid) {
  if (executionHeaps.find(tid) == executionHeaps.end()) {
    executionHeaps.emplace(std::make_pair(tid, ExecutionHeap(10000)));
  }
};
tbb::concurrent_vector<std::thread> threads;

int64_t mainThreadID = 0;

std::unordered_map<std::string, SystemCallTraceReplayModule *> syscallMapLast;
tbb::concurrent_queue<SystemCallTraceReplayModule *> allocationQueue;

/**
 * Stores all analysis modules that should run on the given input files.
 */
tbb::concurrent_vector<AnalysisModule *> analysisModules;

int64_t fileReading_Batch_file = 0;
int64_t fileReading_Batch_push = 0;
int64_t fileReading_Batch_map = 0;
int64_t fileReading_Batch_move = 0;
bool isFirstBatch = true;
bool analysis = false;

int64_t replayerIdx = 0;
tbb::atomic<uint64_t> nThreads = 1;
tbb::atomic<uint64_t> lastExecutedSyscallID = 1;

std::mutex debug_mutex;

RunningSyscallTable currentExecutions;
std::function<void(int64_t, SystemCallTraceReplayModule *)> setRunning = [](
    int64_t tid, SystemCallTraceReplayModule *syscall) {
  RunningSyscallTable::accessor acc;
  currentExecutions.insert(acc, tid);
  acc->second = syscall;
};

#ifdef PROFILE_ENABLE
int64_t duration = 0;
int64_t fileReading = 0;
int64_t gettingRecord = 0;
int64_t loop = 0;
int64_t destroy = 0;
int64_t executionSpinning = 0;
#endif

/**
 * This function declares a group of options that will
 * be allowed for the replayer, store options in a
 * variable map and return it.
 * @param argc: number of arguments on the command line.
 * @param argv: arguments on the command line.
 * @return: vm - a variable map that contains all the options
 *               found on the command line.
 */
boost::program_options::variables_map get_options(int argc, char *argv[]) {
  namespace po = boost::program_options;
  // Declare a group of options that will be allowed only on command line
  po::options_description generic("Generic options");
  generic.add_options()("version,V", "print version of system call replayer")(
      "help,h", "produce help message");

  /*
   * Declare a group of options that will be
   * allowed both on command line and in
   * config file
   */
  po::options_description config("Configuration");
  config.add_options()("verbose,v", "system calls replay in verbose mode")(
      "verify,f",
      "verifies that the data being written/read is "
      "exactly what was used originally")("warn,w", po::value<int>(),
                                          "system call replays in warn mode")(
      "pattern,p", po::value<std::string>(),
      "write repeated pattern data in write system call")(
      "logger,l", po::value<std::string>(),
      "write the replayer logs in specified filename")(
      "analysis,a",
      "perform simple analysis on trace file");

  /*
   * Hidden options, will be allowed both on command line and
   * in config file, but will not be shown to the user.
   */
  po::options_description hidden("Hidden options");
  hidden.add_options()("input-files,I", po::value<std::vector<std::string>>(),
                       "input files");

  po::options_description cmdline_options;
  cmdline_options.add(generic).add(config).add(hidden);

  /*
   * po::options_description config_file_options;
   * config_file_options.add(config).add(hidden);
   */
  po::options_description visible("Allowed options");
  visible.add(generic).add(config);

  po::positional_options_description p;
  p.add("input-files", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
                .options(cmdline_options)
                .positional(p)
                .run(),
            vm);
  po::notify(vm);

  if (vm.count("help") != 0u) {
    std::cerr << visible << std::endl;
    exit(EXIT_SUCCESS);
  }

  return vm;
}

/**
 * process_options function calls get_options() to get
 * a variable map that contains all the options. Then this
 * function uses the variable map to define options that
 * are passed as function arguments.
 *
 * @param argc: number of arguments on the command line.
 * @param argv: arguments on the command line.
 * @param verbose: whether the replayer replays in verbose mode
 * @param verify: whether to verify data read that is from replaying
 *                is same as data in read sys call in the trace file.
 * @param warn_level: replaying warning level
 * @param input_files: DataSeries files that contain system call
 *                     traces
 */
void process_options(int argc, char *argv[], bool &verbose,
                     bool &verify, int &warn_level, std::string &pattern_data,
                     std::string &log_filename,
                     std::vector<std::string> &input_files) {
  boost::program_options::variables_map options_vm = get_options(argc, argv);

  if (options_vm.count("version") != 0u) {
    std::cerr << "sysreplayer version 1.0" << std::endl;
    exit(EXIT_SUCCESS);
  }

  if (options_vm.count("warn") != 0u) {
    warn_level = options_vm["warn"].as<int>();
    if (warn_level > ABORT_MODE) {
      std::cerr << "Wrong value for warn option" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  if (options_vm.count("analysis") != 0u) {
    analysis = true;
  }

  if (options_vm.count("verify") != 0u) {
    verify = true;
  }

  if (options_vm.count("verbose") != 0u) {
    verbose = true;
  }

  if (options_vm.count("pattern") != 0u) {
    pattern_data = options_vm["pattern"].as<std::string>();
  }

  if (options_vm.count("logger") != 0u) {
    log_filename = options_vm["logger"].as<std::string>();
  }

  /*
   * In case of verify, verbose or warn mode, user must specify the
   * log filename in order to save replayer log messages.
   */
  if (verify || verbose || warn_level > DEFAULT_MODE) {
    if (log_filename.empty()) {
      std::cout << "Use '-l' option to provide log filename"
                << " to save replayer logs" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  if (options_vm.count("input-files") != 0u) {
    input_files = options_vm["input-files"].as<std::vector<std::string>>();
  } else {
    std::cout << "No dataseries input files.\n";
    exit(EXIT_FAILURE);
  }
}

/**
 * Creates a prefetch buffer module for each system call.
 * Each prefetch buffer module is going to be used to create
 * system call module.
 * Note that the order of prefetch buffer modules in the return
 * vector matters. Meaning that each system call module
 * needs to use its corresponding prefetch buffer module.
 * Ex: OpenSystemCallTraceReplayModule needs to be instantiated
 * by using the first prefetch buffer module, OpenAtSystemCallTraceReplayModule
 * needs to be instantiated by using the second,
 *CloseSystemCallTraceReplayModule
 * uses third, so on and so forth.
 ******Remember to modify here when supporting a new system call.********
 * Return a list of prefetch buffer modules
 */
std::vector<PrefetchBufferModule *> create_prefetch_buffer_modules(
    std::vector<std::string> &input_files) {
  // This is the prefix extent type of all system calls.
  const std::string kExtentTypePrefix = "IOTTAFSL::Trace::Syscall::";

  // This vector contains the list of system calls that need replaying modules.
  std::vector<std::string> system_calls;
  system_calls.push_back("open");
  system_calls.push_back("openat");
  system_calls.push_back("close");
  system_calls.push_back("read");
  system_calls.push_back("write");
  system_calls.push_back("lseek");
  system_calls.push_back("pread");
  system_calls.push_back("mmappread");
  system_calls.push_back("access");
  system_calls.push_back("faccessat");
  system_calls.push_back("chdir");
  system_calls.push_back("fchdir");
  system_calls.push_back("chroot");
  system_calls.push_back("truncate");
  system_calls.push_back("creat");
  system_calls.push_back("link");
  system_calls.push_back("linkat");
  system_calls.push_back("unlink");
  system_calls.push_back("unlinkat");
  system_calls.push_back("symlink");
  system_calls.push_back("rmdir");
  system_calls.push_back("mkdir");
  system_calls.push_back("mkdirat");
  system_calls.push_back("stat");
  system_calls.push_back("statfs");
  system_calls.push_back("fstatfs");
  system_calls.push_back("pwrite");
  system_calls.push_back("mmappwrite");
  system_calls.push_back("readlink");
  system_calls.push_back("utime");
  system_calls.push_back("chmod");
  system_calls.push_back("fchmod");
  system_calls.push_back("fchmodat");
  system_calls.push_back("chown");
  system_calls.push_back("readv");
  system_calls.push_back("writev");
  system_calls.push_back("lstat");
  system_calls.push_back("fstat");
  system_calls.push_back("fstatat");
  system_calls.push_back("utimes");
  system_calls.push_back("utimensat");
  system_calls.push_back("rename");
  system_calls.push_back("fsync");
  system_calls.push_back("fdatasync");
  system_calls.push_back("fallocate");
  system_calls.push_back("readahead");
  system_calls.push_back("mknod");
  system_calls.push_back("pipe");
  system_calls.push_back("dup");
  system_calls.push_back("dup2");
  system_calls.push_back("dup3");
  system_calls.push_back("fcntl");
  system_calls.push_back("exit");
  system_calls.push_back("execve");
  system_calls.push_back("mmap");
  system_calls.push_back("munmap");
  system_calls.push_back("getdents");
  system_calls.push_back("ioctl");
  system_calls.push_back("clone");
  system_calls.push_back("vfork");
  system_calls.push_back("umask");
  system_calls.push_back("setxattr");
  system_calls.push_back("lsetxattr");
  system_calls.push_back("fsetxattr");
  system_calls.push_back("ftruncate");
  system_calls.push_back("socket");
  system_calls.push_back("socketpair");
  system_calls.push_back("epoll_create");
  system_calls.push_back("accept");
  system_calls.push_back("accept4");

  std::vector<TypeIndexModule *> type_index_modules;

  for (auto &system_call : system_calls) {
    TypeIndexModule *type_index_module =
        new TypeIndexModule(kExtentTypePrefix + system_call);
    type_index_modules.push_back(type_index_module);
  }

  // Specify to read dataseries input files
  for (auto &input_file : input_files) {
    for (auto &type_index_module : type_index_modules) {
      type_index_module->addSource(input_file);
    }
  }

  std::vector<PrefetchBufferModule *> prefetch_buffer_modules;
  for (auto &type_index_module : type_index_modules) {
    // Parallel decompress and replay, 8MiB buffer
    auto module = new PrefetchBufferModule(*type_index_module, 8 * 1024 * 1024);
    prefetch_buffer_modules.push_back(module);
  }

  return prefetch_buffer_modules;
}

/**
 * Creates a replaying module for each system call.
 *****Remember to modify here to create a replaying module when supporting a new
 *system call.*********
 * IMPORTANT: each entry in prefetch_buffer_modules corresponds to
 * its own module.
 */
std::vector<SystemCallTraceReplayModule *>
create_system_call_trace_replay_modules(
    std::vector<PrefetchBufferModule *> prefetch_buffer_modules, bool verbose,
    bool verify, int warn_level, std::string &pattern_data) {
  int module_index = 0;
  auto open_module = new OpenSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto openat_module = new OpenatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto close_module = new CloseSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto read_module = new ReadSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto write_module = new WriteSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level,
      pattern_data);
  auto lseek_module = new LSeekSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto pread_module = new PReadSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto mmappread_module = new MmapPReadSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto access_module = new AccessSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto faccessat_module = new FAccessatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto chdir_module = new ChdirSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto fchdir_module = new FChdirSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto chroot_module = new ChrootSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto truncate_module = new TruncateSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto creat_module = new CreatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto link_module = new LinkSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto linkat_module = new LinkatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto unlink_module = new UnlinkSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto unlinkat_module = new UnlinkatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto symlink_module = new SymlinkSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto rmdir_module = new RmdirSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto mkdir_module = new MkdirSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto mkdirat_module = new MkdiratSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto stat_module = new StatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto statfs_module = new StatfsSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto fstatfs_module = new FStatfsSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto pwrite_module = new PWriteSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level,
      pattern_data);
  auto mmappwrite_module = new MmapPWriteSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level,
      pattern_data);
  auto readlink_module = new ReadlinkSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto utime_module = new UtimeSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto chmod_module = new ChmodSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto fchmod_module = new FChmodSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto fchmodat_module = new FChmodatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto chown_module = new ChownSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto readv_module = new ReadvSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  WritevSystemCallTraceReplayModule *writev_module =
      new WritevSystemCallTraceReplayModule(
          *prefetch_buffer_modules[module_index++], verbose, warn_level,
          pattern_data);
  auto lstat_module = new LStatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto fstat_module = new FStatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto fstatat_module = new FStatatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto utimes_module = new UtimesSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto utimensat_module = new UtimensatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto rename_module = new RenameSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto fsync_module = new FsyncSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto fdatasync_module = new FdatasyncSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto fallocate_module = new FallocateSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto readahead_module = new ReadaheadSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto mknod_module = new MknodSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto pipe_module = new PipeSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto dup_module = new DupSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto dup2_module = new Dup2SystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto dup3_module = new Dup3SystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto fcntl_module = new FcntlSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto exit_module = new ExitSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto execve_module = new ExecveSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto mmap_module = new MmapSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto munmap_module = new MunmapSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto getdents_module = new GetdentsSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, verify, warn_level);
  auto ioctl_module = new IoctlSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto clone_module = new CloneSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto vfork_module = new VForkSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto umask_module = new UmaskSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  SetxattrSystemCallTraceReplayModule *setxattr_module =
      new SetxattrSystemCallTraceReplayModule(
          *prefetch_buffer_modules[module_index++], verbose, verify, warn_level,
          pattern_data);
  LSetxattrSystemCallTraceReplayModule *lsetxattr_module =
      new LSetxattrSystemCallTraceReplayModule(
          *prefetch_buffer_modules[module_index++], verbose, verify, warn_level,
          pattern_data);
  FSetxattrSystemCallTraceReplayModule *fsetxattr_module =
      new FSetxattrSystemCallTraceReplayModule(
          *prefetch_buffer_modules[module_index++], verbose, verify, warn_level,
          pattern_data);
  auto ftruncate_module = new FTruncateSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto socket_module = new SocketSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto socketpair_module = new SocketPairSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto epoll_create_module = new EPollCreateSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto accept_module = new AcceptSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);
  auto accept4_module = new Accept4SystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++], verbose, warn_level);

  /*
   * This vector is going to used to load replaying modules.
   * Therefore, add replaying modules into this vector in here.
   * Remeber to modify here when supporting a new system call.
   */
  std::vector<SystemCallTraceReplayModule *> system_call_trace_replay_modules;
  system_call_trace_replay_modules.push_back(open_module);
  system_call_trace_replay_modules.push_back(openat_module);
  system_call_trace_replay_modules.push_back(close_module);
  system_call_trace_replay_modules.push_back(read_module);
  system_call_trace_replay_modules.push_back(write_module);
  system_call_trace_replay_modules.push_back(lseek_module);
  system_call_trace_replay_modules.push_back(pread_module);
  system_call_trace_replay_modules.push_back(mmappread_module);
  system_call_trace_replay_modules.push_back(access_module);
  system_call_trace_replay_modules.push_back(faccessat_module);
  system_call_trace_replay_modules.push_back(chdir_module);
  system_call_trace_replay_modules.push_back(fchdir_module);
  system_call_trace_replay_modules.push_back(chroot_module);
  system_call_trace_replay_modules.push_back(truncate_module);
  system_call_trace_replay_modules.push_back(creat_module);
  system_call_trace_replay_modules.push_back(link_module);
  system_call_trace_replay_modules.push_back(linkat_module);
  system_call_trace_replay_modules.push_back(unlink_module);
  system_call_trace_replay_modules.push_back(unlinkat_module);
  system_call_trace_replay_modules.push_back(symlink_module);
  system_call_trace_replay_modules.push_back(rmdir_module);
  system_call_trace_replay_modules.push_back(mkdir_module);
  system_call_trace_replay_modules.push_back(mkdirat_module);
  system_call_trace_replay_modules.push_back(stat_module);
  system_call_trace_replay_modules.push_back(statfs_module);
  system_call_trace_replay_modules.push_back(fstatfs_module);
  system_call_trace_replay_modules.push_back(pwrite_module);
  system_call_trace_replay_modules.push_back(mmappwrite_module);
  system_call_trace_replay_modules.push_back(readlink_module);
  system_call_trace_replay_modules.push_back(utime_module);
  system_call_trace_replay_modules.push_back(chmod_module);
  system_call_trace_replay_modules.push_back(fchmod_module);
  system_call_trace_replay_modules.push_back(fchmodat_module);
  system_call_trace_replay_modules.push_back(chown_module);
  system_call_trace_replay_modules.push_back(readv_module);
  system_call_trace_replay_modules.push_back(writev_module);
  system_call_trace_replay_modules.push_back(lstat_module);
  system_call_trace_replay_modules.push_back(fstat_module);
  system_call_trace_replay_modules.push_back(fstatat_module);
  system_call_trace_replay_modules.push_back(utimes_module);
  system_call_trace_replay_modules.push_back(utimensat_module);
  system_call_trace_replay_modules.push_back(rename_module);
  system_call_trace_replay_modules.push_back(fsync_module);
  system_call_trace_replay_modules.push_back(fdatasync_module);
  system_call_trace_replay_modules.push_back(fallocate_module);
  system_call_trace_replay_modules.push_back(readahead_module);
  system_call_trace_replay_modules.push_back(mknod_module);
  system_call_trace_replay_modules.push_back(pipe_module);
  system_call_trace_replay_modules.push_back(dup_module);
  system_call_trace_replay_modules.push_back(dup2_module);
  system_call_trace_replay_modules.push_back(dup3_module);
  system_call_trace_replay_modules.push_back(fcntl_module);
  system_call_trace_replay_modules.push_back(exit_module);
  system_call_trace_replay_modules.push_back(execve_module);
  system_call_trace_replay_modules.push_back(mmap_module);
  system_call_trace_replay_modules.push_back(munmap_module);
  system_call_trace_replay_modules.push_back(getdents_module);
  system_call_trace_replay_modules.push_back(ioctl_module);
  system_call_trace_replay_modules.push_back(clone_module);
  system_call_trace_replay_modules.push_back(vfork_module);
  system_call_trace_replay_modules.push_back(umask_module);
  system_call_trace_replay_modules.push_back(setxattr_module);
  system_call_trace_replay_modules.push_back(lsetxattr_module);
  system_call_trace_replay_modules.push_back(fsetxattr_module);
  system_call_trace_replay_modules.push_back(ftruncate_module);
  system_call_trace_replay_modules.push_back(socket_module);
  system_call_trace_replay_modules.push_back(socketpair_module);
  system_call_trace_replay_modules.push_back(epoll_create_module);
  system_call_trace_replay_modules.push_back(accept_module);
  system_call_trace_replay_modules.push_back(accept4_module);

  return system_call_trace_replay_modules;
}

/**
 * Add all system call replay modules to min heap if the module has extents
 */
void load_syscall_modules(std::vector<SystemCallTraceReplayModule *>
                              &system_call_trace_replay_modules) {
  // Add all the modules to min heap if the module has extents
  for (auto &system_call_trace_replay_module :
       system_call_trace_replay_modules) {
    SystemCallTraceReplayModule *module = system_call_trace_replay_module;
    // getSharedExtent() == NULL means that there are no extents in the module.
    if (module->getSharedExtent()) {
      /*
       * Assert that only version 1.0 is allowed (at this point). Exit if
       * version is wrong.
       * This means that our replayer can process input DataSeries with version
       * x.y
       * if y <= supported_minor_version and x == supported_major_version.
       */
      if (!(module->is_version_compatible(supported_major_version,
                                          supported_minor_version))) {
        std::cerr << "System call replayer currently only support system call "
                     "traces dataseries with version"
                  << supported_major_version << '.' << supported_minor_version
                  << std::endl;
        exit(0);
      }
      module->prepareRow();
      auto movModulePtr = module->move();
      initExecutionHeap(movModulePtr->executing_pid());
      executionHeaps[movModulePtr->executing_pid()].push(movModulePtr);
      syscallMapLast[module->sys_call_name()] = module;
      module->setReplayerIndex(replayerIdx++);
      if (movModulePtr->sys_call_name() == "umask") {
        mainThreadID = movModulePtr->executing_pid();
      }
    } else {
      // Delete the module since it has no system call to replay.
      system_call_trace_replay_module = nullptr;
      delete module;
    }
  }
  numberOfSyscalls = new tbb::atomic<uint64_t>[replayerIdx];
  finishedModules = new tbb::atomic<bool>[replayerIdx];
  std::fill_n(numberOfSyscalls, replayerIdx, 1);
  std::fill_n(finishedModules, replayerIdx, false);
}

/**
 * Before replay any system call, we need to prepare it by
 * setting mask value, file descriptor table, etc. That's
 * where this function comes in.
 */
void prepare_replay() {
  SystemCallTraceReplayModule *syscall_module;
  // Process first record in the dataseries
  if (executionHeaps[mainThreadID].try_pop(syscall_module)) {
    numberOfSyscalls[syscall_module->getReplayerIndex()]--;
    // Get a module that has min unique_id
    // First module to replay should be umask.
    assert(syscall_module->sys_call_name() == "umask");
    // First record should be a umask record.
    assert(syscall_module->unique_id() == 0);

    /*
     * Call umask(0) to "turn off" umask. This is needed b/c the kernel still
     * has to have some umask value.
     * By default the umask of the user running the replayer will be used,
     * and applied to whatever mode values we pass to syscalls like mkdir(path,
     * mode).
     * Essentially, kernel will not modify the mode values that we pass to
     * system calls.
     * Thus whatever mode we pass to mkdir() will take place.
     */
    umask(0);

    // This is needed to initialize first file descriptor
    pid_t traced_app_pid = syscall_module->executing_pid();
    // Initialize standard map values (STDIN, STDOUT, STDERR, AT_FDCWD)
    std::map<int, int> std_fd_map;
    std_fd_map[STDIN_FILENO] = STDIN_FILENO;
    std_fd_map[STDOUT_FILENO] = STDOUT_FILENO;
    std_fd_map[STDERR_FILENO] = STDERR_FILENO;
    std_fd_map[AT_FDCWD] = AT_FDCWD;
    SystemCallTraceReplayModule::replayer_resources_manager_.initialize(
        SystemCallTraceReplayModule::syscall_logger_, traced_app_pid,
        std_fd_map);
    setRunning(traced_app_pid, syscall_module);
    // Replay umask operation.
    syscall_module->execute();
    lastExecutedSyscallID = syscall_module->unique_id();
  }
}

/**
 * Before we perform analysis, we have to register all the analysis modules
 * in the vector `analysisModules`.
 */
void prepare_analysis() {
  // TODO-NEWPEOPLE: We want analysisModules to contain all the analysis modules
  // we plan on running. We'd also like to use the command line to specify which
  // modules to run. Not sure how to do this. Ideally you could specify them
  // like
  // ./reanimator-replayer --analysis "NumericalAnalysisModule,FooAnalysisModule"
  // where FooAnalysisModule was written by the user, not us. But for
  // sufficiently complex analysis, passing a config file might be the best
  // solution.
  //analysisModules.push_back(new DurationAnalysisModule);
  //analysisModules.push_back(new SyscallCountAnalysisModule);
  //analysisModules.push_back(new NumericalAnalysisModule);
  analysisModules.push_back(new CorrelationAnalysisModule);
}

inline void batch_syscall_modules(SystemCallTraceReplayModule *module = nullptr,
                                  bool isFirstTime = false,
                                  unsigned int batch_size = 50) {
  int count = batch_size;

  SystemCallTraceReplayModule *current = nullptr;

  //if (finishedModules[module->getReplayerIndex()] ||
  //    numberOfSyscalls[module->getReplayerIndex()] > batch_size * 2) {
  if (finishedModules[module->getReplayerIndex()]) {
    return;
  }

  current = syscallMapLast[module->sys_call_name()];

  bool endOfRecord = false;
  auto copy = current->move();
  auto readMod = current;

  if (!isFirstTime) {
    executionHeaps[copy->executing_pid()].push(copy);
    numberOfSyscalls[copy->getReplayerIndex()]++;
  }

  while (--count != 0) {
    if (readMod->cur_extent_has_more_record() ||
        readMod->getSharedExtent() != nullptr) {
      PROFILE_START(1)
      readMod->prepareRow();
      PROFILE_END(1, 2, fileReading_Batch_file)

      if (count != 1) {
        PROFILE_START(5)
        auto ptr = readMod->move();
        PROFILE_END(5, 6, fileReading_Batch_move)

        PROFILE_START(3)
        //std::cout << "pushed " << ptr->sys_call_name() << " for thread " << ptr->executing_pid() << std::endl;
        executionHeaps[ptr->executing_pid()].push(ptr);
        PROFILE_END(3, 4, fileReading_Batch_push)

        numberOfSyscalls[ptr->getReplayerIndex()]++;
        //int total = 0;
        //for ( auto it = executionHeaps.begin(); it != executionHeaps.end(); ++it )
        //    total += it->second.size();
        //int num_total = 0;
        //for (int i = 0; i < replayerIdx; i++) {
        //    if (numberOfSyscalls[i] < LLONG_MAX)
        //        num_total += numberOfSyscalls[i];
        //}
        //debug_mutex.lock();
        //std::cout << "total syscalls in execHeaps " << total << " numberOfSyscalls " << num_total << std::endl;
        //debug_mutex.unlock();
      }

    } else {
      syscallMapLast[readMod->sys_call_name()] = readMod;
      finishedModules[readMod->getReplayerIndex()] = true;
      endOfRecord = true;
      numberOfSyscalls[readMod->getReplayerIndex()] = LLONG_MAX;
      break;
    }
  }

  if (count == 0) {
    if (!endOfRecord) {
      syscallMapLast[readMod->sys_call_name()] = readMod;
    }
  }
}

void batch_for_all_syscalls(int batch_size = 50) {
  for (auto module_pair : syscallMapLast) {
    batch_syscall_modules(module_pair.second, isFirstBatch, batch_size);
  }
  isFirstBatch = false;
}

//auto checkModulesFinished = []() -> bool {
//  for (int i = 0; i < replayerIdx; ++i) {
//    if (!finishedModules[i]) return false;
//  }
//  return true;
//};
auto checkModulesFinished = []() -> bool {
  bool isAllFinished = true;
  for (int i = 0; i < replayerIdx; ++i) {
    isAllFinished &= static_cast<bool>(finishedModules[i]);
  }
  return isAllFinished;
};

auto getMinSyscall = []() -> uint64_t {
  tbb::atomic<uint64_t> min = ULONG_MAX;
  for (int i = 0; i < replayerIdx; ++i) {
    min = std::min(min, numberOfSyscalls[i]);
  }
  return min;
};

auto checkSequential = [](SystemCallTraceReplayModule *check) -> bool {
  return (uint64_t)check->unique_id() <= lastExecutedSyscallID + 150;
};

auto checkExecutionValidation = [](SystemCallTraceReplayModule *check) -> bool {
  if (!checkSequential(check)) {
    return false;
  }
  for (auto running : currentExecutions) {
    auto compare = running.second;
    if (compare == nullptr) {
      return false;
    }
    if (check != compare) {
      if (!((check->time_called() >= compare->time_called() &&
             check->time_called() <= compare->time_returned()) ||
            (check->time_returned() >= compare->time_called() &&
             check->time_returned() <= compare->time_returned()))) {
        if (check->time_called() > compare->time_returned()) {
          return false;
        }
      }
    }
  }
  return true;
};

void readerThread() {
  while (!checkModulesFinished()) {
    PROFILE_START(3)
    // TODO-NEWPEOPLE: Ask Umit what this magic number 100 means. If possible,
    // move it to a named constant.
    //while (getMinSyscall() > (100 * nThreads)) {
    //  debug_mutex.lock();
    //  std::cout << "minsyscall " << getMinSyscall() << " nthreads " << nThreads << std::endl;
    //  debug_mutex.unlock();
    //  SystemCallTraceReplayModule *execute_replayer = nullptr;
    //  while (allocationQueue.try_pop(execute_replayer)) {
    //    delete execute_replayer;
    //  }
    //  std::this_thread::yield();
    //}
    //debug_mutex.lock();
    //std::cout << "reader looping\n";
    //debug_mutex.unlock();
    batch_for_all_syscalls(150 * nThreads);
    PROFILE_END(3, 4, fileReading)
  }
}

void executionThread(int64_t threadID) {
  SystemCallTraceReplayModule *execute_replayer = nullptr;
  SystemCallTraceReplayModule *prev_replayer = nullptr;
  tbb::atomic<uint64_t> num_syscalls_processed = 0;

  PROFILE_START(10)

  //while (executionHeaps[threadID].try_pop(execute_replayer)) {
  while (!checkModulesFinished() || executionHeaps[threadID].size()) {
    if (!executionHeaps[threadID].try_pop(execute_replayer)) {
        std::this_thread::yield();
        continue;
    }
    PROFILE_END(10, 11, gettingRecord)
    //debug_mutex.lock();
    //std::cout << "popped syscall " << execute_replayer->sys_call_name() << " from thread " << threadID << " (" << executionHeaps[threadID].size() << " left)" << std::endl;
    //debug_mutex.unlock();

    numberOfSyscalls[execute_replayer->getReplayerIndex()]--;

    PROFILE_START(12)

    PROFILE_START(20)
    while (getMinSyscall() < (10 * nThreads) && !checkModulesFinished()) {
    }
    PROFILE_END(20, 21, executionSpinning)

    setRunning(threadID, execute_replayer);
    allocationQueue.push(prev_replayer);

    PROFILE_START(1)
    if (nThreads > 1) {
      if (!analysis && !checkExecutionValidation(execute_replayer)) {
        setRunning(threadID, nullptr);
        executionHeaps[threadID].push(execute_replayer);
        numberOfSyscalls[execute_replayer->getReplayerIndex()]++;
        prev_replayer = nullptr;
        debug_mutex.lock();
        std::cout << threadID << ":invalid\n";
        debug_mutex.unlock();
        std::this_thread::yield();
        continue;
      }
    }
    //debug_mutex.lock();
    //std::cout << threadID << ":valid execution time\n";
    //debug_mutex.unlock();
    // TODO-NEWPEOPLE: There might be a usecase where we want to execute the
    // trace file while performing analysis at the same time. It might be nice
    // to record metrics of the actual execution so you could later compare
    // to an analysis of the static file. For example, when the trace was
    // recorded, writes were taking 100 ms. But when replayed, writes are only
    // taking 1 ms.
    if (analysis) {
      for (auto am : analysisModules) {
        // NOTE-NEWPEOPLE: `execute_replayer` is the current
        // SystemCallTraceReplayModule that we have reached in the trace. We
        // call its analyze function and pass in a reference to each analysis
        // module. The TraceReplayModule can implement its own version of the
        // `analyze` function to change how the system call wants to be
        // analyzed. Typically, the system call will pass all its information to
        // the analysis module.
        execute_replayer->analyze(*am);
      }
      if (execute_replayer->sys_call_name() == "clone") {
        nThreads++;
        int64_t pid = execute_replayer->return_value();
        setRunning(pid, nullptr);
        threads.emplace_back(executionThread, pid);
        debug_mutex.lock();
        std::cout << "created thread " << pid << std::endl;
        debug_mutex.unlock();
      }
    } else {
      execute_replayer->execute();
    }

    lastExecutedSyscallID =
        std::max((int64_t)lastExecutedSyscallID, execute_replayer->unique_id());
    PROFILE_END(1, 2, duration)

    num_syscalls_processed++;

    // Verify that the state of resources manager is consistent for every
    // SCAN_FD_FREQUENCY sys calls.
    if (num_syscalls_processed % SCAN_FD_FREQUENCY == 0) {
      SystemCallTraceReplayModule::replayer_resources_manager_
          .validate_consistency();
    }

    if ((num_syscalls_processed % 1000000) == 0u) {
      PROFILE_PRINT("total syscall execution time: ", duration)
      PROFILE_PRINT("total spinning over I/O wait time: ", executionSpinning)
      PROFILE_PRINT("total file reading time: ", fileReading)
      PROFILE_PRINT("total loop over syscall time: ", loop)
      PROFILE_PRINT("total DS file batch reading time: ",
                    fileReading_Batch_file)
      PROFILE_PRINT("total syscall record push to PQ time: ",
                    fileReading_Batch_push)
      PROFILE_PRINT("total syscall record move to PQ time: ",
                    fileReading_Batch_move)
      PROFILE_PRINT("total syscall record try_pop to PQ time: ", gettingRecord)
    }
    PROFILE_END(12, 13, loop)
    prev_replayer = execute_replayer;
    PROFILE_SAMPLE(10)
  }
  currentExecutions.erase(threadID);
  debug_mutex.lock();
  std::cout << "execThread ended with id " << threadID << std::endl;
  debug_mutex.unlock();
}

int main(int argc, char *argv[]) {
  int ret = EXIT_SUCCESS;
  bool verbose = false;
  bool verify = false;
  int warn_level = DEFAULT_MODE;
  std::string pattern_data = "";
  std::string log_filename = "";
  std::vector<std::string> input_files;
#ifdef PROFILE_ENABLE
  int64_t warmup = 0;
#endif
  PROFILE_START(5)

  // Process options found on the command line.
  process_options(argc, argv, verbose, verify, warn_level,
                  pattern_data, log_filename, input_files);
  // Create an instance of logger class and open log file to write replayer logs
  SystemCallTraceReplayModule::syscall_logger_ =
      new SystemCallTraceReplayLogger(log_filename);
  // If pattern data is equal to urandom, then open /dev/urandom file
  if (pattern_data == "urandom") {
    SystemCallTraceReplayModule::random_file_.open("/dev/urandom");
    if (!SystemCallTraceReplayModule::random_file_.is_open()) {
      std::cerr << "Unable to open file '/dev/urandom'.\n";
      // Delete the instance of logger class and close the log file
      delete SystemCallTraceReplayModule::syscall_logger_;
      exit(EXIT_FAILURE);
    }
  }

  std::vector<PrefetchBufferModule *> prefetch_buffer_modules =
      create_prefetch_buffer_modules(input_files);

  std::vector<SystemCallTraceReplayModule *> system_call_trace_replay_modules =
      create_system_call_trace_replay_modules(prefetch_buffer_modules, verbose,
                                              verify, warn_level, pattern_data);

  // Double check to make sure all replaying modules are loaded.
  if (system_call_trace_replay_modules.size() !=
      prefetch_buffer_modules.size()) {
    std::cerr << "The number of loaded replaying modules is not same"
              << "as the number of supported system calls\n";
    // Delete the instance of logger class and close the log file
    delete SystemCallTraceReplayModule::syscall_logger_;
    abort();
  }

  load_syscall_modules(system_call_trace_replay_modules);
  prepare_replay();
  prepare_analysis();
  batch_for_all_syscalls(1000);

  PROFILE_END(5, 6, warmup)
  PROFILE_PRINT("warmup: ", warmup)

  std::thread reader(readerThread);
  reader.join();
  std::cout << "reader thread finished\n";
  std::thread executor(executionThread, mainThreadID);
  std::cout << "main thread started\n";
  executor.join();
  std::cout << "main thread finished\n";
  for (uint64_t i = 0; i < threads.size(); i++) {
    std::cout << "joining thread at index " << i << "\n";
    threads[i].join();
  }
  std::cout << "all threads finished\n";
  if (analysis) {
    for (auto am : analysisModules) {
      am->printMetrics(std::cout);
    }
  }
  // destruct analysis modules
  for (auto am : analysisModules) {
    delete am;
  }

  // Close /dev/urandom file
  if (pattern_data == "urandom") {
    SystemCallTraceReplayModule::random_file_.close();
  }

  // Delete the instance of logger class and close the log file
  delete SystemCallTraceReplayModule::syscall_logger_;

  return ret;
}
