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
		<< "Current delay: " << (currentDelay / (int64_t) NANO_TIME_MULTIPLIER)
		      << " (s), "
		<< "Last record timestamp : " << lastRecordTimestamp << " (Tfracs).\n"

		<< "Total: " << (readRecords + writeRecords) << " records, "
		<< (readsSubmitted + writesSubmitted) << " submitted, "
		<< (readsSucceeded + writesSucceeded) << " succeeded, "
		<< (readsPending + writesPending) << " pending, "
		<< (readsSubmitted + writesSubmitted - readsSucceeded - writesSucceeded
		      - readsPending - writesPending) << " failed, "
		<< (readsSubmitted + writesSubmitted - lateReads - lateWrites)
				<< " on-time, "
		<< lateReads + lateWrites << " delayed, "
		<< (readsLateTime + writesLateTime) << " (ns) total late, "
		<< (readsEarlyTime + writesEarlyTime) << " (ns) total early.\n"

		<< "Reads: " << readRecords << " records, "
		<< readsSubmitted << " submitted, "
		<< readsSucceeded << " succeeded, "
		<< readsPending << " pending, "
		<< (readsSubmitted - readsSucceeded - readsPending) << " failed, "
		<< (readsSubmitted - lateReads) << " on-time, "
		<< lateReads << " delayed, "
		<< readsLateTime << " (ns) total late, "
		<< readsEarlyTime << " (ns) total early.\n"

		<< "Writes: " << writeRecords << " records, "
		<< writesSubmitted << " submitted, "
		<< writesSucceeded << " succeeded, "
		<< writesPending << " pending, "
		<< (writesSubmitted - writesSucceeded - writesPending) << " failed, "
		<< (writesSubmitted - lateWrites) << " on-time, "
		<< lateWrites << " delayed, "
        << writesLateTime << " (ns) total late, "
        << writesEarlyTime << " (ns) total early." << std::endl;
}
