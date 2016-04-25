/*
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

/*
 * min heap uses this function to sort elements in the tree.
 * The sorting key is unique id.
 */
struct CompareByUniqueID {
  bool operator()(SystemCallTraceReplayModule* m1, SystemCallTraceReplayModule* m2) const {
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
    ("verify", "verifies that the data being written/read is exactly what was used originally")
    ("warn,w", po::value<int>(), "system call replays in warn mode")
    ("pattern,p", po::value<std::string>(), "write repeated pattern data in write system call")
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
  
  if (options_vm.count("input-files")) {
    input_files = options_vm["input-files"].as<std::vector<std::string> >();
  } else {
    std::cout << "No dataseries input files.\n";
    exit(EXIT_FAILURE);
  }
}

// Define the static fd_map_ in SystemCallTraceReplayModule
std::map<int, int> SystemCallTraceReplayModule::fd_map_;

int main(int argc, char *argv[]) {
  int ret = EXIT_SUCCESS;
  bool verbose = false;
  bool verify = false;
  int warn_level = DEFAULT_MODE;
  std::string pattern_data = "";
  std::vector<std::string> input_files;

  // Process options found on the command line.
  process_options(argc, argv, verbose, 
		  verify, warn_level, pattern_data,
		  input_files);
  
  // This is the prefix extent type of all system calls. 
  const std::string kExtentTypePrefix = "IOTTAFSL::Trace::Syscall::";

  /*
   * This vector contains the list of system calls that need replaying modules.
   */
  std::vector<std::string> system_calls;
  system_calls.push_back("open");
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
  system_calls.push_back("unlink");
  system_calls.push_back("symlink");
  system_calls.push_back("rmdir");

  std::vector<TypeIndexModule *> type_index_modules;

  for (unsigned int i = 0; i < system_calls.size(); i++) {
    TypeIndexModule *type_index_module = new TypeIndexModule(kExtentTypePrefix + system_calls[i]);
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
    PrefetchBufferModule *module = new PrefetchBufferModule(*(type_index_modules[i]), 8 * 1024 * 1024);
    prefetch_buffer_modules.push_back(module);
  }

  /*
   * Create a replaying module for each system call.
   * Remeber to modify here to create a replaying module when supporting a new system call.
   * IMPORTANT: each entry in prefetch_buffer_modules corresponds to its own module.
   */
  OpenSystemCallTraceReplayModule *open_module = new OpenSystemCallTraceReplayModule(*prefetch_buffer_modules[0],
										       verbose,
										       warn_level);
  CloseSystemCallTraceReplayModule *close_module = new CloseSystemCallTraceReplayModule(*prefetch_buffer_modules[1],
											  verbose,
											  warn_level);
  ReadSystemCallTraceReplayModule *read_module = new ReadSystemCallTraceReplayModule(*prefetch_buffer_modules[2],
										       verbose,
										       verify,
										       warn_level);
  WriteSystemCallTraceReplayModule *write_module = new WriteSystemCallTraceReplayModule(*prefetch_buffer_modules[3],
											  verbose,
											  verify,
											  warn_level,
											  pattern_data);
  LSeekSystemCallTraceReplayModule *lseek_module = new LSeekSystemCallTraceReplayModule(*prefetch_buffer_modules[4],
											verbose,
											warn_level);
  PReadSystemCallTraceReplayModule *pread_module = new PReadSystemCallTraceReplayModule(*prefetch_buffer_modules[5],
										       verbose,
										       verify,
										       warn_level);
  AccessSystemCallTraceReplayModule *access_module = new AccessSystemCallTraceReplayModule(*prefetch_buffer_modules[6],
											   verbose,
											   warn_level);
  ChdirSystemCallTraceReplayModule *chdir_module = new ChdirSystemCallTraceReplayModule(*prefetch_buffer_modules[7],
											verbose,
											warn_level);
  TruncateSystemCallTraceReplayModule *truncate_module = new TruncateSystemCallTraceReplayModule(*prefetch_buffer_modules[8],
												 verbose,
												 warn_level);
  CreatSystemCallTraceReplayModule *creat_module = new CreatSystemCallTraceReplayModule(*prefetch_buffer_modules[9],
											verbose,
											warn_level);
  LinkSystemCallTraceReplayModule *link_module = new LinkSystemCallTraceReplayModule(*prefetch_buffer_modules[10],
										     verbose,
										     warn_level);
  UnlinkSystemCallTraceReplayModule *unlink_module = new UnlinkSystemCallTraceReplayModule(*prefetch_buffer_modules[11],
											   verbose,
											   warn_level);
  SymlinkSystemCallTraceReplayModule *symlink_module = new SymlinkSystemCallTraceReplayModule(*prefetch_buffer_modules[12],
											      verbose,
											      warn_level);
  RmdirSystemCallTraceReplayModule *rmdir_module = new RmdirSystemCallTraceReplayModule(*prefetch_buffer_modules[13],
											verbose,
											warn_level);
  /*
   * This vector is going to used to load replaying modules.
   * Therefore, add replaying modules into this vector in here.
   * Remeber to modify here when supporting a new system call.
   */
  std::vector<SystemCallTraceReplayModule *> system_call_trace_replay_modules;
  system_call_trace_replay_modules.push_back(open_module);
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
  system_call_trace_replay_modules.push_back(unlink_module);
  system_call_trace_replay_modules.push_back(symlink_module);
  system_call_trace_replay_modules.push_back(rmdir_module);

  // Double check to make sure all replaying modules are loaded.
  if (system_call_trace_replay_modules.size() != system_calls.size()) {
    std::cerr << "The number of loaded replaying modules is not same as the number of supported system calls\n";
    abort();
  }

  // Define a min heap that stores each module. The heap is ordered by unique_id field.
  std::priority_queue<SystemCallTraceReplayModule*, 
		      std::vector<SystemCallTraceReplayModule*>, 
		      CompareByUniqueID> replayers_heap;
  SystemCallTraceReplayModule *execute_replayer = NULL;

  // Add all the modules to min heap if the module has extents
  for (unsigned int i = 0; i < system_call_trace_replay_modules.size(); ++i) {
    SystemCallTraceReplayModule *module = system_call_trace_replay_modules[i];
    // getSharedExtent() == NULL means that there are no extents in the module.
    if (module->getSharedExtent()) {
      replayers_heap.push(module);
    }
  }
  
  // Process all the records in the dataseries
  while(!replayers_heap.empty()) {
    // Get a module that has min unique_id
    execute_replayer = replayers_heap.top();
    replayers_heap.pop();
    // Replay the operation that has min unique_id
    execute_replayer->execute();
    // Check to see if all the extents in the module are processed
    if (execute_replayer->cur_extent_has_more_record() || 
	execute_replayer->getSharedExtent() != NULL){
      // No, there are more extents, so we add it to min_heap
      replayers_heap.push(execute_replayer);
    }
  }

  return ret;
}
