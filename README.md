# rbnskew
Skew analysis for the RBN network

Tools for analysing frequency accuracy of RBN skimmers.

Uses a set of trusted reference skimmers to calculate the average deviation.

`offlineskew` analyses a selected skimmer and also provides a suggested adjustment 
factor to be applied to the FreqCalibration parameter in SkimSrv.ini.
SkimSrv.ini can be found in %appdata%\Afreet\Products\SkimSrv

offlineskew -f "rbn csv file" -t "targeted skimmer" -d (verbose mode)

`skimlist` does a similar analysis but for all skimmers with more 
than 100 spots in the file. The produced list can optionally be sorted 
by absolute average deviation.

skimlist -f "rbn csv file" -d (verbose) -s (sort result)

The analysis of skimlist has the following characteristics:

* Only considers the 500 most recent spots at any time
* Only includes spots less than 60 seconds apart from a reference spot
* Only considers spots with less than 0.5kHz absolute frequency error
* Does not list skimmers with less than 100 spots 





