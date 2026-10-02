#!/bin/bash
# Uses saved analysis results from yesterday to assemble a list of the worst skimmers
# updatehistdata.sh must have been run before this script is run for the first time.
#set -x

WEBFOLDER="webfiles"
RBNFOLDER="rbndata"
HISTORY=$RBNFOLDER/history.txt
OUTFILE=$WEBFOLDER/rbnworst.txt
DATE="`date -u --date="1 days ago" +%Y%m%d`"

echo "Finding the worst skimmmers for $DATE"

# Produce output file
awk -v date="$DATE" '
BEGIN {
  FS = " ";
  first = 1;
  count = 0;
}
{
  if ($2 == date && (($3 + 0.0) > 3.0 || ($3 + 0.0) < -3.0)) 
  {  
    count++;
    if (first) 
    {
      printf("%s", $1);
      first = 0;
    }
    else 
    {
      if (count >= 10) 
      {
        printf(" %s\n", $1);
        count = 0;
        first = 1;     
      }
      else
      {
        printf(" %s", $1);
      }
    }
  }
}
END {
  printf("\n");
}' $HISTORY | sed 's/-[0-9]\+\b//g' > $OUTFILE

exit
