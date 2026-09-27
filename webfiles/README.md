# Support files for web publication
A set of result files suitable for publication on the web, possibly with 
the help of the three simple PHP scripts `rbnref.php`, `rbnhist.php`, 
and `rbnskew.php`.

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
