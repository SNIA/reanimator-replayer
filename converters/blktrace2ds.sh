#!/bin/bash
#
# Usage: ./blktrace2ds.sh <outputfile> <blktrace_base_file>

if [ $# -ne 2 ]; then
	echo "Usage: $0 <outputfile> <blktrace_base_file>"
	exit 1
fi

OUTPUTFILE=$1
INPUTFILE=$2

TABLEFILE=./tables/snia_to_blktrace_fields_mapping.csv
SPECSTRINGFILE=./specstrings/blktrace

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

if [ -z "$OUTPUTFILE" ]; then
	echo "You must the output file name as a first argument!"
	exit 1
fi

if [ -z "$INPUTFILE" ]; then
	echo "You must provide input files!"
	exit 1
fi

TEMPFILE1=`mktemp`
echo "Using temporary file" $TEMPFILE1

# Run blkparse and grep separately to improve error reporting.

echo "Parsing blktrace file using blkparse..."
blkparse -F Q,"read_write,%T%t,%p,%M%m,%d,%S,%N\n" $INPUTFILE > $TEMPFILE1
RET=$?
if [ $RET -ne 0 ]; then
	echo "blkparse failed with error code $RET"
	rm -f $TEMPFILE1
	exit $RET
fi

TEMPFILE2=`mktemp`
echo "Using temporary file" $TEMPFILE2

grep read_write $TEMPFILE1 > $TEMPFILE2
RET=$?
if [ $RET -ne 0 ]; then
	echo "Unable to find events with TRACE ACTION 'Q'"
	rm -f $TEMPFILE1 $TEMPFILE2
	exit $RET
fi

echo "Pre-processing ..."
./pre-processor blk $TEMPFILE2 $TEMPFILE1
RET=$?
if [ $RET -ne 0 ]; then
	# Nothing to report here. The pre-processor would have reported error(s).
	rm -f $TEMPFILE1 $TEMPFILE2
 	exit $RET
fi

echo "Cleaning up" $TEMPFILE2
rm -f $TEMPFILE2

echo "Converting to DataSeries..."
./csv2ds-extra -q $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $TEMPFILE1

echo "Cleaning up" $TEMPFILE1
rm -f $TEMPFILE1
