#! /bin/bash
#
# ./vss2ds.sh <outputfile> <vscsistats file>

TABLEFILE=tables/snia_block_fields.table
OUTPUTFILE=$1
SPECSTRINGFILE=specstrings/vscsistats.spec

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
	echo "specification string for vscsistats is not found! Please put it in $SPECSTRINGFILE"
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

TEMPFILE=`mktemp`
echo "Using temporary file" $TEMPFILE

echo "Pre-processing ..."
./pre-processor vss $INPUTFILE $TEMPFILE

if [ $? -ne 0 ]; then
        rm -f $TEMPFILE
        exit $?
fi

echo "Converting to DataSeries..."
./csv2ds-extra -q $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $TEMPFILE

echo "Cleaning up" $TEMPFILE
rm -f $TEMPFILE
