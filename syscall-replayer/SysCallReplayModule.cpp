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

private:
	/* DataSeries System Call Trace Fields */
	Variable32Field	given_pathname;
	BoolField	flag_read_only;
	BoolField	flag_write_only;
	BoolField	flag_read_and_write;
	BoolField	flag_append;
	BoolField	flag_async;
	BoolField	flag_close_on_exec;
	BoolField	flag_create;
	BoolField	flag_direct;
	BoolField	flag_directory;
	BoolField	flag_exclusive;
	BoolField	flag_largefile;
	BoolField	flag_no_access_time;
	BoolField	flag_no_controlling_terminal;
	BoolField	flag_no_follow;
	BoolField	flag_no_blocking_mode;
	BoolField	flag_no_delay;
	BoolField	flag_synchronous;
	BoolField	flag_truncate;
	BoolField	mode_R_user;
	BoolField	mode_W_user;
	BoolField	mode_X_user;
	BoolField	mode_R_group;
	BoolField	mode_W_group;
	BoolField	mode_X_group;
	BoolField	mode_R_others;
	BoolField	mode_W_others;
	BoolField	mode_X_others;
	bool verbose;

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
	SystemCallTraceReplayModule(DataSeriesModule &source) : 
		RowAnalysisModule(source),
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
		mode_X_others(series, "mode_X_others")
		{

	}

	/* This function will be called by RowAnalysisModule once
	 * before the first operation is replayed. */
	void prepareForProcessing() {
		std::cout << "Your awesome system call operation replayer starts to replay..." << std::endl;
	}

	/* This function will be called by RowAnalysisModule once
	 * for every system call operation in the trace file 
	 * being processed */
	void processRow() {
		char *pathname = (char *)given_pathname.val();
		uint32_t flags = getOpenFlags();
		mode_t mode = getOpenMode();
		if (verbose) {
			std::cout << "pathname:" << pathname << std::endl;
			std::cout << "flags:" << flags << std::endl;
			std::cout << "mode:" << mode << std::endl;
		}

		int ret = open(pathname, flags, mode);
		if (ret == -1) {
			perror(pathname);
		} else {
			std::cout << given_pathname.val() << " is successfully opened... :)" << std::endl;
		}
	}

	/* This function will be called once by RowAnalysisModule after all
	 * system operations are being replayed.*/
	void completeProcessing() {
		std::cout << "System Call Replayer finished replaying..." << std::endl;
	}
};

int main(int argc, char *argv[]) {
	/*
	 * This reads all extents of open system call
	 * operations from a set of DataSeries files.
	 */
	TypeIndexModule source("IOTTAFSL::Trace::Syscall::open");

	for (int i = 1; i < argc; i++)
		source.addSource(argv[i]);


	PrefetchBufferModule *prefetch = NULL;
	SystemCallTraceReplayModule *replayer = NULL;

	/* Parallel decompress and stats, 64MiB buffer */
	prefetch = new PrefetchBufferModule(source, 64 * 1024 * 1024);
	replayer = new SystemCallTraceReplayModule(*prefetch);

	while (replayer->getExtent());
}
