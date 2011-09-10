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

#include <iostream>
#include "ReplayStats.hpp"
#include "Commander.hpp"

void ReplayStats::printStats(std::ostream &out)
{
	uint64_t now = Commander::timeNow();
	out	<< "Running since: " << (now - replayStartWallclockTime) /
			NANO_TIME_MULTIPLIER << " (s), "
		<< "Current delay: " << (currentDelay - replayStartWallclockTime) /
			NANO_TIME_MULTIPLIER << " (s).\n"

		<< "Total: " << (readRecords + writeRecords) << " records, "
		<< (readsSubmitted + writesSubmitted) << " submitted, "
		<< (readsSucceeded + writesSucceeded) << " succeeded, "
		<< (readsSubmitted + writesSubmitted - readsSucceeded -
				writesSucceeded) << " failed, "
		<< (readsSubmitted + writesSubmitted - lateReads - lateWrites)
				<< " on-time, "
		<< lateReads + lateWrites << " delayed.\n"

		<< "Reads: " << readRecords << " records, "
		<< readsSubmitted << " submitted, "
		<< readsSucceeded << " succeeded, "
		<< (readsSubmitted - readsSucceeded) << " failed, "
		<< (readsSubmitted - lateReads) << " on-time, "
		<< lateReads << " delayed.\n"

		<< "Writes: " << writeRecords << " records, "
		<< writesSubmitted << " submitted, "
		<< writesSucceeded << " succeeded, "
		<< (writesSubmitted - writesSucceeded) << " failed, "
		<< (writesSubmitted - lateWrites) << " on-time, "
		<< lateWrites << " delayed." << std::endl;
}
