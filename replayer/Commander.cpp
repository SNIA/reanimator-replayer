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

	readWriteOp_t rwOp;

	switch (operation) {
	case OPERATION_READ:
		stats->readsSubmitted++;
		stats->readsSubmittedSize += size;
		rwOp = pread64;
		break;

	case OPERATION_WRITE:
		stats->writesSubmitted++;
		stats->writesSubmittedSize += size;
		rwOp = (readWriteOp_t) pwrite64;
		break;
	}

	stats->currentDelay = waitUntil(time);

	uint64_t start = timeNow();
	ssize_t ret = rwOp(fd, buffer, size, offset);
	uint64_t end = timeNow();

	switch (operation) {
	case OPERATION_READ:
		if (stats->currentDelay > 100)
			stats->lateReads++;

		if (ret != -1) { /* Successful read. */
			stats->readsSucceeded++;
			stats->readsSucceededSize += size;
			stats->readTimeSuccess += (end - start);
		} else { /* Failed read. */
			stats->readsFailedSize += size;
			stats->readTimeFailure += (end - start);
		}
		break;

	case OPERATION_WRITE:
		if (stats->currentDelay > 100)
			stats->lateWrites++;

		if (ret != -1) { /* Successful write. */
			stats->writesSucceeded++;
			stats->writesSucceededSize += size;
			stats->writeTimeSuccess += (end - start);
		} else { /* Failed write. */
			stats->writesFailedSize += size;
			stats->writeTimeFailure += (end - start);
		}
		break;
	}

	if (ret == -1 && verbose) {
		std::cerr << "Failed " << ((operation == OPERATION_READ) ? "read" :
			"write") << ": " << time << "," << offset << "," << size << ": "
			<< strerror(errno) << " (errno = " << errno << ").\n";

	}
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
	/* TODO: Handle out of memory */
	aio_op_t *op = (aio_op_t *) calloc(1, sizeof(aio_op_t));
	op->opcode = operation;
	op->cb.aio_nbytes = size;
	op->cb.aio_fildes = fd;
	op->cb.aio_offset = offset;
	op->cb.aio_buf = buffer;

	control_block_queue.push(op);

	ssize_t (*aio_op)(aiocb64*) = (operation == OPERATION_READ) ?
			aio_read64 : aio_write64;

	stats->currentDelay = waitUntil(time);

	int ret = aio_op(&op->cb);

	switch (operation) {
	case OPERATION_READ:
		if (ret != -1) { /* Successfully submitted read. */
			stats->readsSubmitted++;
			stats->readsSubmittedSize += size;
		} else { /* Failed to submit this read. */
			stats->readsFailedSize += size;
		}
		break;

	case OPERATION_WRITE:
		if (ret != -1) { /* Successfully submitted write. */
			stats->writesSubmitted++;
			stats->writesSubmittedSize += size;
		} else { /* Failed to submit this write. */
			stats->writesFailedSize += size;
		}
		break;
	}

	if (ret == -1 && verbose) {
		std::cerr << "Failed " << ((operation == OPERATION_READ) ? "read" :
			"write") << ": " << time << "," << offset << "," << size << ": "
			<< strerror(errno) << " (errno = " << errno << ").\n";
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

	aio_op_t *op = control_block_queue.front();
	int error = aio_error64(&op->cb);

	switch (error) {
		case 0:
			switch (op->opcode) {
			case OPERATION_READ:
				stats->readsSucceeded++;
				stats->readsSucceededSize += op->cb.aio_nbytes;
				break;
			case OPERATION_WRITE:
				stats->writesSucceeded++;
				stats->writesSucceededSize += op->cb.aio_nbytes;
				break;
			}

			delete control_block_queue.front();
			control_block_queue.pop();
			break;
		case EINPROGRESS:
			break;
		default:
			switch (op->opcode) {
			case OPERATION_READ:
				stats->readsFailedSize += op->cb.aio_nbytes;
				break;
			case OPERATION_WRITE:
				stats->writesFailedSize += op->cb.aio_nbytes;
				break;
			}
			if (verbose) {
				/* FIXME: Should LIO_NOP be handled separately here? */
				std::cerr << "Failed " << ((op->opcode == OPERATION_READ) ?
					"read" : "write") << ": " << op->cb.aio_offset << "," <<
					op->cb.aio_nbytes << ": " << strerror(op->cb.__error_code)
					<< " (errno = " << op->cb.__error_code << ").\n";
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
