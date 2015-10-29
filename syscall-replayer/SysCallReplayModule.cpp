/*
 * Copyright (c) 2015 Leixiang Wu
 * Copyright (c) 2011 Jack Ma
 * Copyright (c) 2011 Vasily Tarasov
 * Copyright (c) 2011 Santhosh Kumar Koundinya
 * Copyright (c) 2011 Erez Zadok
 * Copyright (c) 2011 Geoff Kuenning
 * Copyright (c) 2011 Stony Brook University
 * Copyright (c) 2011 Harvey Mudd College
 * Copyright (c) 2011 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <iostream>
#include <string>
#include <sstream>

#include <algorithm>
#include <vector>
#include <queue>         
#include <set>

#include <DataSeries/PrefetchBufferModule.hpp>
#include <DataSeries/TypeIndexModule.hpp>
#include <DataSeries/RowAnalysisModule.hpp>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <cstring>
#include <csignal>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <fcntl.h>


#include <stdexcept>

class SystemCallTraceReplayModule : public RowAnalysisModule {
	protected:
		DoubleField time_called_;
		bool verbose;
	public:
		SystemCallTraceReplayModule(DataSeriesModule &source) : 
			RowAnalysisModule(source),
			time_called_(series, "time_called") {
				verbose = false;
		}

		double time_called() {
			return (double)time_called_.val();
		}

		Extent::Ptr getSharedExtent() {
		    Extent::Ptr e = source.getSharedExtent();
		    if (e == NULL) {
		        completeProcessing();
		        return e;
		    }
		    if (!prepared) {
		        firstExtent(*e);
		    }
		    newExtentHook(*e);
		    series.setExtent(e);
		    if (!prepared) {
		        prepareForProcessing();
		        prepared = true;
		    }
		    return e;
		}

		bool has_more_trace() {
			if (series.morerecords()) {
				return true;
			} else {
				series.clearExtent();
				return false;
			}
		}

		void execute() {
	        ++processed_rows;
	        processRow();
	    	++series;
		}
};

struct LessThanByTimeCalled {
	bool operator()(SystemCallTraceReplayModule* m1, SystemCallTraceReplayModule* m2) const {
		std::cout.precision(32);
		/*
		std::cout<< "compared!!!" << std::endl;

		std::cout << "m1 time called:" << std::fixed << m1->time_called() << std::endl;
		std::cout << "m2 time called:" << std::fixed << m2->time_called() << std::endl;
		*/

		// min heap
   		if (m1->time_called() >= m2->time_called())
   			return true;
   		else 
   			return false;
	}
};


class OpenSystemCallTraceReplayModule : public SystemCallTraceReplayModule {

	private:
		/* DataSeries Open System Call Trace Fields */
		Variable32Field	given_pathname;
		BoolField flag_read_only;
		BoolField flag_write_only;
		BoolField flag_read_and_write;
		BoolField flag_append;
		BoolField flag_async;
		BoolField flag_close_on_exec;
		BoolField flag_create;
		BoolField flag_direct;
		BoolField flag_directory;
		BoolField flag_exclusive;
		BoolField flag_largefile;
		BoolField flag_no_access_time;
		BoolField flag_no_controlling_terminal;
		BoolField flag_no_follow;
		BoolField flag_no_blocking_mode;
		BoolField flag_no_delay;
		BoolField flag_synchronous;
		BoolField flag_truncate;
		BoolField mode_R_user;
		BoolField mode_W_user;
		BoolField mode_X_user;
		BoolField mode_R_group;
		BoolField mode_W_group;
		BoolField mode_X_group;
		BoolField mode_R_others;
		BoolField mode_W_others;
		BoolField mode_X_others;

		uint32_t getOpenFlags() {
			int sum = flag_read_only.val() + flag_write_only.val() + flag_read_and_write.val();
			if (sum != 0) {
				// FIXME!!!!! ERROR
			}
			uint32_t flags = O_RDONLY;
			if (flag_read_only.val() == 1) {
				flags = O_RDONLY;
			} else if (flag_write_only.val() == 1) {
				flags = O_WRONLY;
			} else if (flag_write_only.val() == 1) {
				flags = O_RDWR;
			}
			if (flag_append.val() == 1) {
				flags |= O_APPEND;
			}
			if (flag_async.val() == 1) {
				flags |= O_ASYNC;
			}
			if (flag_close_on_exec.val() == 1) {
				flags |= O_CLOEXEC;
			}
			if (flag_create.val() == 1) {
				flags |= O_CREAT;
			}
			if (flag_direct.val() == 1) {
				flags |= O_DIRECT;
			}
			if (flag_directory.val() == 1) {
				flags |= O_DIRECTORY;
			}
			if (flag_exclusive.val() == 1) {
				flags |= O_EXCL;
			}
			if (flag_largefile.val() == 1) {
				flags |= O_LARGEFILE;
			}
			if (flag_no_access_time.val() == 1) {
				flags |= O_NOATIME;
			}
			if (flag_no_controlling_terminal.val() == 1) {
				flags |= O_NOCTTY;
			}
			if (flag_no_follow.val() == 1) {
				flags |= O_NOFOLLOW;
			}
			if (flag_no_blocking_mode.val() == 1) {
				flags |= O_NONBLOCK;
			}
			if (flag_no_delay.val() == 1) {
				flags |= O_NDELAY;
			}
			if (flag_synchronous.val() == 1) {
				flags |= O_SYNC;
			}
			if (flag_truncate.val() == 1) {
				flags |= O_TRUNC;
			}
			return flags;
		}

		mode_t getOpenMode() {
			mode_t mode = 0;
			if (mode_R_user.val() == 1) {
				mode |= S_IRUSR;
			}
			if (mode_W_user.val() == 1) {
				mode |= S_IWUSR;
			}
			if (mode_X_user.val() == 1) {
				mode |= S_IXUSR;
			}
			if (mode_R_group.val() == 1) {
				mode |= S_IRGRP;
			}
			if (mode_W_group.val() == 1) {
				mode |= S_IWGRP;
			}
			if (mode_X_group.val() == 1) {
				mode |= S_IXGRP;
			}
			if (mode_R_others.val() == 1) {
				mode |= S_IROTH;
			}
			if (mode_W_others.val() == 1) {
				mode |= S_IWOTH;
			}
			if (mode_X_others.val() == 1) {
				mode |= S_IXOTH;
			}
			
			if (mode == 0 && O_CREAT == 0) {
				// FIX ME!!!!!! ERROR
			}
			return mode;
			/*
			std::cout << "mode_R_user: " << mode_R_user.val() << std::endl;
			std::cout << "mode_W_user: " << mode_W_user.val() << std::endl;
			std::cout << "mode_X_user: " << mode_X_user.val() << std::endl;
			std::cout << "mode_R_group: " << mode_R_group.val() << std::endl;
			std::cout << "mode_W_group: " << mode_W_group.val() << std::endl;
			std::cout << "mode_X_group: " << mode_X_group.val() << std::endl;
			std::cout << "mode_R_others: " << mode_R_others.val() << std::endl;
			std::cout << "mode_W_others: " << mode_W_others.val() << std::endl;
			std::cout << "mode_X_others: " << mode_X_others.val() << std::endl;
			*/

		}

	public:
		OpenSystemCallTraceReplayModule(DataSeriesModule &source) : 
			SystemCallTraceReplayModule(source),
			given_pathname(series, "given_pathname"),
			flag_read_only(series, "flag_read_only"),
			flag_write_only(series, "flag_write_only"),
			flag_read_and_write(series, "flag_read_and_write"),
			flag_append(series, "flag_append"),
			flag_async(series, "flag_async"),
			flag_close_on_exec(series, "flag_close_on_exec"),
			flag_create(series, "flag_create"),
			flag_direct(series, "flag_direct"),
			flag_directory(series, "flag_directory"),
			flag_exclusive(series, "flag_exclusive"),
			flag_largefile(series, "flag_largefile"),
			flag_no_access_time(series, "flag_no_access_time"),
			flag_no_controlling_terminal(series, "flag_no_controlling_terminal"),
			flag_no_follow(series, "flag_no_follow"),
			flag_no_blocking_mode(series, "flag_no_blocking_mode"),
			flag_no_delay(series, "flag_no_delay"),
			flag_synchronous(series, "flag_synchronous"),
			flag_truncate(series, "flag_truncate"),
			mode_R_user(series, "mode_R_user"),
			mode_W_user(series, "mode_W_user"),
			mode_X_user(series, "mode_X_user"),
			mode_R_group(series, "mode_R_group"),
			mode_W_group(series, "mode_W_group"),
			mode_X_group(series, "mode_X_group"),
			mode_R_others(series, "mode_R_others"),
			mode_W_others(series, "mode_W_others"),
			mode_X_others(series, "mode_X_others") {

		}

		/* This function will be called by RowAnalysisModule once
		 * before the first operation is replayed. */
		void prepareForProcessing() {
			std::cout << "-----Open System Call Replayer starts to replay...-----" << std::endl;
		}

		/* This function will be called by RowAnalysisModule once
		 * for every system call operation in the trace file 
		 * being processed */
		void processRow() {
			char *pathname = (char *)given_pathname.val();
			uint32_t flags = getOpenFlags();
			mode_t mode = getOpenMode();

			if (verbose) {
				std::cout.precision(25);
				std::cout << "time called:" << std::fixed << time_called() << std::endl;
				std::cout << "pathname:" << pathname << std::endl;
				std::cout << "flags:" << flags << std::endl;
				std::cout << "mode:" << mode << std::endl;
			}
			
			int ret = open(pathname, flags, mode);
			if (ret == -1) {
				perror(pathname);
			} else {
				std::cout << given_pathname.val() << " is successfully opened..." << std::endl;
			}
		}

		/* This function will be called once by RowAnalysisModule after all
		 * system operations are being replayed.*/
		void completeProcessing() {
			std::cout << "-----Open System Call Replayer finished replaying...-----" << std::endl;
		}
};

class CloseSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
	private:
		/* DataSeries Close System Call Trace Fields */
		Int32Field descriptor_;

	public:
		CloseSystemCallTraceReplayModule(DataSeriesModule &source) : 
			SystemCallTraceReplayModule(source),
			descriptor_(series, "descriptor"){
		}

		/* This function will be called by RowAnalysisModule once
		 * before the first operation is replayed. */
		void prepareForProcessing() {
			std::cout << "-----Close System Call Replayer starts to replay...-----" << std::endl;
		}

		/* This function will be called by RowAnalysisModule once
		 * for every system call operation in the trace file 
		 * being processed */
		void processRow() {
			if (verbose) {
				std::cout.precision(25);
				std::cout << "time called:" << std::fixed << time_called() << std::endl;
				std::cout << "descriptor:" << descriptor_.val() << std::endl;
			}
			
			int ret = close(descriptor_.val());
			if (ret == -1) {
				perror("close");
			} else {
				std::cout << "fd " << descriptor_.val() << " is successfully closed..." << std::endl;
			}
		}

		/* This function will be called once by RowAnalysisModule after all
		 * system operations are being replayed.*/
		void completeProcessing() {
			std::cout << "-----Close System Call Replayer finished replaying...-----" << std::endl;
		}
};

class ReadSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
	private:
		/* DataSeries Read System Call Trace Fields */
		Int32Field descriptor_;
		Variable32Field data_read_;
		Int64Field bytes_requested_;
	public:
		ReadSystemCallTraceReplayModule(DataSeriesModule &source) : 
			SystemCallTraceReplayModule(source),
			descriptor_(series, "descriptor"), 
			data_read_(series, "data_read", Field::flag_nullable), 
			bytes_requested_(series, "bytes_requested") {
		}

		/* This function will be called by RowAnalysisModule once
		 * before the first operation is replayed. */
		void prepareForProcessing() {
			std::cout << "-----Read System Call Replayer starts to replay...-----" << std::endl;
		}

		/* This function will be called by RowAnalysisModule once
		 * for every system call operation in the trace file 
		 * being processed */
		void processRow() {
			if (verbose) {
				std::cout.precision(25);
				std::cout << "time called:" << std::fixed << time_called() << std::endl;
				std::cout << "descriptor:" << descriptor_.val() << std::endl;
			}
			char buffer[bytes_requested_.val()];
			int ret = read(descriptor_.val(), buffer, bytes_requested_.val());

			if (ret == -1) {
				perror("read");
			} else {
				std::cout << "read is executed successfully!";
			}
		}

		/* This function will be called once by RowAnalysisModule after all
		 * system operations are being replayed.*/
		void completeProcessing() {
			std::cout << "-----Read System Call Replayer finished replaying...-----" << std::endl;
		}
};

class WriteSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
	private:
		/* DataSeries Write System Call Trace Fields */
		Int32Field descriptor_;
		Variable32Field data_write_;
		Int64Field bytes_requested_;
	public:
		WriteSystemCallTraceReplayModule(DataSeriesModule &source) : 
			SystemCallTraceReplayModule(source),
			descriptor_(series, "descriptor"), 
			data_write_(series, "data_write", Field::flag_nullable), 
			bytes_requested_(series, "bytes_requested") {
		}

		/* This function will be called by RowAnalysisModule once
		 * before the first operation is replayed. */
		void prepareForProcessing() {
			std::cout << "-----Write System Call Replayer starts to replay...-----" << std::endl;
		}

		/* This function will be called by RowAnalysisModule once
		 * for every system call operation in the trace file 
		 * being processed */
		void processRow() {
			if (verbose) {
				std::cout.precision(25);
				std::cout << "time called:" << std::fixed << time_called() << std::endl;
				std::cout << "descriptor:" << descriptor_.val() << std::endl;
			}
			char buffer[bytes_requested_.val()];
			int ret = write(descriptor_.val(), buffer, bytes_requested_.val());
			
			if (ret == -1) {
				perror("write");
			} else {
				std::cout << "write is successfully with content: " << buffer;
			}
		}

		/* This function will be called once by RowAnalysisModule after all
		 * system operations are being replayed.*/
		void completeProcessing() {
			std::cout << "-----Write System Call Replayer finished replaying...-----" << std::endl;
		}
};


int main(int argc, char *argv[]) {
	bool finished_replaying = false;
	/*
	 * This reads all extents of open system call
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

	for (int i = 1; i < argc; i++) {
		for (unsigned int j = 0; j < type_index_modules.size(); j++) {
			type_index_modules[j]->addSource(argv[i]);
		}
	}

	std::vector<PrefetchBufferModule *> prefetch_buffer_modules;
	for (unsigned int i = 0; i < type_index_modules.size(); ++i) {
		PrefetchBufferModule *module = new PrefetchBufferModule(*(type_index_modules[i]), 64 * 1024 * 1024);
		prefetch_buffer_modules.push_back(module);
	}

	/* Parallel decompress and stats, 64MiB buffer */
	OpenSystemCallTraceReplayModule *open_replayer = new OpenSystemCallTraceReplayModule(*prefetch_buffer_modules[0]);
	CloseSystemCallTraceReplayModule *close_replayer = new CloseSystemCallTraceReplayModule(*prefetch_buffer_modules[1]);
	ReadSystemCallTraceReplayModule *read_replayer = new ReadSystemCallTraceReplayModule(*prefetch_buffer_modules[2]);
	WriteSystemCallTraceReplayModule *write_replayer = new WriteSystemCallTraceReplayModule(*prefetch_buffer_modules[3]);

	std::vector<SystemCallTraceReplayModule *> system_call_trace_replay_modules;
	system_call_trace_replay_modules.push_back(open_replayer);
	system_call_trace_replay_modules.push_back(close_replayer);
	system_call_trace_replay_modules.push_back(read_replayer);
	system_call_trace_replay_modules.push_back(write_replayer);

	SystemCallTraceReplayModule *execute_replayer = NULL;

	std::priority_queue<SystemCallTraceReplayModule*, std::vector<SystemCallTraceReplayModule*>, LessThanByTimeCalled> sorted_replayers;
	
	while (!finished_replaying) {
		for (unsigned int i = 0; i < system_call_trace_replay_modules.size(); ++i) {
			SystemCallTraceReplayModule *module = system_call_trace_replay_modules[i];
			if (module->getSharedExtent()) {
				sorted_replayers.push(module);
			}
		}

		if (sorted_replayers.empty()) {
			finished_replaying = true;
			break;
		}

		while(!sorted_replayers.empty()) {
			execute_replayer = sorted_replayers.top();
			sorted_replayers.pop();
			execute_replayer->execute();
			if (execute_replayer->has_more_trace()){
				sorted_replayers.push(execute_replayer);
			}
		}

	}	
	//while (replayer->getSharedExtent());
}
