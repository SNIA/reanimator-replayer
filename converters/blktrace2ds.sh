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

echo "Parsing blktrace file using blkparse"

blkparse -F A,"read_write %T%t %p,%M%m,%d,%S,%N\n" $INPUTFILE | grep read_write > /tmp/blktrace.unparsed

echo "Line processing"
echo -n "" > /tmp/blktrace.parsed
while read line;
do
	# the second field (time) is in nanoseconds
	echo $line | awk -v time=$(echo `echo $line | cut -f 2 -d ' '` \* 4294967296 / 1000000000 | bc) '{ printf("%s,%s,%s\n",$1, time, $3) }'
done < /tmp/blktrace.unparsed > /tmp/blktrace.parsed


./csv2ds-extra $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE /tmp/blktrace.parsed
#rm -f /tmp/blktrace.unparsed
#rm -f /tmp/blktrace.parsed
