#!/bin/bash
# Assumes ssmtp is installed (sudo apt install ssmtp)
# Assumes email is set up in /etc/ssmtp/ssmtp.conf

FILE=webserver/rbnhist.txt
ADDRESS=bjorn@ekelund.nu

echo "Subject: Skimmer skew report for" `date -u "+%F" --date="1 day ago"` > .email.txt
echo "Content-Type:text/html; charset=\"UTF-8\"" >> .email.txt
echo "<pre>" >> .email.txt
grep Skimmer $FILE >> .email.txt
grep -- -- $FILE >> .email.txt
for call in SM7IUN SE5E OH6BG SM0FPR SJ2W N6TV; do
  grep $call $FILE >> .email.txt
done
echo "</pre>" >> .email.txt
sendmail $ADDRESS < .email.txt
exit
