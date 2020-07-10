#!/bin/bash
# Creates analysis results for the past five days
# provide historical data for script webserver/updatehistdata
#set -x

FOLDER="webserver"
DATES="`date -u --date="1 days ago" +%Y%m%d` `date -u --date="2 days ago" +%Y%m%d`\
 `date -u --date="3 days ago" +%Y%m%d` `date -u --date="4 days ago" +%Y%m%d`\
 `date -u --date="5 days ago" +%Y%m%d`"

echo "Creating historical analysis results for:" $DATES

for date in $DATES; do
    echo "Downloading RBN data for:" $date
    wget --quiet --no-hsts http://www.reversebeacon.net/raw_data/dl.php?f=$date -O $FOLDER/rbndata.zip
    FILESIZE=$(stat -c%s $FOLDER/rbndata.zip)
    if [[ $FILESIZE != "0" ]]; then
        gunzip < $FOLDER/rbndata.zip > $FOLDER/rbndata.csv
        echo "Downloaded "$((`wc -l < $FOLDER/rbndata.csv` - 2))" spots."
        ./rbnskew -wq -f $FOLDER/rbndata.csv > $FOLDER/$date.txt
        echo "Analysis done, result saved in" $FOLDER/$date.txt
    else
        echo "Failed to download RBN data"
        exit
    fi
done
exit
