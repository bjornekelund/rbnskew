#!/bin/bash
# Analyses the content of webserver/rbndata.csv and
# creates summaries of station and skimmer activity
# per continent and band.
# Result is saved in rbnact.txt
#set -x

FOLDER="webserver"
DATE=`date -u --date="1 days ago" +%Y-%m-%d`
DFILE="rbndata.csv"
OFILE="rbnact.txt"

echo "Updating activity data for:" $DATE

./cunique -f $FOLDER/$DFILE > $FOLDER/$OFILE

echo >> $FOLDER/$OFILE
echo "Last updated "`date -u "+%F %T"`" UTC" >> $FOLDER/$OFILE

exit
