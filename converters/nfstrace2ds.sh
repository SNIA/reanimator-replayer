#!/bin/bash

#
# nfstrace2ds.sh converts traces in tshark format to Dataseries format.
# The sequence of operations performed by this script is the following:
# 1. Uses nfsparse to generate the CSV file from the trace captured using 
#    tshark. Please use the below command for running tshark
#    # tshark  -r <pcap_file> -R nfs -t e -z proto,colinfo,rpc.xid,rpc.xid -z proto,colinfo,nfs.full_name,nfs.full_name
# 2. Use csv2ds-extra to convert CSV to Dataseries format.
#    Specification of the fields in the input CSV file and the mapping
#    between CSV fields to Dataseries fields are provided by
#    specstrings/nfstrace.spec and tables/nfs_fields.table
#    files, in order.
#

#
# Usage: nfstrace2ds.sh <tshark_capture_file> <outputfile>
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

./nfsparse $INPUTFILE $TEMPFILE

echo "Converting to DataSeries..."
./csv2ds-extra -q $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $TEMPFILE

echo "Cleaning up"
rm -f $TEMPFILE
