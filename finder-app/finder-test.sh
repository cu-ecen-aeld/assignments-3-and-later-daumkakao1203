#!/bin/sh

numfiles=10
writestr=aeld-assignments
assignment=$1

if [ $# -lt 3 ]
then
	GREPSTR=$writestr
else
	numfiles=$2
	writestr=$3
	GREPSTR=$writestr
fi

WRITEDIR=/tmp/aeld-data/$writestr

rm -rf "$WRITEDIR"
mkdir -p "$WRITEDIR"

if [ ! -d "$WRITEDIR" ]
then
	echo "$WRITEDIR could not be created"
	exit 1
fi

for i in $(seq 1 $numfiles)
do
	writer "$WRITEDIR/${i}.txt" "$writestr"
done

OUTPUTSTRING=$(finder.sh "$WRITEDIR" "$GREPSTR")

echo "$OUTPUTSTRING" > /tmp/assignment4-result.txt

set +e
echo "$OUTPUTSTRING" | grep "The number of files are $numfiles and the number of matching lines are $numfiles"
if [ $? -eq 0 ]; then
	echo "success"
	exit 0
else
	echo "failed"
	exit 1
fi
