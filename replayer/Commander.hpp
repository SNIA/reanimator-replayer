/*
 * Copyright (c) 2011 Jack Ma
 * Copyright (c) 2011 Vasily Tarasov
 * Copyright (c) 2011 Koundinya Santhosh Kumar
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

/*
 *	Commander is an abstract object used by ReplayModule to execute
 *	operations.
 *	It provides the functions waitUntil(), execute(), and cleanup().
 *
 *	SynchronousCommander and AsynchronousCommander inherits Commander and
 *	implements execute() and cleanup() with synchronous and asynchronous
 *	read/write, respectively.
 *
 *	INITIALIZATION AND USAGE
 *	As part of the abstraction design, a ReplayModule should declare a
 *	Commander, then initialize it with SynchronousCommander or
 *	AsynchronousCommander. Then ReplayModule should execute each operation
 *	with execute().
 *
 *	waitUntil() halts the program until the real clock time matches the
 *	trace starting time plus the specified time in nanoseconds. The trace
 *	starting time is automatically initialized when waitUntil is called for
 *	the first time.
 *
*/

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <aio.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <queue>

#define NANO_TIME_MULTIPLIER (1000UL * 1000UL * 1000UL)

class Commander {

public:
	Commander(std::string output_device, void *bufferPtr, bool verboseFlag);

	~Commander();

	/* Wait until specified real time, then return */
	void waitUntil(uint64_t time);

	/* Execute an operation with specified time, offset, and size */
	virtual void execute(uint64_t operation,
			     uint64_t time,
			     uint64_t offset,
			     uint64_t size) = 0;

	/* Execute cleanup procedures, if any */
	virtual void cleanup() = 0;

	/* Write out trace statistics to cout */
	void writeStats();

protected:
	/* Return the current time in uint64_t in nanoseconds */
	uint64_t timeNow();

	int fd;
	void *buffer;
	bool timeInitialized;
	uint64_t startTime, startRealTime, prevTime;

	uint64_t totalRead, totalWrite, totalData;
	uint64_t totalReadTime, totalWriteTime;
	uint64_t lateOps, failedOps;

	bool verbose;
};

class SynchronousCommander : public Commander {
public:
	SynchronousCommander(std::string output_device,
			     void *bufferPtr,
			     bool verboseFlag);

	void execute(uint64_t operation,
			     uint64_t time,
			     uint64_t offset,
			     uint64_t size);

	void cleanup();

};

class AsynchronousCommander : public Commander {
public:
	AsynchronousCommander(std::string output_device,
			      void *bufferPtr,
			      bool verboseFlag);

	void execute(uint64_t operation,
			     uint64_t time,
			     uint64_t offset,
			     uint64_t size);

	void cleanup();

	/* Clean out the first/oldest item in control_block_queue if finished.
		Return true if control_block is not empty after operation.
		Else return false. */
	bool checkControlBlockQueue();

private:
	std::queue<aiocb*> control_block_queue;

};

