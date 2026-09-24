#!/bin/bash
# Creates anchors.txt which contains a list 
# of the verified anchors
#set -x

WFOLDER="webfiles"
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
}' < $AFILE > $WFOLDER/$OFILE

echo >> $WFOLDER/$OFILE
echo "Last updated "`date -u "+%F %T"`" UTC" >> $WFOLDER/$OFILE

printf "done\n"
exit
