# rbnskew
Skimmer skew analysis for the RBN network http://www.reversebeacon.net

`rbnskew` analyses a comma separated list of spots (formatted as the csv 
files available from http://www.reversebeacon.net/raw_data) to determine
the relative frequency error for all skimmers listed in the file. 

It uses a set of known and trusted GPSDO-controlled skimmers as reference
and calculates the average deviation in reported frequency for all spots
meeeting the criteria below. 

The trusted skimmers are listed in the file `reference`. One callsign per line.
Comment lines are allowed and start with "#". 

For a quicker rapid analysis, the analysis can be done for only a selected call,
using the -t option.

This option is for instance used  by the script `checkreferences` which uses 
yesterday's RBN data analyze each reference skimmer against the other reference 
skimmers to verify that they can still all be trusted.

The script `getrbndata` downloads all RBN data for a selected month (that does not 
already exist) into the `rbnfiles` subfolder.

The script `yesterday` downloads yesterday's RBN data and runs a full analysis on it.

The adjustment factor listed within parentheses is intended to be applied to
the skimmer's current value of the `FreqCalibration` parameter in `SkimSrv.ini`.
If it is the default 1.0, just replace it with the suggested factor. Otherwise,
multiply the value currently used with the adjustment factor to get the corrected value.

`SkimSrv.ini` can be found in `%appdata%\Afreet\Products\SkimSrv`

`rbnskew -f csvfile [-t callsign] [-dslqrw] [-m N] [-n N]`

 `-t file`\
	Loads RBN spot data set from file.
	
`-d`\
    Debug mode, lots of output.
	
`-s` and `-h`\
    Sorting by average deviation. -s for lowest first, -h for highest first.
	
`-w`\
	Format output for web publication. 
	
`-q`\
	Quiet mode. Does not print to stderr, only to stdout.
	
`-r`\
	Only consider RTTY spots.
	
`-m N`\
	Set minimum number of spots from skimmer to include it in analysis.
	
`-n N`\
	Set minimum SNR required for spot to qualify.

The analysis algorithm has the following default characteristics:

* Only considers the 1000 most recent spots at any time for efficiency reasons
* Only considers spots timestamped within 30 seconds of a reference spot
* Only considers spots with SNR 10dB or more to secure relevance of spot
* Only considers spots above 7MHz to reduce frequency truncation noise
* Only considers spots with 0.5kHz or less absolute frequency error to avoid QSY spots
* Ignores spots from skimmers with less than 100 spots to guarantee statistical significance
* Assumes a relative frequency error, i.e. originating solely from the skimmer's reference oscillator

To run the code on your machine (Linux or Windows' Ubuntu emulator) the following steps are required:

`$ sudo apt install git`\
`$ sudo apt install gcc`\
`$ sudo apt install make`\
`$ git clone https://github.com/bjornekelund/rbnskew`\
`$ cd rbnskew`\
`$ make`
`$ ./rbnskew -f test.csv`
