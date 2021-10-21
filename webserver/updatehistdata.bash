#!/bin/bash
# Uses saved analysis results from the last five days
# to assemble a table sorted by skimmer callsign
# updatewebdata creates the required result files.
# These have be to manually created before the first run.
#set -x

FOLDER="webserver"
FILES="`date -u --date="1 days ago" +%Y%m%d` `date -u --date="2 days ago" +%Y%m%d`\
 `date -u --date="3 days ago" +%Y%m%d` `date -u --date="4 days ago" +%Y%m%d`\
 `date -u --date="5 days ago" +%Y%m%d` `date -u --date="6 days ago" +%Y%m%d`\
 `date -u --date="7 days ago" +%Y%m%d` `date -u --date="8 days ago" +%Y%m%d`\
 `date -u --date="9 days ago" +%Y%m%d` `date -u --date="10 days ago" +%Y%m%d`"

cd $FOLDER
rm -f history.txt

echo "Updating historical data for: $FILES"

for file in $FILES; do
    awk '{ if ($1 == "#") { print $2 " " substr(FILENAME,0,8) " " $3; }}' $file.txt | sed 's/*//g' >> history.txt
done

# Produce header of output file
# Print data in reverse chronological order with newest data to the left
awk '{
  array[$1][$2] = $3;
  call = $1;
}
END {
# Find a call that has been active all five days
  for (trycall in array) {
    if (isarray(array[trycall])) {
      k = 0;
      for (elem in array[trycall])
        k++;
      if (k == 10) {
        call = trycall;
        break;
      }
    }
  }
  printf("Skimmer    ");
  j = 0
  for (datestring in array[call]) {
    date[j++] = substr(datestring, 5, 4);
  }
# Print in antichronological order
  for (j = 9; j >= 0; j--) {
    printf("%6s", date[j]);
  }
  printf("\n-----------------------------------------------------------------------\n");
}' history.txt > rbnhist.txt

# Produce meat of output file
# Print data in reverse chronological order with newest data to the left
awk '
{
  array[$1][$2] = $3;
  call = $1;
}
END {
# Find a call that has been active all five days
  for (trycall in array) {
    if (isarray(array[trycall])) {
      k = 0;
      for (elem in array[trycall])
        k++;
      if (k == 10)
        call = trycall;
    }
  }
# Put the date of the last five days in date[]
  j = 0
  for (datestring in array[call]) {
    date[j++] = datestring;
  }
  for (i in array) {
	printf("%-11s", i);
    if (isarray(array[i])) {
# Print data in row in antichronological order
      for (j = 9; j >= 0; j--) {
        printf("%6s", array[i][date[j]]);
      }
	  printf("\n");
    }
}}' history.txt | sort >> rbnhist.txt

echo >> rbnhist.txt
echo "Last updated "`date -u "+%F %T"`" UTC" >> rbnhist.txt

exit
