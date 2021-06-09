#!/bin/bash
# Assumes ssmtp is installed (sudo apt install ssmtp)
# Assumes email is set up in /etc/ssmtp/ssmtp.conf

FILE=webserver/rbnhist.txt

for address in bjorn@ekelund.nu sm5ajv@qrq.se holger@gatternig.com; do
#for address in bjorn@ekelund.nu bjorn.ekelund@ericsson.com; do
  echo "From: SM7IUN RBN Analytics <noreply@rbn.sm7iun.se>" > .email.txt
  echo "To:" $address >> .email.txt
  echo "Subject: Skimmer skew report for" `date -u "+%F" --date="1 day ago"` >> .email.txt
  echo "Content-Type:text/html; charset=\"utf-8\"" >> .email.txt
  echo "<html lang=\"en\"><body>" >> .email.txt
  echo "<pre>" >> .email.txt
  grep Skimmer $FILE >> .email.txt
  grep -- -- $FILE >> .email.txt
  for call in SE5E SJ2W SM6FMB SM7IUN N6TV OH6BG; do
    grep $call $FILE >> .email.txt
  done
  # List all anchor skimmers before pruning list
  # Add space to end to avoid false matches of aliases
  grep -v \# anchors | sed -e 's/$/ /g' > .anchors
  grep -- -- $FILE >> .email.txt
  grep -f .anchors $FILE >> .email.txt
  echo "</pre>" >> .email.txt
  echo " " >> .email.txt
  echo "Visit <a href=\"https://sm7iun.se/rbn/analytics\">sm7iun.se</a> for more detailed information." >> .email.txt
  echo "</body></html>" >> .email.txt
  /usr/sbin/sendmail $address < .email.txt
done
exit
