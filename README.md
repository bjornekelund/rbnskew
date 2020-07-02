# rbnskew
Skew analysis for the RBN network

`skimlist` analyses a comma separated list of spots (formatted as the csv 
files available from http://www.reversebeacon.net/raw_data) to determine 
the relative frequency error for all skimmers listed in the file. 

It uses a set of known and trusted GPSDO-controlled skimmers as reference 
and calculates the average deviation in reported frequency for all spots 
meeeting the criteria below.

For a more rapid analysis, the analysis can be done for only a selected call.

The adjustment factor listed is intended to be applied to the skimmer's 
current value of the FreqCalibration parameter in SkimSrv.ini.
`SkimSrv.ini` can be found in `%appdata%\Afreet\Products\SkimSrv`

`skimlist -f csvfile [-t callsign] [-d] (verbose) [-s] (sort result)`

The analysis of skimlist has the following characteristics:

* Only considers the 500 most recent spots at any time for efficiency reasons
* Only considers spots within 60 seconds of a reference spot
* Only considers spots with more than 5dB SNR to secure relevance of spot
* Only considers spots above 7000kHz to reduce truncation noise
* Only considers spots with less than 0.5kHz absolute frequency error to avoid QSY spots
* Does not list skimmers with less than 100 spots to guarantee statistical significance
* Assumes a relative frequency error, i.e. originating solely from the skimmer's reference oscillator

`offlineskew` is old code and a development tool. Do not use. 

To run the code on your machine (Linux or Windows' Ubuntu emulator) the following steps are required:

`$ sudo apt install git
`$ sudo apt install gcc
`$ sudo apt install make
`$ git clone https://github.com/bjornekelund/rbnskew
`$ cd rbnskew
`$ make




