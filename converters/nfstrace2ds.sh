#!/bin/bash

#  Copyright (c) 2012 Sudhir Kasanavesi
#  Copyright (c) 2012 Kalyan Chandra
#  Copyright (c) 2012 Nihar Reddy
#  Copyright (c) 2012 Vasily Tarasov
#  Copyright (c) 2012 Erez Zadok
#  Copyright (c) 2012 Stony Brook University
#  Copyright (c) 2012 The Research Foundation of SUNY
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# nfstrace2ds.sh converts NFS traces in PCAP format to DataSeries format.
# The sequence of operations performed by this script is the following:
# 1. Uses nfsparse to generate the CSV file from the trace captured using 
#    tcpdump.  Please use the below command for running tshark
#
# 2. Use csv2ds-extra to convert CSV to Dataseries format.
#    Specification of the fields in the input CSV file and the mapping
#    between CSV fields to Dataseries fields are provided by
#    specstrings/nfstrace.spec and tables/nfs_fields.table
#    files, in order.
#
# Usage: nfstrace2ds.sh <pcap_file> <outputfile>
#

if [ $# -ne 2 ]; then
	echo "Usage: $0 <tshark_capture_file> <outputfile> "
	exit 1
fi

INPUTFILE=$1
OUTPUTFILE=$2

TABLEFILE=./tables/nfs_fields.table
SPECSTRINGFILE=./specstrings/nfstrace.spec

if [ ! -e csv2ds-extra ]; then
	echo "csv2ds-extra binary is not found. Maybe make it?"
	exit 1
fi

if [ ! -e $TABLEFILE ]; then
	echo "NFS-trace table file is not found! Please put it in $TABLEFILE"
	exit 1
fi

if [ ! -e $SPECSTRINGFILE ]; then
	echo "Specification string for nfstrace is not found! Please put it in \
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

echo "Parsing tshark capture file with nfsparse..."

TEMPFILE=`mktemp`
echo "Using temporary file $TEMPFILE"

tshark -o tcp.check_checksum:'FALSE' -o column.format:'"Time", "%t", "Source", "%s", "Destination", "%d", "Protocol", "%p", "No.", "%m", "Info", "%i"' -r $INPUTFILE -R nfs -t e -z proto,colinfo,nfs.full_name,nfs.full_name -z proto,colinfo,rpc.xid,rpc.xid > $TEMPFILE

#tshark -r $INPUTFILE -R nfs -t e -z proto,colinfo,rpc.xid,rpc.xid -z proto,colinfo,nfs.full_name,nfs.full_name > $TEMPFILE

TEMPFILE2=`mktemp`
echo "Using temporary file $TEMPFILE2"

./nfsparse $TEMPFILE $TEMPFILE2

echo "Converting to DataSeries..."
./csv2ds-extra -q $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $TEMPFILE2

echo "Cleaning up"
rm -f $TEMPFILE
rm -f $TEMPFILE2
