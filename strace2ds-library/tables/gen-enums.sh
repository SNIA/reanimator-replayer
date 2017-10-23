#!/bin/sh
# generate enum header file from *.table file

date=`date`

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
# output list of all enums
(
    cat snia_syscall_fields.table | while read syscall field nullable
    do
	test -n "$syscall" && echo "$syscall"
    done
) | sort -i | uniq | while read i ; do
    printf "\tSYSCALL_NAME_%s,\n" `echo "$i" | tr 'a-z' 'A-Z'`
done
# output end of all enum
cat <<EOF
	MAX_SYSCALL_NAMES
};
EOF

######################################################################
### ENUM for syscall fields
# output heading of enum
cat <<EOF

enum syscall_args {
EOF

# output list of all enum
(
    cat snia_syscall_fields.table | while read syscall field nullable
    do
	test -n "$field" && echo "$field"
    done
) | sort -i | uniq | while read i ; do
    printf "\tSYSCALL_FIELD_%s,\n" `echo "$i" | tr 'a-z' 'A-Z'`
done

# output end of all enum
cat <<EOF
	MAX_SYSCALL_FIELDS
};
EOF

exit 0
