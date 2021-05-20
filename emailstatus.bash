#!/bin/bash
# Assumes ssmtp is installed (sudo apt install ssmtp)
# Assumes email is set up in /etc/ssmtp/ssmtp.conf

FILE=webserver/rbnhist.txt

for address in bjorn@ekelund.nu sm5ajv@qrq.se; do
  echo "From: RBN Analytics <sm7iun@sm7iun.se>" > .email.txt
  echo "To:" $address >> .email.txt
  echo "Subject: Skimmer skew report for" `date -u "+%F" --date="1 day ago"` >> .email.txt
  echo "Content-Type:text/html; charset=\"utf-8\"" >> .email.txt
  #echo "Content-Type:text/plain; charset=\"utf-8\"" >> .email.txt
  echo "Content-Transfer-Encoding: quoted-printable" >> .email.txt
  echo "<!DOCTYPE html PUBLIC \"-//WRC//DTD HTML 3.2//EN\">" >> .email.txt
  echo "<html lang="en" xml:lang="en" xmlns= "http://www.w3.org/1999/xhtml">" >> .email.txt
  echo "<pre>" >> .email.txt
  grep Skimmer $FILE >> .email.txt
  grep -- -- $FILE >> .email.txt
  for call in SE5E SJ2W SM0TCZ SM6FMB SM7IUN N6TV OE9GHV OH6BG; do
    grep $call $FILE >> .email.txt
  done
  echo "</pre>" >> .email.txt
  echo "</html>" >> .email.txt
  /usr/sbin/sendmail $address < .email.txt
done
exit
