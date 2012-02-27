#!/bin/bash

#
# Copyright (c) 2011-2012 Jack Ma
# Copyright (c) 2011-2012 Santhosh Kumar Koundinya
# Copyright (c) 2011-2012 Vasily Tarasov
# Copyright (c) 2011-2012 Erez Zadok
# Copyright (c) 2011-2012 Geoff Kuenning
# Copyright (c) 2011-2012 Stony Brook University
# Copyright (c) 2011-2012 Harvey Mudd College
# Copyright (c) 2011-2012 The Research Foundation of SUNY
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

#
# blktrace2ds.sh converts traces in blktrace format to Dataseries format.
# The sequence of operations performed by this script is the following:
# 1. Run blkparse to create CSV file in an appropriate format
# 2. Run pre-processor that performs unit conversion
#    (doing this in C++, not in bash, increases the speed drastically)
# 3. Use csv2ds-extra to convert CSV to Dataseries format.
#    Specification of the fields in the input CSV file and the mapping
#    between CSV fields to Dataseries fields are provided by
#    specstrings/blktrace.spec and tables/snia_block_fields.table
#    files, in order.
#

#
# Usage: blktrace2ds.sh <outputfile> <blktrace_base_file>
#

if [ $# -ne 2 ]; then
	echo "Usage: $0 <blktrace_base_file> <outputfile> "
	exit 1
fi

INPUTFILE=$1
OUTPUTFILE=$2

TABLEFILE=./tables/snia_block_fields.table
SPECSTRINGFILE=./specstrings/blktrace.spec

if [ ! -e csv2ds-extra ]; then
	echo "csv2ds-extra binary is not found. Maybe make it?"
	exit 1
fi

if [ ! -e pre-processor ]; then
	echo "pre-processor binary is not found. Maybe make it?"
	exit 1
fi

if [ ! -e $TABLEFILE ]; then
	echo "Block-trace table file is not found! Please put it in $TABLEFILE"
	exit 1
fi

if [ ! -e $SPECSTRINGFILE ]; then
	echo "Specification string for blktrace is not found! Please put it in \
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

echo "Parsing blktrace file with blkparse..."

TEMPFILE=`mktemp`
echo "Using temporary file $TEMPFILE"

blkparse -q -f '' -F Q,"read_write,%T%t,%p,%M%m,%d,%S,%N\n" -o $TEMPFILE $INPUTFILE
RET=$?
if [ $RET -ne 0 ]; then
	echo "blkparse failed with error code $RET"
	rm -f $TEMPFILE
	exit $RET
fi

echo "Pre-processing..."

TEMPFILE2=`mktemp`
echo "Using temporary file $TEMPFILE2"

./pre-processor blk $TEMPFILE $TEMPFILE2
RET=$?
if [ $RET -ne 0 ]; then
	# Nothing to report here. The pre-processor would have reported error(s).
	rm -f $TEMPFILE $TEMPFILE2
 	exit $RET
fi

echo "Converting to DataSeries..."
./csv2ds-extra -q $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $TEMPFILE2

echo "Cleaning up"
rm -f $TEMPFILE
rm -f $TEMPFILE2
