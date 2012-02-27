#! /bin/bash
#
# ./systrace2ds.sh <outputfile> <spec_string_file> <inputfiles...>


TABLEFILE=tables/snia_block_fields.table
OUTPUTFILE=$1
SPECSTRINGFILE=$2

if [ ! -e csv2ds-extra ]; then
	echo "csv2ds-extra binary is not found. Maybe make it?"
	exit 1
fi

if [ ! -e $TABLEFILE ]; then
	echo "systrace table file is not found! Please put it in $TABLEFILE"
	exit 1
fi

if [ -z "$OUTPUTFILE" ]; then
        echo "You must the output file name as a first argument!"
        exit 1
fi

if [ -z "$SPECSTRINGFILE" ]; then
        echo "You must provide the specification of the records as a string in a file as a second argument!"
        exit 1
fi

i=1
INPUTFILES=""
for arg in $@
do
	if [ $i -gt 2 ]; then
		INPUTFILES=$INPUTFILES" "$arg
	fi
	i=`expr $i + 1`
done

if [ -z "$INPUTFILES" ]; then
        echo "You must provide input files!"
        exit 1
fi

./csv2ds-extra $OUTPUTFILE $TABLEFILE $SPECSTRINGFILE $INPUTFILES
