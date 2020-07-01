# rbnskew
Skew analysis for the RBN network

Offline analysis uses a number of calibrated skimmers as reference and can calculate
the average error for a particular skimmer including suggesting a calibration factor.
The suggested factor assumes the skimmer's factor is set to 1.0. If not, the suggested 
factor should be multiplied with the used factor.

offlineskew -f "rbn csv file" -t "targeted skimmer" -d (verbose mode)

