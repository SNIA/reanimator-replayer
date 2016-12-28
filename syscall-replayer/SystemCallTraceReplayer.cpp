/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
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
 */

#include "SystemCallTraceReplayer.hpp"

/*
 * min heap uses this function to sort elements in the tree.
 * The sorting key is unique id.
 */
struct CompareByUniqueID {
  bool operator()(SystemCallTraceReplayModule* m1,
		  SystemCallTraceReplayModule* m2) const {
    return (m1->unique_id() >= m2->unique_id());
  }
};

/*
 * This function declares a group of options that will
 * be allowed for the replayer, store options in a
 * variable map and return it.
 * @param argc: number of arguments on the command line.
 * @param argv: arguments on the command line.
 * @return: vm - a variable map that contains all the options
 *               found on the command line.
 */
boost::program_options::variables_map get_options(int argc,
						  char *argv[]) {
  namespace po = boost::program_options;
  /*
   * Declare a group of options that will be
   * allowed only on command line
   */
  po::options_description generic("Generic options");
  generic.add_options()
    ("version,V", "print version of system call replayer")
    ("help,h", "produce help message")
    ;

  /*
   * Declare a group of options that will be
   * allowed both on command line and in
   * config file
   */
  po::options_description config("Configuration");
  config.add_options()
    ("verbose,v", "system calls replay in verbose mode")
    ("verify", "verifies that the data being written/read is "
		"exactly what was used originally")
    ("warn,w", po::value<int>(), "system call replays in warn mode")
    ("pattern,p", po::value<std::string>(),
     "write repeated pattern data in write system call")
    ("logger,l", po::value<std::string>(),
     "write the replayer logs in specified filename")
    ;

  /*
   * Hidden options, will be allowed both on command line and
   * in config file, but will not be shown to the user.
   */
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-files,I", po::value<std::vector<std::string> >(),"input files")
    ;

  po::options_description cmdline_options;
  cmdline_options.add(generic).add(config).add(hidden);

  /*
    po::options_description config_file_options;
    config_file_options.add(config).add(hidden);
  */

  po::options_description visible("Allowed options");
  visible.add(generic).add(config);

  po::positional_options_description p;
  p.add("input-files", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).
	    options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << visible << std::endl;
    exit(EXIT_SUCCESS);
  }

  return vm;
}

/*
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
void process_options(int argc, char *argv[],
		     bool &verbose, bool &verify,
		     int &warn_level, std::string &pattern_data,
		     std::string &log_filename,
		     std::vector<std::string> &input_files) {
  boost::program_options::variables_map options_vm = get_options(argc, argv);

  if (options_vm.count("version")) {
    std::cerr << "sysreplayer version 1.0" << std::endl;
    exit(EXIT_SUCCESS);
  }

  if (options_vm.count("warn")){
    warn_level = options_vm["warn"].as<int>();
    if(warn_level > ABORT_MODE){
      std::cerr << "Wrong value for warn option" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  if (options_vm.count("verify")) {
    verify = true;
  }

  if (options_vm.count("verbose")) {
    verbose = true;
  }

  if (options_vm.count("pattern")){
    pattern_data = options_vm["pattern"].as<std::string>();
  }

  if (options_vm.count("logger")){
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

  if (options_vm.count("input-files")) {
    input_files = options_vm["input-files"].as<std::vector<std::string> >();
  } else {
    std::cout << "No dataseries input files.\n";
    exit(EXIT_FAILURE);
  }
}

/*
 * Creates a prefetch buffer module for each system call.
 * Each prefetch buffer module is going to be used to create
 * system call module.
 * Note that the order of prefetch buffer modules in the return
 * vector matters. Meaning that each system call module
 * needs to use its corresponding prefetch buffer module.
 * Ex: OpenSystemCallTraceReplayModule needs to be instantiated
 * by using the first prefetch buffer module, OpenAtSystemCallTraceReplayModule
 * needs to be instantiated by using the second, CloseSystemCallTraceReplayModule
 * uses third, so on and so forth.
 ******Remember to modify here when supporting a new system call.********
 * Return a list of prefetch buffer modules
 */
std::vector<PrefetchBufferModule *> create_prefetch_buffer_modules(std::vector<std::string> &input_files) {
  // This is the prefix extent type of all system calls.
  const std::string kExtentTypePrefix = "IOTTAFSL::Trace::Syscall::";

  /*
   * This vector contains the list of system calls that need replaying modules.
   */
  std::vector<std::string> system_calls;
  system_calls.push_back("open");
  system_calls.push_back("openat");
  system_calls.push_back("close");
  system_calls.push_back("read");
  system_calls.push_back("write");
  system_calls.push_back("lseek");
  system_calls.push_back("pread");
  system_calls.push_back("access");
  system_calls.push_back("chdir");
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
  system_calls.push_back("utimes");
  system_calls.push_back("utimensat");
  system_calls.push_back("rename");
  system_calls.push_back("fsync");
  system_calls.push_back("mknod");
  system_calls.push_back("pipe");
  system_calls.push_back("dup");
  system_calls.push_back("dup2");
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

  std::vector<TypeIndexModule *> type_index_modules;

  for (unsigned int i = 0; i < system_calls.size(); i++) {
    TypeIndexModule *type_index_module = new TypeIndexModule(kExtentTypePrefix
                   + system_calls[i]);
    type_index_modules.push_back(type_index_module);
  }

  /* Specify to read dataseries input files */
  for (std::vector<std::string>::iterator iter = input_files.begin();
       iter != input_files.end();
       iter++) {
    for (unsigned int j = 0; j < type_index_modules.size(); j++) {
      type_index_modules[j]->addSource(*iter);
    }
  }

  std::vector<PrefetchBufferModule *> prefetch_buffer_modules;
  for (unsigned int i = 0; i < type_index_modules.size(); ++i) {
    /* Parallel decompress and replay, 8MiB buffer */
    PrefetchBufferModule *module =
      new PrefetchBufferModule(*(type_index_modules[i]), 8 * 1024 * 1024);
    prefetch_buffer_modules.push_back(module);
  }

  return prefetch_buffer_modules;
}

/*
 * Creates a replaying module for each system call.
 *****Remember to modify here to create a replaying module when supporting a new system call.*********
 * IMPORTANT: each entry in prefetch_buffer_modules corresponds to
 * its own module.
 */
std::vector<SystemCallTraceReplayModule *> create_system_call_trace_replay_modules (
  std::vector<PrefetchBufferModule *> prefetch_buffer_modules,
  bool verbose,
  bool verify,
  int warn_level,
  std::string &pattern_data) {
  int module_index = 0;
  OpenSystemCallTraceReplayModule *open_module =
    new OpenSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  OpenatSystemCallTraceReplayModule *openat_module =
    new OpenatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  CloseSystemCallTraceReplayModule *close_module =
    new CloseSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  ReadSystemCallTraceReplayModule *read_module =
    new ReadSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  WriteSystemCallTraceReplayModule *write_module =
    new WriteSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level,
      pattern_data);
  LSeekSystemCallTraceReplayModule *lseek_module =
    new LSeekSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  PReadSystemCallTraceReplayModule *pread_module =
    new PReadSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  AccessSystemCallTraceReplayModule *access_module =
    new AccessSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  ChdirSystemCallTraceReplayModule *chdir_module =
    new ChdirSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  TruncateSystemCallTraceReplayModule *truncate_module =
    new TruncateSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  CreatSystemCallTraceReplayModule *creat_module =
    new CreatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  LinkSystemCallTraceReplayModule *link_module =
    new LinkSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  LinkatSystemCallTraceReplayModule *linkat_module =
    new LinkatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  UnlinkSystemCallTraceReplayModule *unlink_module =
    new UnlinkSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  UnlinkatSystemCallTraceReplayModule *unlinkat_module =
    new UnlinkatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  SymlinkSystemCallTraceReplayModule *symlink_module =
    new SymlinkSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  RmdirSystemCallTraceReplayModule *rmdir_module =
    new RmdirSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  MkdirSystemCallTraceReplayModule *mkdir_module =
    new MkdirSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  MkdiratSystemCallTraceReplayModule *mkdirat_module =
    new MkdiratSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  StatSystemCallTraceReplayModule *stat_module =
    new StatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  StatfsSystemCallTraceReplayModule *statfs_module =
    new StatfsSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  FStatfsSystemCallTraceReplayModule *fstatfs_module =
    new FStatfsSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  PWriteSystemCallTraceReplayModule *pwrite_module =
    new PWriteSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level,
      pattern_data);
  ReadlinkSystemCallTraceReplayModule *readlink_module =
    new ReadlinkSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  UtimeSystemCallTraceReplayModule *utime_module =
    new UtimeSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  ChmodSystemCallTraceReplayModule *chmod_module =
    new ChmodSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  FChmodSystemCallTraceReplayModule *fchmod_module =
    new FChmodSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  FChmodatSystemCallTraceReplayModule *fchmodat_module =
    new FChmodatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  ChownSystemCallTraceReplayModule *chown_module =
    new ChownSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  ReadvSystemCallTraceReplayModule *readv_module =
    new ReadvSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  WritevSystemCallTraceReplayModule *writev_module =
    new WritevSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level,
      pattern_data);
  LStatSystemCallTraceReplayModule *lstat_module =
    new LStatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  FStatSystemCallTraceReplayModule *fstat_module =
    new FStatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  UtimesSystemCallTraceReplayModule *utimes_module =
    new UtimesSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  UtimensatSystemCallTraceReplayModule *utimensat_module =
    new UtimensatSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  RenameSystemCallTraceReplayModule *rename_module =
    new RenameSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  FsyncSystemCallTraceReplayModule *fsync_module =
    new FsyncSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  MknodSystemCallTraceReplayModule *mknod_module =
    new MknodSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  PipeSystemCallTraceReplayModule *pipe_module =
    new PipeSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  DupSystemCallTraceReplayModule *dup_module =
    new DupSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  Dup2SystemCallTraceReplayModule *dup2_module =
    new Dup2SystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  FcntlSystemCallTraceReplayModule *fcntl_module = 
    new FcntlSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  ExitSystemCallTraceReplayModule *exit_module =
    new ExitSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  ExecveSystemCallTraceReplayModule *execve_module =
    new ExecveSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  MmapSystemCallTraceReplayModule *mmap_module =
    new MmapSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  MunmapSystemCallTraceReplayModule *munmap_module =
    new MunmapSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  GetdentsSystemCallTraceReplayModule *getdents_module =
    new GetdentsSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      verify,
      warn_level);
  IoctlSystemCallTraceReplayModule *ioctl_module =
    new IoctlSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  CloneSystemCallTraceReplayModule *clone_module =
    new CloneSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  VForkSystemCallTraceReplayModule *vfork_module =
    new VForkSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);
  UmaskSystemCallTraceReplayModule *umask_module =
    new UmaskSystemCallTraceReplayModule(
      *prefetch_buffer_modules[module_index++],
      verbose,
      warn_level);

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
  system_call_trace_replay_modules.push_back(access_module);
  system_call_trace_replay_modules.push_back(chdir_module);
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
  system_call_trace_replay_modules.push_back(utimes_module);
  system_call_trace_replay_modules.push_back(utimensat_module);
  system_call_trace_replay_modules.push_back(rename_module);
  system_call_trace_replay_modules.push_back(fsync_module);
  system_call_trace_replay_modules.push_back(mknod_module);
  system_call_trace_replay_modules.push_back(pipe_module);
  system_call_trace_replay_modules.push_back(dup_module);
  system_call_trace_replay_modules.push_back(dup2_module);
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

  return system_call_trace_replay_modules;
}

/*
 * Add all system call replay modules to min heap if the module has extents
 */
void load_syscall_modules(std::priority_queue<SystemCallTraceReplayModule*,
  std::vector<SystemCallTraceReplayModule*>,
  CompareByUniqueID> &replayers_heap, std::vector<SystemCallTraceReplayModule *> &system_call_trace_replay_modules) {
  // Add all the modules to min heap if the module has extents
  for (unsigned int i = 0; i < system_call_trace_replay_modules.size(); ++i) {
    SystemCallTraceReplayModule *module = system_call_trace_replay_modules[i];
    // getSharedExtent() == NULL means that there are no extents in the module.
    if (module->getSharedExtent()) {
      /*
       * Assert that only version 1.0 is allowed (at this point). Exit if version is wrong.
       * This means that our replayer can process input DataSeries with version x.y
       * if y <= supported_minor_version and x == supported_major_version.
       */
      if (!(module->is_version_compatible(supported_major_version, supported_minor_version))) {
        std::cerr << "System call replayer currently only support system call traces \
          dataseries with version" << supported_major_version << '.' << supported_minor_version
          << std::endl;
        exit(0);
      }
      replayers_heap.push(module);
    }
  }
}

/*
 * Before replay any system call, we need to prepare it by
 * setting mask value, file descriptor table, etc. That's
 * where this function comes in. 
 */
void prepare_replay(std::priority_queue<SystemCallTraceReplayModule*,
  std::vector<SystemCallTraceReplayModule*>,
  CompareByUniqueID> &syscall_replayer) {
  // Process first record in the dataseries
  if (!syscall_replayer.empty()) {
    // Get a module that has min unique_id
    SystemCallTraceReplayModule *syscall_module = syscall_replayer.top();
    // First module to replay should be umask.
    assert(syscall_module->sys_call_name()=="umask");
    // First record should be a umask record.
    assert(syscall_module->unique_id()==0);

    /* Call umask(0) to “turn off” umask. This is needed b/c the kernel still has to have some umask value.
     * By default the umask of the user running the replayer will be used,
     * and applied to whatever mode values we pass to syscalls like mkdir(path, mode).
     * Essentially, kernel will not modify the mode values that we pass to system calls.
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
      SystemCallTraceReplayModule::syscall_logger_,
      traced_app_pid, std_fd_map);

    syscall_replayer.pop();
    // Replay umask operation.
    syscall_module->execute();
    // Check to see if all the extents in the umask module are processed
    if (syscall_module->cur_extent_has_more_record() ||
      syscall_module->getSharedExtent() != NULL) {
      // No, there are more extents, so we add it to min_heap
      syscall_replayer.push(syscall_module);
    }
  }
}

int main(int argc, char *argv[]) {
  int ret = EXIT_SUCCESS;
  bool verbose = false;
  bool verify = false;
  int warn_level = DEFAULT_MODE;
  std::string pattern_data = "";
  std::string log_filename = "";
  std::vector<std::string> input_files;

  // Process options found on the command line.
  process_options(argc, argv, verbose,
		  verify, warn_level, pattern_data,
		  log_filename, input_files);

  // Create an instance of logger class and open log file to write replayer logs
  SystemCallTraceReplayModule::syscall_logger_ = new SystemCallTraceReplayLogger(log_filename);
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

  std::vector<PrefetchBufferModule *> prefetch_buffer_modules = create_prefetch_buffer_modules(input_files);

  std::vector<SystemCallTraceReplayModule *> system_call_trace_replay_modules = create_system_call_trace_replay_modules(
    prefetch_buffer_modules,
    verbose,
    verify,
    warn_level,
    pattern_data);

  // Double check to make sure all replaying modules are loaded.
  if (system_call_trace_replay_modules.size() != prefetch_buffer_modules.size()) {
    std::cerr << "The number of loaded replaying modules is not same"
	      << "as the number of supported system calls\n";
    // Delete the instance of logger class and close the log file
    delete SystemCallTraceReplayModule::syscall_logger_;
    abort();
  }

  /*
   * Define a min heap that stores each module. The heap is ordered
   * by unique_id field.
   */
  std::priority_queue<SystemCallTraceReplayModule*,
          std::vector<SystemCallTraceReplayModule*>,
          CompareByUniqueID> replayers_heap;
  load_syscall_modules(replayers_heap, system_call_trace_replay_modules);
  prepare_replay(replayers_heap);
  SystemCallTraceReplayModule *execute_replayer = NULL;

  // Process all the records in the dataseries
  while(!replayers_heap.empty()) {
    // Get a module that has min unique_id
    execute_replayer = replayers_heap.top();
    replayers_heap.pop();
    // Replay the operation that has min unique_id
    execute_replayer->execute();
    // Check to see if all the extents in the module are processed
    if (execute_replayer->cur_extent_has_more_record() ||
      execute_replayer->getSharedExtent() != NULL) {
      // No, there are more extents, so we add it to min_heap
      replayers_heap.push(execute_replayer);
    } else {
      // No, there are no more extents, so we delete the module
      delete execute_replayer;
    }
  }

  // Close /dev/urandom file
  if (pattern_data == "urandom") {
    SystemCallTraceReplayModule::random_file_.close();
  }

  // Delete the instance of logger class and close the log file
  delete SystemCallTraceReplayModule::syscall_logger_;

  return ret;
}
