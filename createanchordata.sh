#!/bin/bash
# Creates anchors.txt which contains a list 
# of the verified anchors
#set -x

WEBFOLDER="webfiles"
INFILE="VERIFIED"
OUTFILE=$WEBFOLDER/anchors.txt

printf "Updating anchors.txt..."

awk '
{
  if ($1 !~ /^#/) {
    printf("%s ", $1);
  }
}
END {
  printf("\n");
}' < $INFILE > $OUTFILE

echo >> $OUTFILE
echo "Last updated "`date -u "+%F %T"`" UTC" >> $OUTFILE

printf "done\n"
exit
