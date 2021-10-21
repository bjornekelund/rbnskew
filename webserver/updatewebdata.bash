#!/bin/bash
# Run a full analysis on yesterday's RBN data in file rbndata.csv 
# Put result in files rbnskew.txt, and rbnskew2.txt

FILE=`date -u --date="1 days ago" +%Y%m%d`
FOLDER="webserver"
SAVEFILE=$FOLDER/`date -u --date="1 days ago" +%Y%m%d`.txt
DELFILE=$FOLDER/`date -u --date="11 days ago" +%Y%m%d`.txt

./rbnskew -wq -f $FOLDER/rbndata.csv > $SAVEFILE
tr "#" " " < $SAVEFILE > $FOLDER/rbnskew.txt
echo "Updated rbnskew.txt and also saved result in "$SAVEFILE

rm -f $DELFILE
echo "Deleted "$DELFILE

./rbnskew -wqh -f $FOLDER/rbndata.csv | tr "#" " " > $FOLDER/rbnskew2.txt
echo "Updated rbnskew2.txt"
exit
