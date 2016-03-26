#!/bin/bash

# Note: following variables are defined in Makefile.
# Therefore, this script is usually invoked by the command: make test
# You need to provide following variables to invoke this script alone.
# Also, you can override them if you want to customize this test script.
# SYS_CALL_CONVERTER_DIR
# SYS_CALL_REPLAYER_DIR
# SYS_CALL_REPLAYER_TEST_DIR
# STRACE_STRSIZE

# Modify following variables if file name changes.
TEST_STRACE=$SYS_CALL_REPLAYER_TEST_DIR/open-test.strace
TEST_PROG=$SYS_CALL_REPLAYER_TEST_DIR/open-test-prog
TEST_DS=$SYS_CALL_REPLAYER_TEST_DIR/open-test.ds
TEST_RESULT=0
SYS_CALLS=open

###########################Setup Start########################################
# Enter test directory.
cd $SYS_CALL_REPLAYER_TEST_DIR

# The name of tmp file that is going to be opened
TMPFILE="./$tmp-file.$$"

# Create the tmp file.
echo "Hello world" > $TMPFILE
###########################Setup End########################################

###########################Conversion Start#################################
# Generate strace of the test program.
strace -ttt -e trace=$SYS_CALLS -s $STRACE_STRSIZE -o $TEST_STRACE $TEST_PROG $TMPFILE

# Remove tmp file.
rm -f $TMPFILE

# Go to the sys call converter directory
cd $SYS_CALL_CONVERTER_DIR

# Run strace2ds script to automatically converter strace to DS.
./$STRACE2DS_SCRIPT $TEST_STRACE $TEST_DS

# Update test result
TEST_RESULT=$?

if test $TEST_RESULT != 0
then
    echo "$SYS_CALLS test failed"
    exit $TEST_RESULT
fi
###########################Conversion End###################################

###########################Replaying Start##################################
# Enter replayer directory.
cd $SYS_CALL_REPLAYER_DIR

# Create the tmp file for replaying.
echo "Hello world" > $TMPFILE

# Run the replayer
./$SYS_CALL_REPLAYER $TEST_DS

# Update test result
TEST_RESULT=$?

echo -n "$SYS_CALLS test "

if test $TEST_RESULT = 0
then
    echo "passed"
else
    echo "failed"
fi

# Clean up
rm -f $TMPFILE $TEST_STRACE $TEST_DS
###########################Replaying End####################################
