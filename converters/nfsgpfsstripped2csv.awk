#  Copyright (c) 2012 Vasily Tarasov
#  Copyright (c) 2012 Dean Hildebrand
#  Copyright (c) 2012 Erez Zadok
#  Copyright (c) 2012 Stony Brook University
#  Copyright (c) 2012 The Research Foundation of SUNY
#  Copyright (c) IBM Research - Almaden
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# nfsgpfsstripped2csv.awk converts GPFS traces produced by mmtrace to CSV format.
# Is ivoked by nfsgpfsstripped2ds.sh.
#


BEGIN {
	OFS=","
	OFMT="%d"
}

next_line_is_first_trace_record == 1 {
	first_record_seconds = $1
	next_line_is_first_trace_record = 0
}

$1 ==  "----------" {
	next_line_is_first_trace_record = 1
}

/TRACE_VNOP: READ:/ {
	print "read_request", seconds_to_reltfracs($1), $8, $14 + 0, $16, ""
}

/TRACE_VNOP: WRITE:/ {
	print "write_request", seconds_to_reltfracs($1), $8, $14 + 0, $16, "", "", "", ""
}

END{}

function seconds_to_reltfracs(seconds) {
	return int((seconds - first_record_seconds) * 2^32)
}
