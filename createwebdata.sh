#!/bin/bash
# Run a full analysis on yesterday's RBN data in file rbndata.csv 
# Put result in files rbnskew.txt, and rbnskew2.txt
#set -x

RBNFOLDER="rbndata"
WEBFOLDER="webfiles"
SAVEFILE=$RBNFOLDER/`date -u --date="1 days ago" +%Y%m%d`.txt
DELFILE=$RBNFOLDER/`date -u --date="11 days ago" +%Y%m%d`.txt

./rbnskew -wq -f $RBNFOLDER/rbndata.csv > $SAVEFILE
echo "Created $SAVEFILE and deleted $DELFILE"
rm -f $DELFILE

cat $SAVEFILE | tr "#" " " > $WEBFOLDER/rbnskew.txt
echo "Updated rbnskew.txt"

./rbnskew -wqh -f $RBNFOLDER/rbndata.csv | tr "#" " " > $WEBFOLDER/rbnskew2.txt
echo "Updated rbnskew2.txt"

cat $SAVEFILE | tr "*" " " | gawk -f csv.awk > $WEBFOLDER/rbnskew.csv
echo "Updated rbnskew.csv"

exit
