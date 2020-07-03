# rbnskew
Skimmer skew analysis for the RBN network http://www.reversebeacon.net

`skimlist` analyses a comma separated list of spots (formatted as the csv  
files available from http://www.reversebeacon.net/raw_data) to determine  
the relative frequency error for all skimmers listed in the file. 

It uses a set of known and trusted GPSDO-controlled skimmers as reference  
and calculates the average deviation in reported frequency for all spots  
meeeting the criteria below. 

The trusted skimmers are listed in the file `reference`. One callsign per line.  
Comment lines are allowed and start with "#". 

For a more rapid analysis, the analysis can be done for only a selected call,  
using the -t option.

This option is for instance used  by the script `checkreferences` which walks through  
the current list of reference skimmers and checks each of them against the others. 

The script `getrbndata` downloads all RBN data for a selected month into the  
`rbnfiles` subfolder.

The adjustment factor listed within parentheses is intended to be applied to  
the skimmer's current value of the FreqCalibration parameter in SkimSrv.ini.  
If it is 1.0, just replace it with the suggested factor. Otherwise,  
multiply the currently used value with the factor.

`SkimSrv.ini` can be found in `%appdata%\Afreet\Products\SkimSrv`

`skimlist -f csvfile [-t callsign] [-d] (verbose) [-s] (sort result) [-q] (quiet)`

The analysis of skimlist has the following characteristics:

* Only considers the 1000 most recent spots at any time for efficiency reasons
* Only considers spots timestamped within 30 seconds of a reference spot
* Only considers spots with SNR 10dB or more to secure relevance of spot
* Only considers spots above 7MHz to reduce truncation noise
* Only considers spots with 0.5kHz or less absolute frequency error to avoid QSY spots
* Ignores spots from skimmers with less than 100 spots to guarantee statistical significance
* Assumes a relative frequency error, i.e. originating solely from the skimmer's reference oscillator

`offlineskew` is a development tool and contains old code. Do not use. 

To run the code on your machine (Linux or Windows' Ubuntu emulator) the following steps are required:

`$ sudo apt install git`\
`$ sudo apt install gcc`\
`$ sudo apt install make`\
`$ git clone https://github.com/bjornekelund/rbnskew`\
`$ cd rbnskew`\
`$ make`
