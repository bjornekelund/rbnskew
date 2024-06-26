#!/bin/bash
# Get yesterday's RBN data and run a number of analyses on it
# Put result in files rbnskew.txt, rbnskew2.txt, rbnhist.txt
# and anchors.txt and upload them to web host.
#
# Checks if it has already run for today and if it has, exits
#
# Using the command line parameter TEST will only send the
# results to a single email address.
#set -x

# Move to correct folder to allow cron execution on RPi
[ -d "/home/sm7iun/rbnskew" ] && cd /home/sm7iun/rbnskew

DATE=`date -u --date="1 days ago" +%Y%m%d`
OLDESTRES=`date -u --date="5 days ago" +%Y%m%d`.txt
FOLDER="webserver"

echo "---"
echo "Job started "`date -u "+%F %T"` UTC

START=$SECONDS

# Check if we already did the work for today, if so, exit
[ -f $FOLDER/done ] || echo "Never" > $FOLDER/done
if [ "$DATE" == "`cat $FOLDER/done`" ]; then
    echo "Nothing to do."
    exit
fi

[ -f $FOLDER/$OLDESTRES ] || ./initweb.bash

# Do the work
echo "Downloading RBN data for "`date -u --date="1 days ago" +%Y-%m-%d`

wget --quiet --no-hsts http://www.reversebeacon.net/raw_data/dl.php?f=$DATE -O $FOLDER/rbndata.zip

FILESIZE=$(stat -c%s $FOLDER/rbndata.zip)
if [[ $FILESIZE != "0" ]]; then
    gunzip < $FOLDER/rbndata.zip > $FOLDER/rbndata.csv
    echo "Downloaded yesterday's "$((`wc -l < $FOLDER/rbndata.csv` - 2))" spots into rbndata.csv"
    ./makenewref.bash
    ./$FOLDER/updatewebdata.bash
    ./$FOLDER/updatehistdata.bash
    ./$FOLDER/updateanchordata.bash
    cd $FOLDER
    printf "Uploading to web hosting..."
    ./upload.bash
    printf "done\n"
    echo $DATE > done
    cd ..
else
    echo "Could not download yesterday's RBN data!"
    echo "Job failed "`date -u "+%F %T" `
    exit
fi

./emailstatus.bash $1
echo "Job ended "`date -u "+%F %T"`" UTC and took $((SECONDS-START)) seconds"
exit
