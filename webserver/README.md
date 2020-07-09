# Support files for web publication
A set of files to upload data in text form to a web server for display using 
the simple PHP scripts `rbnref.php`, `rbnhist.php`, and `rbnskew.php`.

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
