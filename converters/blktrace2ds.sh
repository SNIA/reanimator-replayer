#! /bin/bash
#
# ./blktrace2ds.sh <outputfile> <blktrace_base_file>


TABLEFILE=tables/snia_to_blktrace_fields_mapping.csv
OUTPUTFILE=$1
SPECSTRINGFILE=specstrings/blktrace

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
	echo "specification string for blktrace is not found! Please put it in $SPECSTRINGFILE"
	exit 1
fi

if [ -z "$OUTPUTFILE" ]; then
        echo "You must the output file name as a first argument!"
        exit 1
fi

i=1
INPUTFILE=$2

if [ -z "$INPUTFILE" ]; then
        echo "You must provide input files!"
        exit 1
fi

TEMPFILE1=`mktemp`
echo "Using temporary file" $TEMPFILE1

echo "Parsing blktrace file using blkparse..."
blkparse -F A,"read_write,%T%t,%p,%M%m,%d,%S,%N\n" $INPUTFILE | grep read_write > $TEMPFILE1
if [ $? -ne 0 ]; then
        rm -f $TEMPFILE1
        exit $?
fi

TEMPFILE2=`mktemp`
echo "Using temporary file" $TEMPFILE2

echo "Pre-processing ..."
./pre-processor blk $TEMPFILE1 $TEMPFILE2

if [ $? -ne 0 ]; then
        rm -f $TEMPFILE1 $TEMPFILE2
        exit $?
fi

echo "Cleaning up" $TEMPFILE1
rm -f $TEMPFILE1

echo "Converting to DataSeries..."
./csv2ds-extra -q $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $TEMPFILE2

echo "Cleaning up" $TEMPFILE2
rm -f $TEMPFILE2
