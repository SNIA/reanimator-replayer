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

class ReplayStats
{
public:
	ReplayStats(): totalRead(0), totalWrite(0), totalData(0), totalReadTime(0),
	totalWriteTime(0), lateOps(0), failedOps(0), replayStartWallclockTime(0),
	rowsOutOfRange(0), lastReplayedRecordTimestamp(0){}

public: /* Internal state is left public for convenience. */
	uint64_t totalRead, totalWrite, totalData;
	uint64_t totalReadTime, totalWriteTime; /* All in nanos */
	/* Use a signed quantity for current delay. We can be ahead sometimes. */
	int64_t currentDelay;
	uint64_t lateOps, failedOps;
	/* Trace start (wall clock) time in nanos */
	uint64_t replayStartWallclockTime;
	uint64_t rowsOutOfRange;
	uint64_t lastReplayedRecordTimestamp; /* In Tfracs. */
};

#endif /* REPLAY_STATS_HPP_ */
