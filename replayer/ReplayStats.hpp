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

#ifndef REPLAY_STATS_HPP_
#define REPLAY_STATS_HPP_

#include <iostream>

class ReplayStats
{
public:
	ReplayStats(): readRecords(0), readsSubmitted(0), readsSucceeded(0),
		lateReads(0), writeRecords(0), writesSubmitted(0), writesSucceeded(0),
		lateWrites(0), readRecordsSize(0), readsSubmittedSize(0),
		readsSucceededSize(0), readsFailedSize(0), writeRecordsSize(0),
		writesSubmittedSize(0), writesSucceededSize(0), writesFailedSize(0),
		readTimeSuccess(0), readTimeFailure(0), writeTimeSuccess(0),
		writeTimeFailure(0), currentDelay(0), replayStartWallclockTime(0),
		lastRecordTimestamp(0) {}

	void printStats(std::ostream &out);

public: /* Internal state is left public for convenience. */

	/* Counts. */
	uint64_t readRecords, readsSubmitted, readsSucceeded, lateReads;
	uint64_t writeRecords, writesSubmitted, writesSucceeded, lateWrites;

	/* Size of transfer, in bytes. */
	uint64_t readRecordsSize, readsSubmittedSize, readsSucceededSize,
		readsFailedSize;
	uint64_t writeRecordsSize, writesSubmittedSize, writesSucceededSize,
		writesFailedSize;

	/* Time spent making requests, in nanos. Not used in the 'async' mode. */
	uint64_t readTimeSuccess, readTimeFailure;
	uint64_t writeTimeSuccess, writeTimeFailure;

	/* Use a signed quantity for current delay. We can be ahead sometimes. */
	int64_t currentDelay;

	/* Trace start (wall clock) time in nanos. */
	uint64_t replayStartWallclockTime;

	/* The timestamp associated with the last read record, in Tfracs. Useful to
	 * locate the  (last) row read from the DataSeries file. */
	uint64_t lastRecordTimestamp;
};

#endif /* REPLAY_STATS_HPP_ */
