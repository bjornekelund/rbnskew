#!/bin/bash
# Assumes ssmtp is installed (sudo apt install ssmtp)
# Assumes email is set up in /etc/ssmtp/ssmtp.conf

FILE=webserver/rbnhist.txt
COUNT=0
echo "Mailing status report to:"
for address in `grep -v \# mailrecipients`; do
#for address in bjorn@ekelund.nu bjornekelund@gmail.com; do
  printf $address
  printf " "
  # Email header
  echo "From: SM7IUN RBN Analytics <noreply@rbn.sm7iun.se>" > .email.txt
  echo "To:" $address >> .email.txt
  echo "Subject: Skimmer skew report for" `date -u "+%F" --date="1 day ago"` >> .email.txt
  echo "Content-Type:text/html; charset=\"utf-8\"" >> .email.txt
  echo "<html lang=\"en\"><body>" >> .email.txt
  echo "<pre>" >> .email.txt

  # Header line and separator
  grep Skimmer $FILE >> .email.txt
  grep -- -- $FILE >> .email.txt

  # List all tracked skimmers
  # Add space to end to avoid false matches of aliases
  grep -v \# trackedskimmers | sed -e 's/$/ /g' > .trackedskimmers
  grep -f .trackedskimmers $FILE >> .email.txt

  # Separator line
  grep -- -- $FILE >> .email.txt

  # List all anchor skimmers before removing misbehaving ones
  # Add space to end to avoid false matches of aliases
  grep -v \# anchors | sed -e 's/$/ /g' > .anchors
  grep -f .anchors $FILE >> .email.txt

  echo >> .email.txt
  echo "</pre>" >> .email.txt
  echo "Visit <a href=\"https://sm7iun.se/rbn/analytics\">sm7iun.se</a> for more detailed information." >> .email.txt
  echo "</body></html>" >> .email.txt
  COUNT=$(($COUNT+1))
  if ! (($COUNT % 4)); then
    echo
  fi
  /usr/sbin/sendmail $address < .email.txt
done
printf "\n"
exit
