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
# strace2ds.sh converts traces in strace format to Dataseries format.
# The sequence of operations performed by this script is the following:
# 1. Run strace2csv that converts raw strace to CVS appropriate format
#    (doing this in C++, not in bash, increases the speed drastically)
# 2. Use csv2ds to convert CSV to Dataseries format.
#    Specification of the fields in the input CSV file and the mapping
#    between CSV fields to Dataseries fields are provided by
#    specstrings/strace.spec and tables/snia_snia_syscall_fields.table
#
# Usage: ./strace2ds.sh <strace_input_file> <ds_output_file>
#

TABLEFILE=tables/snia_syscall_fields.table
INPUTFILE=$1
OUTPUTFILE=$2
SPECSTRINGFILE=specstrings/strace.spec
XML_DIR=./xml/

make
if [ ! -e csv2ds ]; then
    echo "csv2ds binary is not found. Maybe make it?"
    exit 1
fi

if [ ! -e strace2csv ]; then
    echo "strace2csv binary is not found. Maybe make it?"
    exit 1
fi

if [ ! -e $TABLEFILE ]; then
    echo "system call trace table file is not found! Please put it in $TABLEFILE"
    exit 1
fi

if [ ! -e $SPECSTRINGFILE ]; then
    echo "specification string for strace is not found! Please put it in $SPECSTRINGFILE"
    exit 1
fi

if [ ! -e $XML_DIR ]; then
    echo "XML directory is missing. Using generate-xml script to generate XML files..."
    ./generate-xml.sh $TABLEFILE
    if [ $? -ne 0 ]; then
	echo "generate-xml.sh failed to generate. Please generate them on your own."
	exit 1
    fi
fi

if [ -z "$INPUTFILE" ]; then
    echo "You must provide input files!"
    exit 1
fi

if [ -z "$OUTPUTFILE" ]; then
    echo "You must provide the output file name as the second argument!"
    exit 1
fi

TEMPFILE="temp-file.$$"
touch $TEMPFILE
echo "Using temporary file" $TEMPFILE

echo "Converting strace to CSV..."
./strace2csv $INPUTFILE $TEMPFILE

if [ $? -ne 0 ]; then
    rm -f $TEMPFILE
    exit $?
fi

echo "Converting strace to CSV - Done"

echo "Converting CSV to DataSeries..."
./csv2ds -q $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $XML_DIR $TEMPFILE

echo "Removing temporary file" $TEMPFILE
rm -f $TEMPFILE

echo "Converting CSV to DataSeries - Done"
