# Support files for web publication
A set of files to upload data in text form to a web server for display using 
the simple PHP scripts `rbnref.php`, `rbnhist.php`, and `rbnskew.php`.

It is also possible to embed the text file in a Wordpress page using
the *Insert PHP Code Snippet* plugin and the body code from the PHP scripts above. 

`<?php\
    echo "<pre>";\
    echo file_get_contents("rbnhist.txt");\
    echo "</pre>";\
?>`

You also need a script called `upload` that uploads the three files `rbnskew.txt`, 
`rbnref.txt`, and `rbnhist.txt`. This file is not included since it contains user 
ID and password to the web site.

For a hosting service supporting FTP, this file could look like this:

`#!/bin/sh\
HOST='ftpcluster.myhosting.com'\
USER='myusername'  
PASSWD='mypassword'  
FILE1='rbnskew.txt'  
FILE2='rbnref.txt'  
FILE3='rbnhist.txt'  
ftp -n $HOST <<END_SCRIPT  
quote USER $USER  
quote PASS $PASSWD  
cd mywebpage.com/public_html  
put $FILE1  
put $FILE2  
put $FILE3  
quit  
END_SCRIPT  
exit 0`  
