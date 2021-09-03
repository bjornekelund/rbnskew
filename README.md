# rbnskew
Skimmer skew analysis for the RBN network http://www.reversebeacon.net

`rbnskew` analyses a comma separated list of spots (formatted as the csv 
files available from http://www.reversebeacon.net/raw_data) to determine
the relative frequency error for all skimmers listed in the file. 

It uses a set of trusted skimmers to calculate the average deviation in reported
frequency for all spots meeeting a selection of criterias.

The default filename for the list of trusted skimmers is `reference`. 

The script `updateweb.bash` is run shortly after UTC midnight every day to 
update https://sm7iun.se/rbn/analytics. To embed the text files in the Wordpress 
page, php snippets are used. 

`updateweb.bash` calls the scripts `makenewref.bash`, `webserver/updatewebdata.bash`, 
`webserver/updatehistdata.bash`, and `webserver/updateactdata.bash` and finally
uploads the analysis results to the web server using the script `webserver/upload.bash`. 
The upload script is not available in the repo since it contains login information. 

To maximize the number of usable spots, the skew analysis is done in two successive steps. 

After downloading the data from RBN, the script `makenewref.bash` is executed.
The purpose of this first step is to determine which skimmers were reliable yesterday.

This is done by running `rbnskew` on yesterday's RBN data set using the skimmmers in the file `ANCHORS` 
as reference. The file `ANCHORS` contains a set of highly trusted (typically GPSDO-controlled) 
skimmers.

The results are then used to create an expanded list of trusted skimmers which is saved in the 
file `reference`. `makenewref.bash` lists all skimmers with more than 100 spots and 
less than 0.2ppm deviation from the "anchor" skimmers in this updated `reference` file. 

The second step is then to run `webserver/updatewebdata.bash` using the updated `reference` 
file and create the text output for the web site.

The script `webserver/updatehistdata.bash` uses the results from `updatewebdata.bash` for 
the last five days to create a text table.

The script `webserver/updateactdata.bash` calculates the activity statistics from yesterday's 
RBN data set and creates two text tables. 

The format of the `reference` file is simple. One callsign per line.
Comment lines are allowed and start with "#". 

For a more rapid analysis, the analysis can be done for only a selected call,
using the -t option.

The script `initweb.bash` offers a crude way to start up the process. 
It will run a basic (not two-step) analysis of the last five days of 
RBN data to make sure the table created by `webserver/updatehistdata.bash` is not empty. 

The script `getrbndata` downloads all RBN data for a selected month (that does not 
already exist) into the `rbnfiles` subfolder.

The adjustment factor listed within parentheses is intended to be applied to
CW Skimmer Server's current value of the `FreqCalibration` parameter in `SkimSrv.ini`.
If it is the default 1.0, just replace it with the suggested factor. Otherwise,
multiply the value currently used with the adjustment factor to get the corrected value.

`SkimSrv.ini` can be found in `%appdata%\Afreet\Products\SkimSrv`

`rbnskew -f csvfile [-dshqrw] [-t callsign] [-m N] [-n N] [-x N]`

 `-f file`\
	File name of RBN spot data set.
	
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

`-x N`\
    Set maximum allowed difference in time stamp to a reference spot for spot to qualify.

 `-t callsign`\
    Do analysis only for this skimmer callsign. Runs considerably faster.

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
`$ make`\
`$ ./rbnskew -f rbnfiles/test.csv`
