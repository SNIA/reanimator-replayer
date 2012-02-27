/*
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
#ifndef COMMANDER_HPP_
#define COMMANDER_HPP_

#include <string>
#include <cstdint>
#include <queue>
#include <libaio.h>

#define OPERATION_READ		0
#define OPERATION_WRITE		1
#define SECTOR_SIZE			512

#define NANO_TIME_MULTIPLIER (1000ULL * 1000ULL * 1000ULL)

#include "ReplayStats.hpp"

class Commander {

public:
	Commander(std::string output_device, void *bufferPtr, bool verboseFlag,
			ReplayStats *stats);

	~Commander();

	/* Wait until specified relative record time, then return
	 *
	 * @time: The timestamp associated with the record, in nanos.
	 *
	 * @return: The actual time spent waiting, in nanos. A negative value
	 * indicates that we returned ahead of time. */
	int64_t waitUntil(uint64_t time);

	/* Execute an operation with specified time, offset, and size.
	 * FIXME: Use enumerators for operation codes. */
	virtual void execute(uint64_t operation,
			     uint64_t time,
			     uint64_t offset,
			     uint64_t size,
			     bool sync) = 0;

	/* Execute cleanup procedures, if any */
	virtual void cleanup() = 0;

	/* Return the current time in uint64_t in nanoseconds */
	static uint64_t timeNow();

	/* Return the replay statistics object associated with this commander. */
	ReplayStats *getReplayStats() const;

protected:

	int fd;
	void *buffer;
	bool timeInitialized;
	uint64_t traceRecordStartTime; /* The first time in the trace record */
	/* The wall clock time when we started replaying the trace */
	uint64_t replayStartWallclockTime;
	bool verbose;
	ReplayStats *stats;
};

class SynchronousCommander : public Commander {
public:
	SynchronousCommander(std::string output_device,
			     void *bufferPtr,
			     bool verboseFlag,
			     ReplayStats *stats);

	void execute(uint64_t operation,
			     uint64_t time,
			     uint64_t offset,
			     uint64_t size,
			     bool sync);

	void cleanup();

	virtual ~SynchronousCommander() {}
};

class AsynchronousCommander : public Commander {
public:
	AsynchronousCommander(std::string output_device,
			      void *bufferPtr,
			      bool verboseFlag,
			      ReplayStats *stats,
			      int maxevents_);

	void execute(uint64_t operation,
			     uint64_t time,
			     uint64_t offset,
			     uint64_t size,
			     bool sync);

	void cleanup();

	virtual ~AsynchronousCommander() {}

private:
	int checkSubmittedEvents();

	io_context_t io_ctx;
	int maxevents;
	struct timespec ios_st;
};

#endif /* COMMANDER_HPP_ */
