#!/bin/bash
# Assumes ssmtp is installed (sudo apt install ssmtp)
# Assumes email is set up in /etc/ssmtp/ssmtp.conf

FILE=webserver/rbnhist.txt

for address in bjorn@ekelund.nu sm5ajv@qrq.se holger@gatternig.com; do
#for address in bjorn@ekelund.nu; do
  echo "From: SM7IUN RBN Analytics <sm7iun@sm7iun.se>" > .email.txt
  echo "To:" $address >> .email.txt
  echo "Subject: Skimmer skew report for" `date -u "+%F" --date="1 day ago"` >> .email.txt
  echo "Content-Type:text/html; charset=\"utf-8\"" >> .email.txt
#  echo "Mime-Version: 1.0" >> .email.txt
#  echo "Content-Type:text/plain; charset=\"utf-8\"" >> .email.txt
#  echo "Content-Transfer-Encoding: quoted-printable" >> .email.txt
#  echo "<!DOCTYPE html PUBLIC \"-//WRC//DTD HTML 3.2//EN\">" >> .email.txt
  echo "<html lang=\"en\"><body>" >> .email.txt
  echo "<pre>" >> .email.txt
  grep Skimmer $FILE >> .email.txt
  grep -- -- $FILE >> .email.txt
  for call in SE5E SJ2W SM0TCZ SM6FMB SM7IUN N6TV OH6BG; do
    grep $call $FILE >> .email.txt
  done
  grep -- -- $FILE >> .email.txt
  grep AC0C-1 $FILE >> .email.txt
  grep 'K3PA ' $FILE >> .email.txt
  grep G4ZFE $FILE >> .email.txt
  grep 'KM3T-1' $FILE >> .email.txt
  grep 'KM3T ' $FILE >> .email.txt
  grep OE9GHV $FILE >> .email.txt
  grep NA0B $FILE >> .email.txt
  grep LZ3CB $FILE >> .email.txt
  grep DF7GB $FILE >> .email.txt
  grep WB6BEE $FILE >> .email.txt
  echo "</pre>" >> .email.txt
  echo "Visit <a href=\"https://sm7iun.se/rbn/analytics\">sm7iun.se</a> for more detailed information." >> .email.txt
  echo "</body></html>" >> .email.txt
  /usr/sbin/sendmail $address < .email.txt
done
exit
