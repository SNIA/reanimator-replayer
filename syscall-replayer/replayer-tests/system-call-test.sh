#!/bin/bash
#
# Note: following variables are defined in Makefile.
# This script is usually invoked by makefile: make
# Modify following variables if the path of your converter or replayer is different.
# Following variables are going to be used by the test scripts
SYS_CALL_CONVERTER_DIR=$HOME/trace2model/syscall-converter
SYS_CALL_REPLAYER_DIR=$HOME/trace2model/syscall-replayer
SYS_CALL_REPLAYER_TEST_DIR=$SYS_CALL_REPLAYER_DIR/replayer-tests
SYS_CALL_REPLAYER=system-call-replayer
STRACE_STRSIZE=4096
STRACE2DS_SCRIPT=strace2ds.sh
# Note: script first argument is the name of system call that you want to test.
#       $1 format: test-default|warn|abort-syscallname
#       second argument is the mode that we want to execute replayer.
#
# Extract syscallname and mode by parsing $1 argument
REPLAY_MODE=`echo $1 | cut -d '-' -f 2`
SYS_CALL_NAME=`echo $1 | cut -d '-' -f 3`
TEST_STRACE=$SYS_CALL_REPLAYER_TEST_DIR/$SYS_CALL_NAME-test.strace
TEST_PROG=$SYS_CALL_REPLAYER_TEST_DIR/$SYS_CALL_NAME-test-prog
TEST_DS=$SYS_CALL_REPLAYER_TEST_DIR/$SYS_CALL_NAME-test.ds
TEST_RESULT=0

# Print a message on console
echo "Run system call replayer test $SYS_CALL_NAME in $REPLAY_MODE mode"

# Since some system calls depend on other system calls, we need to capture the
# system call that user wants to test and some system calls that are needed.
# Ex: close. Since a file needs to be opened first before being closed. Therefore,
# we have to capture both open and close if user wants to test close.
SYS_CALLS=$SYS_CALL_NAME
case "$SYS_CALL_NAME" in
    "close")
	SYS_CALLS=open,close
	;;
esac

# Modify following code if mode values are changed.
DEFAULT_MODE="-w0"
WARN_MODE="-w1"
ABORT_MODE="-w2"

# Modify following code if there are more modes.
if [ "$REPLAY_MODE" = "abort" ]
then
    REPLAY_MODE=$ABORT_MODE
elif [ "$REPLAY_MODE" = "warn" ]
then
    REPLAY_MODE=$WARN_MODE
else
    REPLAY_MODE=$DEFAULT_MODE
fi

###########################Setup Start########################################
# Enter test directory.
cd $SYS_CALL_REPLAYER_TEST_DIR
if test $? != 0
then
    echo "Enter system call replayer test directory - Failed"
    exit 1
fi

# Generate strace of the test program.
strace -ttt -v -e trace=$SYS_CALLS -s $STRACE_STRSIZE -o $TEST_STRACE $TEST_PROG
if test $? != 0
then
    echo "Strace: capture system call traces of $SYS_CALL_NAME test program - Failed"
    exit 1
fi

###########################Setup End########################################

###########################Conversion Start#################################
# Go to the sys call converter directory
cd $SYS_CALL_CONVERTER_DIR
if test $? != 0
then
    echo "Enter system call converter directory - Failed"
    exit 1
fi

# Run strace2ds script to automatically converter strace to DS.
./$STRACE2DS_SCRIPT $TEST_STRACE $TEST_DS

if test $? != 0
then
    echo "Convertering strace to DataSeries - Failed"
    exit 1
fi
###########################Conversion End###################################

###########################Replaying Start##################################
# Enter replayer directory.
cd $SYS_CALL_REPLAYER_DIR
if test $? != 0
then
    echo "Enter system call replayer directory - failed"
    exit $1
fi

# Run the replayer

./$SYS_CALL_REPLAYER $TEST_DS $REPLAY_MODE
# Update test result
TEST_RESULT=$?

# Replaying is finished, so clean things up
rm -f $TEST_STRACE $TEST_DS

echo -n "$SYS_CALL_NAME test "

if test $TEST_RESULT = 0
then
    echo "passed"
else
    echo "failed"
fi
###########################Replaying End####################################
