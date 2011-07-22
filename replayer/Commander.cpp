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

#include "Commander.hpp"

Commander::Commander(std::string output_device,
		     void *bufferPtr,
		     bool verboseFlag)
	: fd(open(output_device.c_str(), O_RDWR | O_DIRECT)),
	  buffer(bufferPtr),
	  timeInitialized(false),
	  totalRead(0), totalWrite(0), totalData(0),
	  totalReadTime(0), totalWriteTime(0), lateOps(0), failedOps(0),
	  verbose(verboseFlag)
{
	if (fd < 0) {
		std::cout << "Warning: Cannot find device" << std::endl;
		exit(0);
	}
}

Commander::~Commander()
{
	if (!close(fd))
		std::cout << "Warning: Error closing file" << std::endl;
}

void Commander::waitUntil(uint64_t time)
{
	struct timespec sleeptime;

	uint64_t currentTime = timeNow();

	if (!timeInitialized) {
		/* wait until starting time */
		sleeptime.tv_sec = time / NANO_TIME_MULTIPLIER;
		sleeptime.tv_nsec = time % NANO_TIME_MULTIPLIER;
		nanosleep(&sleeptime, NULL);

		timeInitialized = true;
		startTime = time;
		prevTime = time;

		currentTime = timeNow();
		startRealTime = currentTime;
	}

	int64_t timeDelta = (time - startTime + startRealTime) - currentTime;

	if (timeDelta <= 0 && time != prevTime)	{
		lateOps++;
		if (verbose) {
			std::cout << "Operation did not execute in time."
				  << std::endl;
			std::cout << "\tTrace Time: " << time << " "
				  << "Time Difference: " << timeDelta
				  << std::endl;
		}
	} else {
		sleeptime.tv_sec = timeDelta / NANO_TIME_MULTIPLIER;
		sleeptime.tv_nsec = timeDelta % NANO_TIME_MULTIPLIER;
		nanosleep(&sleeptime, NULL);
	}

	prevTime = time;
}

void Commander::writeStats()
{
	std::cout << "IO Summary: "
		  << (timeNow() - startRealTime) << " tracetime, "
		  << totalData << " operationdata, "
		  << totalRead << " reads, "
		  << totalWrite << " writes, "
		  << totalReadTime << " readtime, "
		  << totalWriteTime << " writetime, "
		  << lateOps << " lateops, "
		  << failedOps << " failedops"
		  << std::endl;
}

uint64_t Commander::timeNow()
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return ((uint64_t) now.tv_sec) * NANO_TIME_MULTIPLIER + now.tv_nsec;
}

SynchronousCommander::SynchronousCommander(std::string output_device,
					   void *bufferPtr,
					   bool verboseFlag)
	: Commander(output_device, bufferPtr, verboseFlag) { }

void SynchronousCommander::execute(uint64_t operation,
				   uint64_t time,
				   uint64_t offset,
				   uint64_t size)
{
	waitUntil(time);

	uint64_t start = timeNow();

	lseek(fd, offset, SEEK_SET);

	if (operation == 0) {
		if (read(fd, buffer, size) == -1) {
			failedOps++;
			if (verbose)
				std::cout << time
					  << ": can't create read request!"
					  << std::endl;
		}

		totalReadTime += timeNow() - start;
		totalRead++;
	} else {
		if (write(fd, buffer, size) == -1) {
			failedOps++;
			if (verbose)
				std::cout << time
					  << ": can't create write request!"
					  << std::endl;
		}

		totalWriteTime += timeNow() - start;
		totalWrite++;
	}

	totalData += size;
}

void SynchronousCommander::cleanup()
{

}

/* AsynchronousCommander */

AsynchronousCommander::AsynchronousCommander(std::string output_device,
					     void *bufferPtr,
					     bool verboseFlag)
	:	Commander(output_device, bufferPtr, verboseFlag)
{

}

void AsynchronousCommander::execute(uint64_t operation,
				    uint64_t time,
				    uint64_t offset,
				    uint64_t size)
{
	aiocb * cb = new aiocb;
	memset(cb, 0, sizeof(aiocb));
	cb->aio_nbytes = size;
	cb->aio_fildes = fd;
	cb->aio_offset = offset;
	cb->aio_buf = buffer;
	control_block_queue.push(cb);

	waitUntil(time);

	if (operation == 0) {
		if (aio_read(cb) != 0) {
			failedOps++;
			if (verbose)
				std::cout << time
					  << ": can't create aio_read request!"
					  << std::endl;
		}
		totalRead++;
	} else {
		if (aio_write(cb) != 0) {
			failedOps++;
			if (verbose)
				std::cout << time
					  << ": can't create aio_write request!"
					  << std::endl;
		}
		totalWrite++;
	}

	checkControlBlockQueue();
	totalData += size;
}

void AsynchronousCommander::cleanup()
{
	while (checkControlBlockQueue());
}

bool AsynchronousCommander::checkControlBlockQueue()
{
	if (control_block_queue.empty())
		return 0;

	switch (aio_error(control_block_queue.front())) {
		case 0:
			delete control_block_queue.front();
			control_block_queue.pop();
			break;
		case EINPROGRESS:
			break;
		default:
			failedOps++;
			if (verbose)
				std::cout << "Warning: "
					  << "can't execute asychronous command"
					  << std::endl;
			delete control_block_queue.front();
			control_block_queue.pop();
			break;
	}

	if (control_block_queue.empty())
		return 0;
	else
		return 1;
}
