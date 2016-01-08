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
 * open, close, read, and write and has various options to modify the
 * behavior of this program.
 *
 * USAGE
 * ./system-call-replayer <-vwp> <input files>
 * 
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

// min heap uses this to sort elements in the tree.
struct CompareByTimeCalled {
  bool operator()(SystemCallTraceReplayModule* m1, SystemCallTraceReplayModule* m2) const {
    if (m1->time_called() >= m2->time_called())
      return true;
    else 
      return false;
  }
};

// define the static fd_map_ in SystemCallTraceReplayModule
std::map<int, int> SystemCallTraceReplayModule::fd_map_;

int main(int argc, char *argv[]) {
  namespace po = boost::program_options;
  
  int ret = EXIT_SUCCESS;
  bool verbose = false;
  bool verify = false;
  int warn_level = DEFAULT_MODE;
  std::string pattern_data = "";
  std::vector<std::string> input_files;
  
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
    std::cerr << visible << "\n";
    return ret;
  }
  
  if (vm.count("version")) {
    std::cerr << "sysreplayer version 1.0" << "\n";
    return ret;
  }

  if (vm.count("warn")){
    warn_level = vm["warn"].as<int>();
    if(warn_level > ABORT_MODE){
      std::cout << "Wrong value for warn option" << std::endl;  
      return EXIT_FAILURE;}
  }

  if (vm.count("verify")) {
    verify = true;
  }

  if (vm.count("verbose")) {
    verbose = true;
  }
  
  if (vm.count("pattern")){
    pattern_data = vm["pattern"].as<std::string>();
  }
  
  if (vm.count("input-files")) {
    input_files = vm["input-files"].as<std::vector<std::string> >();
  } else {
    std::cout << "No dataseries input files.\n";
    ret = EXIT_FAILURE;
    return ret;
  }

  /*
   * This reads all extents of system call
   * operations from a set of DataSeries files.
   */
  const std::string kExtentTypePrefix = "IOTTAFSL::Trace::Syscall::";

  std::vector<std::string> system_calls;
  system_calls.push_back("open");
  system_calls.push_back("close");
  system_calls.push_back("read");
  system_calls.push_back("write");
  
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
    /* Parallel decompress and replay, 64MiB buffer */
    PrefetchBufferModule *module = new PrefetchBufferModule(*(type_index_modules[i]), 64 * 1024 * 1024);
    prefetch_buffer_modules.push_back(module);
  }

  OpenSystemCallTraceReplayModule *open_replayer = new OpenSystemCallTraceReplayModule(*prefetch_buffer_modules[0],
										       verbose,
										       warn_level);
  CloseSystemCallTraceReplayModule *close_replayer = new CloseSystemCallTraceReplayModule(*prefetch_buffer_modules[1],
											  verbose,
											  warn_level);
  ReadSystemCallTraceReplayModule *read_replayer = new ReadSystemCallTraceReplayModule(*prefetch_buffer_modules[2],
										       verbose,
										       verify,
										       warn_level);
  WriteSystemCallTraceReplayModule *write_replayer = new WriteSystemCallTraceReplayModule(*prefetch_buffer_modules[3],
											  verbose,
											  verify,
											  warn_level,
											  pattern_data);

  std::vector<SystemCallTraceReplayModule *> system_call_trace_replay_modules;
  system_call_trace_replay_modules.push_back(open_replayer);
  system_call_trace_replay_modules.push_back(close_replayer);
  system_call_trace_replay_modules.push_back(read_replayer);
  system_call_trace_replay_modules.push_back(write_replayer);

  // Define a min heap that stores each module. The heap is ordered by time_called field.
  std::priority_queue<SystemCallTraceReplayModule*, std::vector<SystemCallTraceReplayModule*>, CompareByTimeCalled> replayers_heap;
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
    // Get a module that has min time_called
    execute_replayer = replayers_heap.top();
    replayers_heap.pop();
    // Replay the operation that has min time_called
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
