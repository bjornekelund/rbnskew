#!/bin/bash
# get a full month of data, unpack and delete zip files
YEAR="2024"
MONTH="03"
FOLDER="rbnfiles"
mkdir -p $FOLDER
for day in {01..31}
  do
    FILE=$YEAR$MONTH$day
    if [ ! -f "$FOLDER/$FILE.csv" ]; then
        wget --quiet --no-hsts http://www.reversebeacon.net/raw_data/dl.php?f=$FILE -O $FOLDER/$FILE.zip
        FILESIZE=$(stat -c%s "$FOLDER/$FILE.zip")
        if [[ $FILESIZE != "0" ]]; then
            gunzip < $FOLDER/$FILE.zip > $FOLDER/$FILE.csv
            echo "Downloaded "$FOLDER/$FILE.csv" with "$((`wc -l < $FOLDER/$FILE.csv` - 2))" spots."
        fi
    else
         echo $FILE".csv already exist"
    fi
  rm -f $FOLDER/$FILE.zip
  done
exit
