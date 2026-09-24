# Support files for web publication
A set of shell scripts to perform three different analyses and 
upload their results in text form to a web server for display 
using the simple PHP scripts `rbnref.php`, `rbnhist.php`, and `rbnskew.php`.

The scripts are `updatewebdata`, `updaterefdata`, `updatehistdata`, and
`updateactdata`.
As written, they will run on a Raspberry Pi under Raspbian. Note that they are 
intended to be run *in the folder above to avoid having copies 
of `reference` and `rreference` in the folder.

To execute them periodically, add the script `updateweb` to the Pi's crontab. 
To run them hourly:

```
10 * * * * /home/sm7iun/rbnskew/updateweb >> /home/sm7iun/webrbnskew.log 2>&1
```

Note that `updatewebdata` must be run before `updatehistdata` since the
latter relies on the output from the former when the date changes. 
`updaterefdata` should be run last since it can take many minutes 
to run after a busy day like a major contest. 

It is also possible to embed the text file in a Wordpress page using
the *Insert PHP Code Snippet* plugin and the body code from the PHP 
scripts above. 

```
<?php
    echo "<pre>";
    echo file_get_contents("rbnhist.txt");
    echo "</pre>";
?>
```
