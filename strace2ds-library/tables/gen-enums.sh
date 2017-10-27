#!/bin/sh
# generate enum header file from *.table file

date=`date`
tmp=.tmp_count.$$

######################################################################
### ENUM for syscall names
# output heading of enum
cat <<EOF
/*
 * This file was auto generated on $date.
 * DO NOT EDIT BY HAND.
 */
enum syscall_names {
EOF
count=0
# output list of all enums
(
    cat snia_syscall_fields.table | while read syscall field nullable
    do
	test -n "$syscall" && echo "$syscall"
    done
) | sort -i | uniq | while read i ; do
    printf "\tSYSCALL_NAME_%s = %d,\n" `echo "$i" | tr 'a-z' 'A-Z'` "$count"
    let count=count+1
    echo $count > $tmp
done
# output end of all enum
printf "\tMAX_SYSCALL_NAMES = %d\n};\n" "`cat $tmp`"

######################################################################
### ENUM for syscall fields
# output heading of enum
cat <<EOF

enum syscall_args {
EOF
count=0
# output list of all enum
(
    cat snia_syscall_fields.table | while read syscall field nullable
    do
	test -n "$field" && echo "$field"
    done
) | sort -i | uniq | while read i ; do
    printf "\tSYSCALL_FIELD_%s = %d,\n" `echo "$i" | tr 'a-z' 'A-Z'` "$count"
    let count=count+1
    echo $count > $tmp
done

# output end of all enum
printf "\tMAX_SYSCALL_FIELDS = %d\n};\n" "`cat $tmp`"

/bin/rm -f "$tmp"
exit 0
