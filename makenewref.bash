#!/bin/bash
# Calculate a new set of reference skimmers for the next night's run
# Built to be called from script updateweb

FOLDER="webserver"
PSOURCE=$FOLDER/`date -u --date="1 days ago" +%Y%m%d`p.txt # Result with anchors
SOURCE=$FOLDER/`date -u --date="1 days ago" +%Y%m%d`.txt # Result with reference skimmers
REFFILE="reference" # File with reference skimmers
VERFILE="verified" # File with verified anchors

echo "-"

# Remove the worst anchor if it has more than a certain deviation to avoid 
# temporarily misbehaving anchors to destroy results.
./removebadanchors.bash anchors .tmp
./removebadanchors.bash .tmp $VERFILE

printf "Selecting reference skimmers..."

./rbnskew -wq -f $FOLDER/rbndata.csv -c $VERFILE > $SOURCE

echo "# Automatically generated reference file" > $REFFILE

echo "# Skimmers with 0.0ppm deviation and more than 100 qualified spots" >> $REFFILE
awk '{if ($1 == "#" && ($3 >= 0.0 ? $3 : -$3) == 0.0 && $4 > 100){print $2;}}' $SOURCE | sed 's/*//g' >> $REFFILE

echo "# Skimmers with 0.1ppm deviation and more than 200 qualified spots" >> $REFFILE
awk '{if ($1 == "#" && ($3 >= 0.0 ? $3 : -$3) == 0.1 && $4 > 200){print $2;}}' $SOURCE | sed 's/*//g' >> $REFFILE

#echo "# Skimmers with 0.2ppm deviation and more than 1000 qualified spots" >> $REFFILE
#awk '{if ($1 == "#" && ($3 >= 0.0 ? $3 : -$3) == 0.2 && $4 > 1000){print $2;}}' $SOURCE | sed 's/*//g' >> $REFFILE

printf "done.\n"

exit
