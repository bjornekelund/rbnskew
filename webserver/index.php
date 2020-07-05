<!DOCTYPE html>
<html>
<head>
 <title>SM7IUN</title>
</head>
<body>

<h1>Welcome to SM7IUN RBN Analytics</h1>
<?php
#    echo "Current time: ";
#    echo date('Y-m-d H:i:s');
    echo "<pre>";
    echo file_get_contents("/home/pi/rbnskew/rbnskew.txt");
    echo "</pre>";
?>
</body>
</html>
