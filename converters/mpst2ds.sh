#! /bin/bash
#
# ./mpst2ds.sh <outputfile> <Microsoft Production Server Trace File>

TABLEFILE=tables/snia_block_fields.table
OUTPUTFILE=$1
SPECSTRINGFILE=specstrings/mpstrace.spec

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
	echo "specification string for vscsistats is not found! Please put it in" \
		"$SPECSTRINGFILE"
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

echo "Extracting DiskRead and DiskWrite events from $INPUTFILE ..."
grep -w 'DiskRead\|DiskWrite' $INPUTFILE | grep -v TimeStamp > $TEMPFILE1
RET=$?
if [ $RET -ne 0 ]; then
	echo "Unable to find DiskRead or DiskWrite events."
	rm -f $TEMPFILE1
	exit $RET
fi

TEMPFILE2=`mktemp`
echo "Using temporary file" $TEMPFILE2

echo "Pre-processing ..."
./pre-processor mps $TEMPFILE1 $TEMPFILE2
RET=$?
if [ $? -ne 0 ]; then
        rm -f $TEMPFILE1 $TEMPFILE2
        exit $RET
fi

echo "Cleaning up $TEMPFILE1."
rm -f $TEMPFILE1

echo "Converting to DataSeries..."
./csv2ds-extra -q $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $TEMPFILE2

echo "Cleaning up" $TEMPFILE2
rm -f $TEMPFILE
