#!/usr/bin/env bash
# Creates analysis results for the past five days
# provide historical data for script webfiles/updatehistdata
# Used when installing fresh
#set -x

RBNFOLDER="rbnfiles"
DATES="`date -u --date="1 days ago" +%Y%m%d` `date -u --date="2 days ago" +%Y%m%d`\
 `date -u --date="3 days ago" +%Y%m%d` `date -u --date="4 days ago" +%Y%m%d`\
 `date -u --date="5 days ago" +%Y%m%d` `date -u --date="6 days ago" +%Y%m%d`\
 `date -u --date="7 days ago" +%Y%m%d` `date -u --date="8 days ago" +%Y%m%d`\
 `date -u --date="9 days ago" +%Y%m%d` `date -u --date="10 days ago" +%Y%m%d`"

echo "Creating historical analysis results for:" $DATES

for date in $DATES; do
    echo "Downloading RBN data for:" $date
    wget --quiet --no-hsts http://www.reversebeacon.net/raw_data/dl.php?f=$date -O $RBNFOLDER/rbndata.zip
    FILESIZE=$(stat -c%s $RBNFOLDER/rbndata.zip)
    if [[ $FILESIZE != "0" ]]; then
        gunzip < $RBNFOLDER/rbndata.zip > $RBNFOLDER/rbndata.csv
        echo "Downloaded "$((`wc -l < $RBNFOLDER/rbndata.csv` - 2))" spots."
        ./rbnskew -wq -f $RBNFOLDER/rbndata.csv > $RBNFOLDER/$date.txt
        echo "Analysis done, result saved in" $RBNFOLDER/$date.txt
    else
        echo "Failed to download RBN data"
        exit
    fi
done
exit
