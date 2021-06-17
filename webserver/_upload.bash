#!/bin/sh
# This is just an example file with dummy information
ftp -n ftp.server.com  <<END_SCRIPT
quote USER UsErNaMe
quote PASS pAsSwOrD
cd sm7iun.se/targetfolder
put rbnskew.txt
put rbnskew2.txt
put rbnhist.txt
put rbnact.txt
put anchors.txt
quit
END_SCRIPT
exit 0
