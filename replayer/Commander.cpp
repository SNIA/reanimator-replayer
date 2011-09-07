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
#define _LARGEFILE64_SOURCE	1
#define OPERATION_READ		0

#include "Commander.hpp"

#include <iostream>
#include <string>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <aio.h>

#include <ctime>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <queue>

#include <stdexcept>

Commander::Commander(std::string output_device,
		     void *bufferPtr,
		     bool verboseFlag,
		     ReplayStats *stats)
	: fd(-1),
	  buffer(bufferPtr),
	  timeInitialized(false),
	  verbose(verboseFlag),
	  stats(stats)
{
	fd = open(output_device.c_str(), O_RDWR | O_DIRECT | O_LARGEFILE);
	if (fd < 0) {
		std::stringstream msg;
		msg << "Cannot open device '" << output_device << "': " <<
				strerror(errno) << " (errno = " << errno << ").";
		throw std::runtime_error(msg.str());
	}

	if (!bufferPtr) {
		throw std::invalid_argument("NULL buffer");
	}

	if (!stats) {
		throw std::invalid_argument("NULL stats");
	}
}

Commander::~Commander()
{
	if (close(fd))
		std::cerr << "Warning: Error closing device (errno = " << errno <<
			").\n";
}

int64_t Commander::waitUntil(uint64_t time)
{
	uint64_t currentTime = timeNow();

	if (!timeInitialized) {
		timeInitialized = true;
		traceRecordStartTime = time;
		stats->replayStartWallclockTime = replayStartWallclockTime =
				currentTime;
		return 0; /* Don't wait if this is the first record */
	}

	int64_t timeDelta = (time - traceRecordStartTime) -
			(currentTime - replayStartWallclockTime);

	/* FIXME: Replace the hard-coded value below with (some constant times)
	 * the clock resolution. */
	if (timeDelta > 100) {
		struct timespec sleeptime;
		sleeptime.tv_sec = timeDelta / NANO_TIME_MULTIPLIER;
		sleeptime.tv_nsec = timeDelta % NANO_TIME_MULTIPLIER;
		nanosleep(&sleeptime, NULL);
	} else {
		stats->lateOps++;
	}

	return -timeDelta;
}

uint64_t Commander::timeNow()
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return ((uint64_t) now.tv_sec) * NANO_TIME_MULTIPLIER + now.tv_nsec;
}

SynchronousCommander::SynchronousCommander(std::string output_device,
					   void *bufferPtr,
					   bool verboseFlag,
					   ReplayStats *stats)
	: Commander(output_device, bufferPtr, verboseFlag, stats) { }

void SynchronousCommander::execute(uint64_t operation,
				   uint64_t time,
				   uint64_t offset,
				   uint64_t size)
{
	typedef ssize_t (*readWriteOp_t)(int fd, void *buf, size_t count,
			__off64_t offset);

	readWriteOp_t rwOp = (operation == OPERATION_READ) ?
			pread64 : (readWriteOp_t)pwrite64;

	stats->currentDelay = waitUntil(time);
	uint64_t start = timeNow();

	if (rwOp(fd, buffer, size, offset) != -1) {
		uint64_t elapsedTime = timeNow() - start;
		if (operation == OPERATION_READ) {
			stats->totalReadTime += elapsedTime;
		}  else {
			stats->totalWriteTime += elapsedTime;
		}
		stats->totalData += size;
	}
	else {
		stats->failedOps++;
		if (verbose) {
			std::cerr << "Failed " << ((operation == OPERATION_READ) ? "read" :
				"write") << ":" << time << "," << offset << "," << size << ": "
				<< strerror(errno) << "(errno = " << errno << ").\n";
		}
	}

	(operation == OPERATION_READ) ? stats->totalRead++ : stats->totalWrite++;
}

void SynchronousCommander::cleanup()
{

}

/* AsynchronousCommander */

AsynchronousCommander::AsynchronousCommander(std::string output_device,
					     void *bufferPtr,
					     bool verboseFlag,
					     ReplayStats *stats)
	:	Commander(output_device, bufferPtr, verboseFlag, stats)
{

}

void AsynchronousCommander::execute(uint64_t operation,
				    uint64_t time,
				    uint64_t offset,
				    uint64_t size)
{
	aiocb64 * cb = new aiocb64;
	memset(cb, 0, sizeof(aiocb64));
	cb->aio_nbytes = size;
	cb->aio_fildes = fd;
	cb->aio_offset = offset;
	cb->aio_buf = buffer;
	control_block_queue.push(cb);
	ssize_t (*readWriteOp)(aiocb64*) =
			(operation == OPERATION_READ) ? aio_read64 : aio_write64;

	waitUntil(time);
	if (readWriteOp(cb) != -1) {
		stats->totalData += size;
	} else {
		stats->failedOps++;
		if (verbose) {
			std::cerr << "Failed " << ((operation == OPERATION_READ) ? "read" :
				"write") << "," << time << "," << offset << "," << size << ": "
				<< strerror(errno) << "(errno = " << errno << ").\n";
		}
	}

	checkControlBlockQueue();
}

void AsynchronousCommander::cleanup()
{
	while (checkControlBlockQueue());
}

bool AsynchronousCommander::checkControlBlockQueue()
{
	if (control_block_queue.empty())
		return 0;

	int error;
	aiocb64 *cb;

	switch ((error=aio_error((aiocb*)(cb=control_block_queue.front())))) {
		case 0:
			delete control_block_queue.front();
			control_block_queue.pop();
			break;
		case EINPROGRESS:
			break;
		default:
			stats->failedOps++;
			if (verbose) {
				/* FIXME: Should LIO_NOP be handled separately here? */
				std::cerr << "Failed " << ((cb->aio_lio_opcode == LIO_READ) ?
					"read" : "write") << "," << cb->aio_offset << "," <<
					cb->aio_nbytes << ": " << strerror(cb->__error_code) <<
					"(errno = " << cb->__error_code << ").\n";
			}
			delete control_block_queue.front();
			control_block_queue.pop();
			break;
	}

	if (control_block_queue.empty())
		return false;
	else
		return true;
}

ReplayStats *Commander::getReplayStats() const
{
	return stats;
}

void Commander::writeStats()
{
	uint64_t now = Commander::timeNow();
	std::cout << "Running since: " <<
			(now - stats->replayStartWallclockTime) /
			NANO_TIME_MULTIPLIER << " (s)"
			<< ", last trace record: " << stats->lastReplayedRecordTimestamp
			<< " (Tfracs)"
			<< ", current delay: " << stats->currentDelay /
			NANO_TIME_MULTIPLIER << " (s)"
			<< "\nReads: " << stats->totalRead
			<< ", writes: " << stats->totalWrite
			<< ", late ops: " << stats->lateOps
			<< ", failed ops: " << stats->failedOps
			<< "\nread time: " << stats->totalReadTime /
			NANO_TIME_MULTIPLIER
			<< ", write time: " << stats->totalWriteTime /
			NANO_TIME_MULTIPLIER << std::endl;
}
