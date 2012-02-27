#! /bin/bash
#
# ./spc2ds.sh <outputfile> <inputfile>


TABLEFILE=tables/snia_block_fields.table
OUTPUTFILE=$1
SPECSTRINGFILE=specstrings/spctrace.spec

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

INPUTFILE=$2
if [ -z "$INPUTFILE" ]; then
        echo "You must provide input files!"
        exit 1
fi

TEMPFILE=`mktemp`
echo "Using temporary file" $TEMPFILE

echo "Parsing spectrace..."
./pre-processor spc $INPUTFILE $TEMPFILE

if [ $? -ne 0 ]; then
        rm -f $TEMPFILE
        exit $?
fi

echo "Converting to DataSeries..."
./csv2ds-extra -q $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $TEMPFILE

echo "Removing temporary file" $TEMPFILE
rm -f $TEMPFILE
