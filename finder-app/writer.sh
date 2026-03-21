#!/bin/sh

if [ $# -lt 2 ]
then
	echo "Error: Two arguments are required"
	exit 1
fi

WRITEFILE=$1
WRITESTR=$2

WRITEDIR=$(dirname "$WRITEFILE")

mkdir -p "$WRITEDIR"

echo "$WRITESTR" > "$WRITEFILE"

if [ $? -ne 0 ]
then
	echo "Error: Could not write to file $WRITEFILE"
	exit 1
fi

