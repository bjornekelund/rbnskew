#!/bin/bash
# Calculate a new set of reference skimmers for the next night's run
# Built to be called from script updateweb

FOLDER="webserver"
SOURCE=$FOLDER/`date -u --date="1 days ago" +%Y%m%d`.txt
REFFILE="reference"

echo "-"
printf "Selecting reference skimmers..."
./rbnskew -wq -f $FOLDER/rbndata.csv -c anchors > $SOURCE

echo "# Automatically generated reference file" > $REFFILE

printf "# Geographically motivated reference skimmers\nCX6VM\n3B8CW\nJO1YYP\n" >> $REFFILE

echo "# Skimmers with 0.0ppm deviation and more than 20 qualified spots" >> $REFFILE
awk '{if ($1 == "#" && ($3 >= 0.0 ? $3 : -$3) < 0.1 && $4 > 20){print $2;}}' $SOURCE | sed 's/*//g' >> $REFFILE

echo "# Skimmers with 0.1ppm deviation and more than 50 qualified spots" >> $REFFILE
awk '{if ($1 == "#" && ($3 >= 0.0 ? $3 : -$3) == 0.1 && $4 > 50){print $2;}}' $SOURCE | sed 's/*//g' >> $REFFILE

echo "# Skimmers with 0.2ppm deviation and more than 100 qualified spots" >> $REFFILE
awk '{if ($1 == "#" && ($3 >= 0.0 ? $3 : -$3) == 0.2 && $4 > 100){print $2;}}' $SOURCE | sed 's/*//g' >> $REFFILE

printf "done.\n"

exit