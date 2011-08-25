#! /bin/bash
#
# ./spc2ds.sh <outputfile> <inputfile>


TABLEFILE=table/blocktrace.csv
OUTPUTFILE=$1
SPECSTRINGFILE=string/spctrace

if [ ! -e csv2ds-extra ]; then
	echo "csv2ds-extra binary is not found. Maybe make it?"
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

i=1
INPUTFILE=$2

if [ -z "$INPUTFILE" ]; then
        echo "You must provide input files!"
        exit 1
fi

echo "Parsing spcetrace"
sed 's/r/0/g;s/w/1/g;s/\.//g;s/,/ /g' $INPUTFILE > /tmp/spctrace.unparsed

echo "Line processing"
echo -n "" > /tmp/spctrace.parsed
while read line;
do
	echo $line | awk -v time=$(expr `echo $line | cut -f 5 -d ' '` \* 4294967296 / 1000000) '{printf("%s,%s,%s,%s,%s\n",$1, $2, $3, $4, time) }'  >> /tmp/spctrace.parsed
done < /tmp/spctrace.unparsed

echo $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE /tmp/spctrace.parsed
./csv2ds-extra $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE /tmp/spctrace.parsed
#rm -f /tmp/spctrace.parsed
