#!/usr/bin/env bash
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
WEBFOLDER="webfiles"
RBNFOLDER="rbndata"
CREDFILE=WEBCREDENTIALS

echo "---"
echo "Job started "`date -u "+%F %T"` UTC

START=$SECONDS

# Check if we already did the work for today, if so, exit
[ -f $WEBFOLDER/done ] || echo "Never" > $WEBFOLDER/done
if [ "$DATE" == "`cat $WEBFOLDER/done`" ]; then
    echo "Nothing to do."
    exit
fi

[ -f rbnskew ] || make

[ -f $RBNFOLDER/$OLDESTRES ] || ./initweb.sh

# Do the work
echo "Downloading RBN data for "`date -u --date="1 days ago" +%Y-%m-%d`

wget --quiet --no-hsts http://www.reversebeacon.net/raw_data/dl.php?f=$DATE -O $RBNFOLDER/rbndata.zip

FILESIZE=$(stat -c%s $RBNFOLDER/rbndata.zip)
if [[ $FILESIZE != "0" ]]; then
    gunzip < $RBNFOLDER/rbndata.zip > $RBNFOLDER/rbndata.csv
    echo "Downloaded yesterday's "$((`wc -l < $RBNFOLDER/rbndata.csv` - 2))" spots"
    ./makenewref.sh
    ./createwebdata.sh
    ./createhistdata.sh
    ./createanchordata.sh
#    printf "Uploading to web hosting...\n"
    ./ftptohost.sh $CREDFILE $WEBFOLDER/rbnskew.txt $WEBFOLDER/rbnskew2.txt $WEBFOLDER/rbnhist.txt $WEBFOLDER/anchors.txt $WEBFOLDER/rbnskew.csv
#    printf "done\n"
    echo $DATE > $WEBFOLDER/done
else
    echo "Could not download yesterday's RBN data!"
    echo "Job failed "`date -u "+%F %T" `
    exit
fi

./emailstatus.sh $1
echo "Job ended "`date -u "+%F %T"`" UTC and took $((SECONDS-START)) seconds"
exit
