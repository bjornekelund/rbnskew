# Support files for web publication
A set of files to perform three different analyses and upload their results 
in text form to a web server for display using the simple PHP 
scripts `rbnref.php`, `rbnhist.php`, and `rbnskew.php`.

The three scripts are `updatewebdata`, `updaterefdata`, and `updatehistdata`. 
As written, they will run a Raspberry Pi under Raspbian. They are intended to be 
run in the folder above to avoid having local copies of `reference` and `rreference`.

To execute them periodically, add them to the Pi's crontab. To run them hourly:

```
10 * * * * /home/pi/rbnskew/webserver/updatewebdata >> /home/pi/webrbnskew.log 2>&1
20 * * * * /home/pi/rbnskew/webserver/updatehistdata >> /home/pi/webrbnskew.log 2>&1
30 * * * * /home/pi/rbnskew/webserver/updaterefdata >> /home/pi/webrbnskew.log 2>&1
```

Note that `updatewebdata` must be run before `updatehistdata` since the
latter relies on the output from the former when the date changes. 
`updaterefdata` should be run last since it can take a very long time 
to run after a busy day like a major contest. 

It is also possible to embed the text file in a Wordpress page using
the *Insert PHP Code Snippet* plugin and the body code from the PHP scripts above. 

```
<?php
    echo "<pre>";
    echo file_get_contents("rbnhist.txt");
    echo "</pre>";
?>
```

You also need a script called `upload` that uploads the three files `rbnskew.txt`, 
`rbnref.txt`, and `rbnhist.txt` to the web host. This file is not included in the 
repo since it unfortunately must contain sensitive information in plain text. 

For a hosting service supporting FTP, this file could look like this:

```
#!/bin/sh
ftp -n ftpcluster.myhosting.com <<END_SCRIPT
quote USER myusername
quote PASS mypassword
cd mywebpage.com/public_html
put rbnskew.txt
put rbnref.txt
put rbnhist.txt
quit
END_SCRIPT
exit 0
```