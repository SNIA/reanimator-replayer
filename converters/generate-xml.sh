#!/bin/bash
# This script generates xml files given a configuration table in a folder named
# 'xml'.
#
# params: tablefile
#
# If no specification string is specified, generate a specification string based
# on tablefile.

if [ $# != 1 ]; then
	echo "Usage: $0 <tablefile>"
	exit 1
fi

mkdir -p xml

specstring=""
callstring=""
while read line;
do
	name=`cut -f 1 <<< "$line"`
	if [[ `grep -P "/$name/" <<< "$callstring"` == '' ]]; then
		allfields=`sed 's/ /,/g' <<< $(grep -P "^$name\t" $1 | awk '{ print $2 }')`
		specstring="$specstring""$name($allfields);"
		callstring="$callstring""/$name/"
	fi
done < $1

calls=`sed -e "s/\s//g" <<<$specstring| tr -d "\n" | sed 's/"//g'  | cut -d ";" -f 1- --output-delimiter=" "`

for call in $calls
do
	callname=`echo $call | cut -d "(" -f 1`
	parms=`echo $call | cut -d "(" -f 2 | sed 's/)//' | cut -d "," -f 1- --output-delimiter=" "`

	if [ "`grep -P "^$callname\t" < $1`" == "" ]; then
		echo "ERROR: Generator cannot find syscall $callname in the specified table!"
		exit 1
	fi

	if [ $callname == "Common" ]; then
		commonparms=$parms
	else
		echo "<ExtentType name=\"IOTTAFSL:Trace:Syscall:$callname\" namespace=\"iotta.snia.org.fsl.cs.sunysb.edu\" version=\"1.00\">" > xml/$callname.xml
		for parm in $commonparms $parms
		do
			line=`grep -P "^$callname\t$parm\t" < $1`
			if [ "$line" == "" ]; then
				line=`grep -P "Common\t$parm\t" < $1`
			fi
			if [ "$line" == "" ]; then
				echo
				echo "ERROR: Generator cannot find field $callname:$parm in the specified table!"
				exit 1
			fi

			typename=`echo "$line" | cut -f 4`
			nullable=`echo "$line" | cut -f 3`
			if [ "$nullable" == "1" ]; then
				op=yes
			else
				op=no
			fi
			echo "  <field name=\"$parm\" type=\"$typename\" opt_nullable=\"$op\"/>" >> xml/$callname.xml

		done

		echo "</ExtentType>" >> xml/$callname.xml
	fi
done
