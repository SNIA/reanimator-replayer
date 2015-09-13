#! /bin/bash
#
# Copyright (c) 2015-2016 Leixiang Wu
# Copyright (c) 2011-2012 Erez Zadok
# Copyright (c) 2011-2012 Stony Brook University
# Copyright (c) 2011-2012 The Research Foundation of SUNY
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
#
# strace2ds.sh converts traces in strace format to Dataseries format.
# The sequence of operations performed by this script is the following:
# 1. Run pre-processor that converts raw strace CVS to CVS appropriate format
#    (doing this in C++, not in bash, increases the speed drastically)
# 2. Use csv2ds-extra to convert CSV to Dataseries format.
#    Specification of the fields in the input CSV file and the mapping
#    between CSV fields to Dataseries fields are provided by
#    specstrings/blktrace.spec and tables/snia_block_fields.table
#    files, in order.
#
# Usage: strace2ds.sh <outputfile> <strace_base_file>
#

TABLEFILE=tables/snia_syscall_fields.table
OUTPUTFILE=$1
INPUTFILE=$2
SPECSTRINGFILE=specstrings/strace.spec

if [ ! -e csv2ds-extra ]; then
	echo "csv2ds-extra binary is not found. Maybe make it?"
	exit 1
fi

if [ ! -e pre-processor ]; then
	echo "pre-processor binary is not found. Maybe make it?"
	exit 1
fi

if [ ! -e $TABLEFILE ]; then
	echo "blocktrace table file is not found! Please put it in $TABLEFILE"
	exit 1
fi

if [ ! -e $SPECSTRINGFILE ]; then
	echo "specification string for spctrace is not found! Please put it in $SPECSTRINGFILE"
	exit 1
fi

if [ -z "$OUTPUTFILE" ]; then
        echo "You must the output file name as a first argument!"
        exit 1
fi

if [ -z "$INPUTFILE" ]; then
        echo "You must provide input files!"
        exit 1
fi

TEMPFILE=`mktemp`
echo "Using temporary file" $TEMPFILE

echo "Parsing spectrace..."
./pre-processor sys $INPUTFILE $TEMPFILE

if [ $? -ne 0 ]; then
        rm -f $TEMPFILE
        exit $?
fi

echo "Converting to DataSeries..."
./csv2ds-extra -q $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $TEMPFILE

echo "Removing temporary file" $TEMPFILE
#rm -f $TEMPFILE
