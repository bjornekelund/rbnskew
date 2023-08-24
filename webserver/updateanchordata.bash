#!/bin/bash
# Creates anchors.txt which contains a list 
# of the verified anchors
#set -x

FOLDER="webserver"
AFILE="VERIFIED"
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
