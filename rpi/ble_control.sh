#!/bin/bash

DEV_ADDR="00:15:83:00:5B:44" # CHANGE THIS 
PORT=$1
ACTION=$2

if [ "$PORT" == "1" ]; then
	if [ "$ACTION" == "1" ]; then
		cmd="05010401FF00"
	else
		cmd="040104010100"
	fi
elif [ "$PORT" == "2" ]; then
	if [ "$ACTION" == "1" ]; then
		cmd="05010402FF00"
	else
		cmd="040104020100"
	fi
elif [ "$PORT" == "3" ]; then
	if [ "$ACTION" == "1" ]; then
		cmd="05010403FF00"
	else
		cmd="040104030100"
	fi
else
	exit
fi

expect << EOF
spawn gatttool -b $DEV_ADDR -I
send "connect\n"
expect "Connection successful"
expect ">"
send "char-write-cmd 0x25 $cmd\n"
expect ">"
exit
EOF
