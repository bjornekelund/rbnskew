#!/bin/bash
# Run a full analysis on yesterday's RBN data in file rbndata.csv 
# Put result in files rbnskew.txt, and rbnskew2.txt

FILE=`date -u --date="1 days ago" +%Y%m%d`
RBNFOLDER="rbndata"
WEBFOLDER="webfiles"
SAVEFILE=$WEBFOLDER/`date -u --date="1 days ago" +%Y%m%d`.txt
DELFILE=$WEBFOLDER/`date -u --date="11 days ago" +%Y%m%d`.txt

./rbnskew -wq -f $RBNFOLDER/rbndata.csv > $SAVEFILE
tr "#" " " < $SAVEFILE > $WEBFOLDER/rbnskew.txt
echo "Updated rbnskew.txt and also saved result in "$SAVEFILE

rm -f $DELFILE
echo "Deleted "$DELFILE

./rbnskew -wqh -f $RBNFOLDER/rbndata.csv | tr "#" " " > $WEBFOLDER/rbnskew2.txt
echo "Updated rbnskew2.txt"

tr "*" " " < $SAVEFILE | gawk -f csv.awk > $WEBFOLDER/rbnskew.csv
echo "Updated rbnskew.csv"

exit
