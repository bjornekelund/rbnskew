#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>

// Standard length of strings
#define STRLEN 32
// Standard length of lines
#define LINELEN 256
// Time format in RBN data file
#define FMT "%Y-%m-%d %H:%M:%S"
// Number of most recent spots considered in analysis
#define SPOTSWINDOW 1000
// Maximum number of reference skimmers
#define MAXREF 50
// Maximum number of skimmers supported
#define MAXSKIMMERS 400
// Usage string
#define USAGE "Usage: %s -f filename [-t callsign] [-d] [-s] [-q]\n"
// Max number of seconds apart from a reference spot
#define MAXAPART 30
// Minimum SNR required for spot to be used
#define MINSNR 6
// Minimum frequency for spot to be used
#define MINFREQ 7000
// Minimum number of spots to be analyzed
#define MINSPOTS 50
// Maximum difference from reference spot times 100Hz
#define MAXERR 5
// Name of file containing callsigns of reference skimmmers
#define REFFILENAME "reference"
// Mode of spots
#define MODE "CW"
// Max size of dataset
# define MAXSET 10000000

// Print to stderr. Print also to stdout if piped.
static void printboth(char *outstring, bool quiet)
{
    if (!quiet)
        fprintf(stderr, "%s", outstring);

    if (isatty(STDOUT_FILENO) == 0)
        printf("%s", outstring);
}

int main(int argc, char *argv[])
{
    struct RawSpot
    {
        char de[STRLEN];         // Skimmer callsign
        char dx[STRLEN];         // Spotted call
        time_t time;             // Spot timestamp in epoch format
        char timestring[STRLEN]; // Spot timestamp in ascii
        int snr;                 // SNR for spotcount
        int freq;                // 10x spot frequency
        char mode[STRLEN];       // Transmission mode
    };   

    struct Spot 
    {
        char de[STRLEN];   // Skimmer callsign
        char dx[STRLEN];   // Spotted call
        time_t time;       // Spot timestamp in epoch format
        int snr;           // SNR for spotcount
        int freq;          // 10x spot frequency
        bool reference;    // Originates from a reference skimmer
        bool analyzed;     // Already analyzed
    };

    struct Skimmer
    {
        char name[STRLEN]; // Skimmer callsign
        double accdev;     // Accumulated deviation in ppm
        double avdev;      // Average deviation in ppm
        double absavdev;   // Absolute average deviation in ppm
        double sd;         // Standard deviation
        int count;         // Number of analyzed spots
        time_t first;      // Earliest spot
        time_t last;       // Latest spot
    };

    int referenceskimmers = 0;
    char referenceskimmer[MAXREF][STRLEN];
    FILE *fp, *fr;
    char filename[STRLEN] = "", target[STRLEN] = "";
    int totalspots = 0, usedspots = 0, c, got, spp = 0, refspots = 0, minsnr = MINSNR;
    time_t starttime, stoptime, firstspot, lastspot;
    // time_t spottime;
    struct tm *timeinfo, stime;
    bool verbose = false, worst = false, reference, sort = false, targeted = false, quiet = false;
    char line[LINELEN] = "";
    // char de[STRLEN], dx[STRLEN], timestring[STRLEN], mode[STRLEN];
    char *spotmode = "CW";
    char firsttimestring[STRLEN], lasttimestring[STRLEN];
    char outstring[LINELEN], tempstring[STRLEN];
    // int int, freq, ;
    int delta, adelta, skimmers = 0, skimpos, column, minspots = MINSPOTS, rawspot;
    // float freq;

    static struct RawSpot dataset[MAXSET];
    static struct Spot pipeline[SPOTSWINDOW];
    static struct Skimmer skimmer[MAXSKIMMERS], temp;

    // Avoid that unitialized entries in pipeline are used
    for (int i = 0; i < SPOTSWINDOW; i++)
        pipeline[i].analyzed = true;

    while ((c = getopt(argc, argv, "wqt:sdf:m:rn:")) != -1)
    {
        switch (c)
        {
            case 'f': // Filename
                strcpy(filename, optarg);
                break;
            case 't': // Callsign selected for analysis
                if (strlen(optarg) == 0)
                {
                    fprintf(stderr, USAGE, argv[0]);
                    return 1;
                }
                for (int i = 0; i < (int)strlen(optarg) + 1; i++)
                    target[i] = toupper(optarg[i]);
                targeted = true;
                break;
            case 'd': // Verbose debug mode
                verbose = true;
                break;
            case 'w': // Sort on ppm deviation, worst first
                sort = true;
                worst = true;
                break;
            case 'q': // Quiet, do not print to stderr
                quiet = true;
                break;
            case 's': // Sort on ppm deviation, best first
                sort = true;
                break;
            case 'm': // Minimum number of spots to consider skimmer
                minspots = atoi(optarg);
                break;
            case 'n': // Minimum SNR to consider spot
                minsnr = atoi(optarg);
                break;
            case 'r': // RTTY mode
                spotmode = "RTTY";
                break;
            case '?':
                fprintf(stderr, USAGE, argv[0]);
                return 1;
            default:
                abort();
        }
    }

    if (strlen(filename) == 0)
    {
        fprintf(stderr, USAGE, argv[0]);
        return 1;
    }

    fr = fopen(REFFILENAME, "r");

    if (fr == NULL) 
    {
        fprintf(stderr, "Can not open file \"%s\". Abort.\n", REFFILENAME);
        return 1;
    }
    
    while (fgets(line, LINELEN, fr) != NULL)
    {
        if (sscanf(line, "%s", tempstring) == 1)
        {
            // Don't include comments or target call
            if (tempstring[0] != '#' && strcmp(tempstring, target) != 0)
            {
                strcpy(referenceskimmer[referenceskimmers], tempstring);
                referenceskimmers++;
                if (referenceskimmers >= MAXREF) 
                {
                    fprintf(stderr, "Overflow: More than %d reference skimmers defined.\n", MAXREF);
                    return 1;
                }

            }
        }
    }

    (void)fclose(fr);

    (void)time(&starttime);
    timeinfo = localtime(&starttime);

    if (verbose && !quiet)
        fprintf(stderr, "Starting at %s", asctime(timeinfo));

    if (isatty(STDOUT_FILENO) == 0)
        printf("Skimmer accuracy analysis based on RBN offline data.\n");

    fp = fopen(filename, "r");
    
    if (fp == NULL) 
    {
        fprintf(stderr, "Can not open file \"%s\". Abort.\n", filename);
        return 1;
    }  

    while (fgets(line, LINELEN, fp) != NULL)
    {
        char de[STRLEN], dx[STRLEN], timestring[STRLEN], mode[STRLEN];
        int snr;
        time_t spottime;
        float freq;

        // callsign,de_pfx,de_cont,freq,band,dx,dx_pfx,dx_cont,mode,db,date,speed,tx_mode
        got = sscanf(line, "%[^,],%*[^,],%*[^,],%f,%*[^,],%[^,],%*[^,],%*[^,],%*[^,],%d,%[^,],%*[^,],%s",
            de, &freq, dx, &snr, timestring, mode);

        if (got == 6 ) // If parsing is successful
        {
            (void)strptime(timestring, FMT, &stime);
            spottime = mktime(&stime);

            strcpy(dataset[totalspots].de, de);
            strcpy(dataset[totalspots].dx, dx);
            dataset[totalspots].freq = freq * 10;
            dataset[totalspots].snr = snr;
            dataset[totalspots].time = spottime;
            strcpy(dataset[totalspots].timestring, timestring);
            strcpy(dataset[totalspots].mode, mode);            

            if (totalspots++ == 0) // If first spot
            {
                lastspot = spottime;
                firstspot = spottime;
            }
            else
            {
                lastspot = spottime > lastspot ? spottime : lastspot; 
                firstspot = spottime < firstspot ? spottime : firstspot;
            }

            if (totalspots >= MAXSET) 
            {
                fprintf(stderr, "Overflow: More than %d spots in data set.\n", MAXSET);
                return 1;
            }

        }
    }

    // Go through raw data twice, second time only to calculate standard deviation
    for (int iteration = 1; iteration <= 2; iteration++) 
    {
        for (rawspot = 0; rawspot < totalspots; rawspot++)
        {
            char timestring[STRLEN];
            
            // If SNR is sufficient and frequency OK and mode is right
            if (dataset[rawspot].snr >= minsnr && dataset[rawspot].freq >= MINFREQ * 10 && strcmp(spotmode, dataset[rawspot].mode) == 0) 
            {
                reference = false;
                // Check if this spot is from a reference skimmer
                for (int i = 0; i < referenceskimmers; i++)
                {
                    if (strcmp(dataset[rawspot].de, referenceskimmer[i]) == 0)
                    {
                        reference = true;
                        break;
                    }
                }

                // If it is reference spot, use it to check all un-analyzed,
                // non-reference spots in the pipeline
                if (reference)
                {               
                    // fprintf(stderr, "Found reference spot %s\n", dataset[rawspot].de);
                    refspots++;
                    
                    for (int i = 0; i < SPOTSWINDOW; i++)
                    {
                        if (!pipeline[i].analyzed && !pipeline[i].reference &&
                            strcmp(pipeline[i].dx, dataset[rawspot].dx) == 0 &&
                            abs((int)difftime(pipeline[i].time, dataset[rawspot].time)) <= MAXAPART &&
                            !(targeted && strcmp(pipeline[i].de, target) != 0))
                        {
                            delta = pipeline[i].freq - dataset[rawspot].freq;
                            adelta = delta > 0 ? delta : -delta;

                            pipeline[i].analyzed = true; // To only analyze each spot once

                            if (adelta <= MAXERR) // Only consider spots less than MAXERR off from reference skimmer
                            {
                                // if (iteration == 1)
                                    // usedspots++;

                                // Print outliers if in debug mode
                                if (adelta > 2 && verbose && !quiet && iteration == 1) 
                                {
                                    stime = *localtime(&pipeline[i].time);
                                    (void)strftime(timestring, STRLEN, FMT, &stime);
                                    fprintf(stderr, "Outlier spot of %8s by %8s at %7.1f (was %7.1f) off by %+3.1f @ %s\n",
                                        pipeline[i].dx, pipeline[i].de, pipeline[i].freq / 10.0,
                                        dataset[rawspot].freq / 10.0, delta / 10.0, dataset[rawspot].timestring);
                                }

                                // Check if this skimmer is already in list
                                skimpos = -1;
                                for (int j = 0; j < skimmers; j++)
                                {
                                    if (strcmp(pipeline[i].de, skimmer[j].name) == 0)
                                    {
                                        skimpos = j;
                                        break;
                                    }
                                }

                                if (iteration == 1) 
                                {
                                    usedspots++;

                                    if (skimpos != -1) // if in the list, update it
                                    {
                                        skimmer[skimpos].accdev += 1000000.0 * delta / dataset[rawspot].freq;
                                        skimmer[skimpos].count++;
                                        if (pipeline[i].time > skimmer[skimpos].last)
                                            skimmer[skimpos].last = pipeline[i].time;
                                        if (pipeline[i].time < skimmer[skimpos].first)
                                            skimmer[skimpos].first = pipeline[i].time;
                                    }
                                    else // If new skimmer, add it to list
                                    {
                                        strcpy(skimmer[skimmers].name, pipeline[i].de);
                                        skimmer[skimmers].accdev = 1000000.0 * delta / dataset[rawspot].freq;
                                        skimmer[skimmers].count = 1;
                                        skimmer[skimmers].first = pipeline[i].time;
                                        skimmer[skimmers].last = pipeline[i].time;
                                        skimmer[skimmers].sd = 0.0;
                                        skimmers++;
                                        if (verbose && !quiet)
                                            fprintf(stderr, "Found skimmer #%d: %s \n", skimmers, pipeline[i].de);
                                        if (skimmers > MAXSKIMMERS) 
                                        {
                                            fprintf(stderr, "Overflow: More than %d skimmers found.\n", MAXSKIMMERS);
                                            return 1;
                                        }
                                    }
                                }
                                else // Iteration 2
                                {
                                    // Calculating the average every time is a little wasteful but simple
                                    skimmer[skimpos].avdev = skimmer[skimpos].accdev / skimmer[skimpos].count;
                                    skimmer[skimpos].absavdev = fabs(skimmer[skimpos].avdev);
                                    // We know skimmer is in the list with index skimpos
                                    double diff = 1000000.0 * delta / dataset[rawspot].freq - skimmer[skimpos].avdev;
                                    skimmer[skimpos].sd += diff * diff;
                                }                                                                        
                            }
                        }
                    }
                }

                // Save new spot in pipeline
                strcpy(pipeline[spp].de, dataset[rawspot].de);
                strcpy(pipeline[spp].dx, dataset[rawspot].dx);
                pipeline[spp].freq = dataset[rawspot].freq;
                pipeline[spp].snr = dataset[rawspot].snr;
                pipeline[spp].reference = reference;
                pipeline[spp].analyzed = false;
                pipeline[spp].time = dataset[rawspot].time  ;

                // Move pointer and wrap around at top of pipeline
                spp = (spp + 1) % SPOTSWINDOW;
            }
        }
    }
    
    (void)fclose(fp);

    // // Calculate statistics
    // for (int i = 0; i < skimmers; i++)
    // {
        // skimmer[i].avdev = skimmer[i].accdev / skimmer[i].count;
        // skimmer[i].absavdev = fabs(skimmer[i].avdev);
    // }

    // Sort by callsign, or average deviation if desired
    for (int i = 0; i < skimmers - 1; ++i)
    {
        for (int j = 0; j < skimmers - 1 - i; ++j)
        {
            if (sort ? (worst ? skimmer[j].absavdev < skimmer[j + 1].absavdev : skimmer[j].absavdev > skimmer[j + 1].absavdev )
                : strcmp(skimmer[j].name, skimmer[j + 1].name) > 0)
            {
                temp = skimmer[j + 1];
                skimmer[j + 1] = skimmer[j];
                skimmer[j] = temp;
            }
        }
    }

    // List reference skimmers
    strcpy(outstring, "Reference skimmers: ");
    printf("%s", outstring);
    column = (int)strlen(outstring);
    for (int i = 0; i < referenceskimmers; i++)
    {
        sprintf(outstring, i == referenceskimmers - 1 ? "and %s" : "%s, ", referenceskimmer[i]);
        printf("%s", outstring);
        column += strlen(outstring);
        if (column > 60 && i < referenceskimmers - 1)
        {
            printf("\n  ");
            column = 5;
        }
    }
    printf(".\n");

    // Print results 
    stime = *localtime(&firstspot);
    (void)strftime(firsttimestring, STRLEN, FMT, &stime);
    stime = *localtime(&lastspot);
    (void)strftime(lasttimestring, STRLEN, FMT, &stime);
    sprintf(outstring, "%d RBN spots processed (%s to %s).\n", totalspots, firsttimestring, lasttimestring);
    printboth(outstring, quiet);

    sprintf(outstring, "  of which %d spots (%.1f%%) were reference spots.\n", refspots, 100.0 * refspots / totalspots);
    printboth(outstring, quiet);

    if (targeted) {
        stime = *localtime(&skimmer[0].first);
        (void)strftime(firsttimestring, STRLEN, FMT, &stime);
        stime = *localtime(&skimmer[0].last);
        (void)strftime(lasttimestring, STRLEN, FMT, &stime);
        sprintf(outstring, "The selected skimmer produced an average of %.0f qualified spots per hour\n    between %s and %s.\n", 
            3600.0 * skimmer[0].count / difftime(skimmer[0].last, skimmer[0].first), firsttimestring, lasttimestring);
    }
    else
        sprintf(outstring, "The average spot flow was %.0f spots per minute from %d active skimmers.\n",
        60 * totalspots / difftime(lastspot, firstspot), skimmers);
    printboth(outstring, quiet);

    sprintf(outstring, "%d spots qualified for analysis by meeting the following criteria:\n",
        (targeted && usedspots <= minspots) ? 0 : usedspots);
    printboth(outstring, quiet);

    if (targeted)
        printboth(" * Spotted by the selected skimmer.\n", quiet);

    sprintf(outstring, " * Mode of spot is %s.\n" , spotmode);
    printboth(outstring, quiet);

    sprintf(outstring, " * Also spotted by a reference skimmer within the %d most recent spots.\n", SPOTSWINDOW);
    printboth(outstring, quiet);

    sprintf(outstring, " * Also spotted by a reference skimmer within %ds. \n", MAXAPART);
    printboth(outstring, quiet);

    sprintf(outstring, " * SNR is %ddB or higher. \n", minsnr);
    printboth(outstring, quiet);

    sprintf(outstring, " * Frequency is %dkHz or higher. \n", MINFREQ);
    printboth(outstring, quiet);

    sprintf(outstring, " * Frequency deviation from reference skimmer is %.1fkHz or less.\n", MAXERR / 10.0);
    printboth(outstring, quiet);

    sprintf(outstring, " * At least %d spots from same skimmer in data set.\n", minspots);
    printboth(outstring, quiet);

    (void)time(&stoptime);
    timeinfo = localtime(&stoptime);
    sprintf(outstring, "Total processing time was %.0f seconds.\n", difftime(stoptime, starttime));
    if (isatty(STDOUT_FILENO) == 0)
        printf("%s", outstring);

    // Present results
    for (int i = 0; i < skimmers; i++)
    {
        if (skimmer[i].count >= minspots)
        {
            printf("Skimmer %-9s average deviation %+5.1fppm (std %3.1f) over %5d spots (%11.9f)\n",
                skimmer[i].name, skimmer[i].avdev, sqrt(skimmer[i].sd / skimmer[i].count), skimmer[i].count, 1.0 + skimmer[i].avdev / 1000000.0
                );
        }
    }

    return 0;
};
