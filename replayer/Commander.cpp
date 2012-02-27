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

#include "Commander.hpp"
#include "aio_sync_flag.h"

#include <iostream>
#include <string>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libaio.h>

#include <ctime>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cassert>

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

/*
 * time variable MUST be in nano seconds
 */
int64_t Commander::waitUntil(uint64_t time)
{
	uint64_t currentTime = timeNow();

	/* Initialize times, this happens when the first request is replayed */
	if (!timeInitialized) {
		timeInitialized = true;
		traceRecordStartTime = time;
		stats->replayStartWallclockTime = replayStartWallclockTime =
				currentTime;
		return 0; /* Don't wait if this is the first record */
	}

	int64_t timeDelta = (int64_t)(time - traceRecordStartTime) -
				(currentTime - replayStartWallclockTime);

	/*
	 * If timeDelta is sufficiently large, sleep.
	 * Make sure that if we are woken up too early - sleep more.
	 * XXX: Replace the hard-coded value below
	 *
	 */
	if (timeDelta > 100) {
		struct timespec sleeptime, remtime;
		int rc;

		sleeptime.tv_sec = timeDelta / NANO_TIME_MULTIPLIER;
		sleeptime.tv_nsec = timeDelta % NANO_TIME_MULTIPLIER;

		for (;;) {
			rc = nanosleep(&sleeptime, &remtime);
			if (rc == 0)
				break;

			if (errno != EINTR) {
				std::cerr << "sleep failed with errno = " << errno << "\n";
				break;
			}

			/*
			 * XXX: Check that remtime is lareger than 100 
			 */
			sleeptime = remtime;
		}
	}

	/*
	 * Return how late this request was. If it came earlier,
	 * then negative time is returned.
	 */
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
				   uint64_t size,
				   bool sync)
{
	assert(!(offset % SECTOR_SIZE));
	typedef ssize_t (*readWriteOp_t)(int fd, void *buf, size_t count,
			__off64_t offset);

	readWriteOp_t rwOp = NULL;

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

	default:
	    assert(0 && "Illegal Operation.");
	    break;
	}

	stats->currentDelay = waitUntil(time);

	uint64_t start = timeNow();
	ssize_t ret = rwOp(fd, buffer, size, offset);
	uint64_t end = timeNow();

	switch (operation) {
	case OPERATION_READ:
		if (stats->currentDelay > 0) {
			stats->lateReads++;
			stats->readsLateTime += stats->currentDelay;
		} else if (stats->currentDelay < 0) {
		    stats->readsEarlyTime += stats->currentDelay;
		}

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
		if (stats->currentDelay > 0) {
			stats->lateWrites++;
			stats->writesLateTime += stats->currentDelay;
		} else if (stats->currentDelay < 0) {
		    stats->writesEarlyTime += stats->currentDelay;
		}

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

/****************************************************
 ************ AsynchronousCommander *****************
 ****************************************************/

AsynchronousCommander::AsynchronousCommander(std::string output_device,
						void *bufferPtr,
						bool verboseFlag,
						ReplayStats *stats,
						int maxevents_)
	: Commander(output_device, bufferPtr, verboseFlag, stats),
	  io_ctx(NULL), maxevents(maxevents_)
{
	int rc = io_setup(maxevents, &io_ctx);
	if (rc)
		assert(false);

	/*
	 * For disk drives, request service times are in the order of
	 * milli-seconds.  Choose the sleep
	 * time to be a tenth of the average service time.
	 */
	ios_st.tv_sec = 0;
	ios_st.tv_nsec = 100000;
}

void AsynchronousCommander::execute(uint64_t operation,
				    uint64_t time,
				    uint64_t offset,
				    uint64_t size,
				    bool sync)
{
	assert(!(offset % SECTOR_SIZE));

	/*
	 * Allocating and initializing I/O Control Block
	 * XXX: pre-allocate a pool of iocbps
	 */
	struct iocb *iocbp = (struct iocb *)malloc(sizeof(struct iocb));
	if (!iocbp)
		assert(false);

	switch (operation) {
	case OPERATION_READ:
		io_prep_pread(iocbp, fd, buffer, size, offset);
		break;
	case OPERATION_WRITE:
		io_prep_pwrite(iocbp, fd, buffer, size, offset);
		break;
	default:
		assert(false);
		break;
	}

	if (sync)
		io_set_sync_flag(iocbp);

	struct iocb *ios[] = {iocbp};

	/*
	 * Wait till the request time comes. Simulatneously
	 * save the wait time for statistics.
	 */
	stats->currentDelay = waitUntil(time);

	/*
	 * Submit I/O. In case -EAGAIN is returned, check for
	 * completed events, this should free some in-kernel
	 * resources.
	 */
	int rc;
	while ((rc = io_submit(io_ctx, 1, ios)) && rc == -EAGAIN) {
		int eventsCompleted = checkSubmittedEvents();
		/*
		 * If no completed events, just wait a bit
		 */
		if (!eventsCompleted)
			nanosleep(&ios_st, NULL);
	}

	switch (operation) {
	case OPERATION_READ:
		if (stats->currentDelay > 0) {
			stats->lateReads++;
			stats->readsLateTime += stats->currentDelay;
		} else if (stats->currentDelay < 0) {
			stats->readsEarlyTime += stats->currentDelay;
		}

		if (rc == 1) { /* Successfully submitted read */
			stats->readsSubmitted++;
			stats->readsSubmittedSize += size;
			stats->readsPending++;
		} else { /* Failed to submit this read */
			stats->readsFailedSize += size;
		}
		break;
	case OPERATION_WRITE:
		if (stats->currentDelay > 0) {
			stats->lateWrites++;
			stats->writesLateTime += stats->currentDelay;
		} else if (stats->currentDelay < 0) {
			stats->writesEarlyTime += stats->currentDelay;
		}

		if (rc == 1) { /* Successfully submitted write. */
			stats->writesSubmitted++;
			stats->writesPending++;
			stats->writesSubmittedSize += size;
		} else { /* Failed to submit this write. */
			stats->writesFailedSize += size;
		}
		break;
	}

	if (rc != 1 && verbose) {
		std::cerr << "Failed "
			<< ((operation == OPERATION_READ) ? "read" : "write")
			<< ": " << time << "," << offset << "," << size << ": "
			<< strerror(errno) << " (errno = " << -rc << ").\n";
	}

	checkSubmittedEvents();
}

void AsynchronousCommander::cleanup()
{
	struct timespec wait_time = {1, 0}; /* 1 second sleep time */

	while (stats->readsPending || stats->writesPending) {
		checkSubmittedEvents();
		nanosleep(&wait_time, NULL);
	}
}

int AsynchronousCommander::checkSubmittedEvents()
{
	int eventsCompleted = 0;
	struct io_event event;

	/*
	 * XXX: we can get multiple events with a single call,
	 * without calling io_getevents() multiple times.
	 */
	while (io_getevents(io_ctx, 0, 1, &event, NULL) == 1) {
		switch (event.obj->aio_lio_opcode) {
		case IO_CMD_PREAD:
			stats->readsPending--;
			if (event.res == event.obj->u.c.nbytes) {
				stats->readsSucceeded++;
				stats->readsSucceededSize += event.obj->u.c.nbytes;
			} else {
		 		stats->readsFailedSize += event.obj->u.c.nbytes;
			}
			break;
		case IO_CMD_PWRITE:
			stats->writesPending--;
			if (event.res == event.obj->u.c.nbytes) {
				stats->writesSucceeded++;
				stats->writesSucceededSize += event.obj->u.c.nbytes;
			} else {
				stats->writesFailedSize += event.obj->u.c.nbytes;
			}
			break;
		default:
			assert(false);
		break;
		}
		eventsCompleted++;
		free(event.obj);
	}

	return eventsCompleted;
}

ReplayStats *Commander::getReplayStats() const
{
	return stats;
}
