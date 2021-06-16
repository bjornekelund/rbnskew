#!/bin/bash
# Analyses the content of webserver/rbndata.csv and
# creates summaries of station and skimmer activity
# per continent and band.
# Result is saved in rbnact.txt
#set -x

FOLDER="webserver"
AFILE="verified"
OFILE="anchors.txt"

printf "Updating anchors.txt..."

awk '
{
  if ($1 !~ /^#/) {
    printf("%s ", $1);
  }
}
END {
  printf("\n");
}' < $AFILE > $FOLDER/$OFILE

echo >> $FOLDER/$OFILE
echo "Last updated "`date -u "+%F %T"`" UTC" >> $FOLDER/$OFILE

printf "done\n"
exit
