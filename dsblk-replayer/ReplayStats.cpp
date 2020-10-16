/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
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
