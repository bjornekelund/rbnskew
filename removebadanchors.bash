#!/bin/bash

# Removes worst anchor if it has a deviation more than 0.2ppm
# Uses webserver/rbndata.csv
# $1 incoming anchorfile
# $2 outgoing and pruned anchorfile

RBNDATA=webserver/rbndata.csv
RESULT=.result

# Create a grep filter file with anchors. Add space to end of callsign to avoid
# that e.g. KM3T matches with KM3T-1
grep -v \# $1 | sed -e 's/$/ /g' > .anchors

# First, run an analysis with only anchor skimmers. 
printf "Removing inaccurate anchors..."
./rbnskew -wq -f $RBNDATA -c $1 | awk '{if ($1 == "#"){print $0}}' | sed 's/*//g' | grep -f .anchors > $RESULT

# Remove the worst but only if it is more than 0.2ppm off.
awk '
BEGIN {
  worstdev = -1.0;
  worstskimmer = "ZZZ";
  found = 0; }
{
  if ($1 == "#" && $3 ~ /^[+-]?[0-9]+\.[0-9]$/) {
    dev = $3 + 0;
    absdev = dev < 0.0 ? -dev : dev;
#    printf("dev=%f absdev=%f\n", dev, absdev) >> "/dev/stderr";
    if (absdev > worstdev) {
      worstdev = absdev;
      if (absdev > 0.2) {
        worstskimmer = $2;
      }
      found = 1;
    }
  }
}
END {
  printf("%s\n", worstskimmer);
}'  $RESULT > .worstskimmer

grep -v -f .worstskimmer $RESULT | awk '{if ($1 == "#"){print $2}}' | sed 's/*//g' > $2

echo done
exit
