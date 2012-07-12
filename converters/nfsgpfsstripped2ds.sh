#!/bin/bash

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
# nfsgpfsstripped2ds.sh converts GPFS traces produced by mmtrace to DataSeries format.
# DataSeries specification was selected so that it resembles NFS trace because
# we are actually interested in the NFS trace, and use GPFS only as a trace
# collecting facility.
#
# The sequence of operations performed by this script is the following:
# 1. Calls nfsgpfsstripped2csv.awk script to converts the trace
#    produced by GPFS's mmtrace to convert it to a CSV file.
#
# 2. Use csv2ds-extra to convert CSV to Dataseries format.
#    Specification of the fields in the input CSV file and the mapping
#    between CSV fields to Dataseries fields are provided by
#    specstrings/nfstrace-gpfsstripped.spec and tables/nfs_gpfsstripped_fields.table
#    files, in order.
#
# Usage: nfsgpfsstripped2ds.sh <pcap_file> <outputfile>
#

if [ $# -ne 2 ]; then
	echo "Usage: $0 <gpfs_trace_file> <outputfile>"
	exit 1
fi

INPUTFILE=$1
OUTPUTFILE=$2

TABLEFILE="./tables/nfs_gpfsstripped_fields.table"
SPECSTRINGFILE="./specstrings/nfstrace-gpfsstripped.spec"

if [ ! -e csv2ds-extra ]; then
	echo "csv2ds-extra binary is not found. Maybe make it?"
	exit 1
fi

if [ ! -e $TABLEFILE ]; then
	echo "NFS-GPFS-STRIPPED-trace table file is not found! Please put it in $TABLEFILE"
	exit 1
fi

if [ ! -e $SPECSTRINGFILE ]; then
	echo "Specification string for nfs-gpfs-stripped-trace is not found! Please put it in \
		$SPECSTRINGFILE"
	exit 1
fi

if [ -z "$INPUTFILE" ]; then
	echo "You must specify input file as a 1st argument!"
	exit 1
fi

if [ -z "$OUTPUTFILE" ]; then
	echo "You must specify output file as a 2nd argument!"
	exit 1
fi

echo "Converting trace to CSV..."

TEMPFILE=`mktemp`
echo "Using temporary file $TEMPFILE"

awk --non-decimal-data -f nfsgpfsstripped2csv.awk < "$INPUTFILE" > "$TEMPFILE"

echo "Converting to DataSeries..."
./csv2ds-extra -q $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $TEMPFILE

echo "Cleaning up"
rm -f $TEMPFILE
