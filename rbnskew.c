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
#define MINSNR 10
// Minimum frequency for spot to be used
#define MINFREQ 7000
// Minimum number of spots to be analyzed
#define MINSPOTS 100
// Maximum difference from reference spot times 100Hz 
#define MAXERR 5
// Name of file containing callsigns of reference skimmmers
#define REFFILENAME "reference"

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
    int count;         // Number of analyzed spots
    time_t first;      // Earliest spot
    time_t last;       // Latest spot
};

// Print to stderr. Print also to stdout if piped.
void printboth(char *outstring, bool quiet)
{
    if (!quiet)
        fprintf(stderr, "%s", outstring);

    if (isatty(STDOUT_FILENO) == 0)
        printf("%s", outstring);
}

int main(int argc, char *argv[]) {
    int referenceskimmers = 0;
    char referenceskimmer[MAXREF][STRLEN];
    FILE *fp, *fr;
    char filename[STRLEN] = "", target[STRLEN] = "";
    int totalspots = 0, usedspots = 0, c, got, i, j, spp = 0;
    time_t starttime, stoptime, spottime, firstspot, lastspot;
    struct tm *timeinfo, stime;
    bool verbose = false, reference, sort = false, targeted = false, quiet = false;
    char line[LINELEN], de[STRLEN], dx[STRLEN], timestring[STRLEN];
    char firsttimestring[STRLEN], lasttimestring[STRLEN];
    char outstring[LINELEN], tempstring[STRLEN];
    int snr, delta, adelta, skimmers = 0, skimpos, column;
    float freq;

    struct Spot pipeline[SPOTSWINDOW];
    struct Skimmer skimmer[MAXSKIMMERS], temp;

    // Avoid that unitialized entries in pipeline are used
    for (i = 0; i < SPOTSWINDOW; i++)
        pipeline[i].analyzed = true;

    while ((c = getopt(argc, argv, "qt:sdf:")) != -1)
    {
        switch (c)
        {
            case 'f':
                strcpy(filename, optarg);
                break;
            case 't':
                if (strlen(optarg) == 0)
                {
                    fprintf(stderr, USAGE, argv[0]);
                    return 1;
                }
                for (i = 0; i < strlen(optarg) + 1; i++)
                    target[i] = toupper(optarg[i]);
                targeted = true;
                break;
            case 'd':
                verbose = true;
                break;
            case 'q':
                quiet = true;
                break;
            case 's':
                sort = true;
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

    while (fgets(line, LINELEN, fr))
    {
        if (sscanf(line, "%s", tempstring) == 1)
        {
            // Don't include comments or target call
            if (tempstring[0] != '#' && strcmp(tempstring, target) != 0)
            {
                strcpy(referenceskimmer[referenceskimmers], tempstring);
                referenceskimmers++;
                if (referenceskimmers > MAXREF) {
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

    // List reference skimmers
    strcpy(outstring, "Reference skimmers used: ");
    printf("%s", outstring);
    column = (int)strlen(outstring);
    for (i = 0; i < referenceskimmers; i++)
    {
        sprintf(outstring, i == referenceskimmers - 1 ? "and %s" : "%s, ", referenceskimmer[i]);
        printf("%s", outstring);
        column += strlen(outstring);
        if (column > 70)
        {
            printf("\n      ");
            column = 6;
        }
    }
    printf(".\n");

    fp = fopen(filename, "r");

    while (fgets(line, LINELEN, fp))
    {
        // callsign,de_pfx,de_cont,freq,band,dx,dx_pfx,dx_cont,mode,db,date,speed,tx_mode
        got = sscanf(line, "%[^,],%*[^,],%*[^,],%f,%*[^,],%[^,],%*[^,],%*[^,],%*[^,],%d,%[^,],%*s", 
            de, &freq, dx, &snr, timestring);

        if (got == 5 ) // If parsing is successful
        {
            totalspots++;

            if (snr >= MINSNR && freq >= MINFREQ) // If SNR is sufficient and frequency OK
            {
                (void)strptime(timestring, FMT, &stime);
                spottime = mktime(&stime);

                reference = false;
                // Check if this spot is from a reference skimmer
                for (i = 0; i < referenceskimmers; i++)
                {
                    if (strcmp(de, referenceskimmer[i]) == 0)
                    {
                        reference = true;
                        // break;
                    }
                }

                // If it is reference spot, use it to check all un-analyzed,
                // non-reference spots in the pipeline
                if (reference)
                {
                    for (i = 0; i < SPOTSWINDOW; i++)
                    {
                        if (!pipeline[i].analyzed && !pipeline[i].reference &&
                            strcmp(pipeline[i].dx, dx) == 0 &&
                            abs(difftime(pipeline[i].time, spottime)) <= MAXAPART &&
                            !(targeted && strcmp(pipeline[i].de, target) != 0))
                        {
                            delta = pipeline[i].freq - (int)round(freq * 10.0);
                            adelta = delta > 0 ? delta : -delta;

                            // if (pipeline[i].reference || pipeline[i].analyzed)
                                // fprintf(stderr, "Error!\n");

                            pipeline[i].analyzed = true; // To only analyze each spot once

                            if (adelta <= MAXERR) // Only consider spots less than MAXERR off from reference skimmer
                            {
                                usedspots++;

                                if (adelta > 2 && verbose && !quiet) // Print outliers
                                {
                                    stime = *localtime(&pipeline[i].time);
                                    (void)strftime(timestring, STRLEN, FMT, &stime);                        
                                    fprintf(stderr,
                                        "Outlier spot of %8s by %8s at %7.1f (was %7.1f) off by %+3.1f @ %s\n",
                                        pipeline[i].dx, pipeline[i].de, pipeline[i].freq / 10.0, 
                                        freq, delta / 10.0, timestring);
                                }

                                // Check if this skimmer is already in list
                                skimpos = -1;
                                for (j = 0; j < skimmers; j++)
                                {
                                    if (strcmp(pipeline[i].de, skimmer[j].name) == 0)
                                    {
                                        skimpos = j;
                                        break;
                                    }
                                }

                                if (skimpos != -1) // if in the list, update it
                                {
                                    skimmer[skimpos].accdev += 100000.0 * delta / freq;
                                    skimmer[skimpos].count++;
                                    if (pipeline[i].time > skimmer[skimpos].last)
                                        skimmer[skimpos].last = pipeline[i].time;
                                    if (pipeline[i].time < skimmer[skimpos].first)
                                        skimmer[skimpos].first = pipeline[i].time;
                                }
                                else // If new skimmer, add it to list
                                {
                                    strcpy(skimmer[skimmers].name, pipeline[i].de);
                                    skimmer[skimmers].accdev = 100000.0 * delta / freq;
                                    skimmer[skimmers].count = 1;
                                    skimmer[skimmers].first = pipeline[i].time;
                                    skimmer[skimmers].last = pipeline[i].time;
                                    skimmers++;
                                    if (verbose && !quiet)
                                        fprintf(stderr, "Found skimmer #%d: %s \n", skimmers, pipeline[i].de);
                                    if (skimmers > MAXSKIMMERS) {
                                        fprintf(stderr, "Overflow: More than %d skimmers found.\n", MAXSKIMMERS);
                                        return 1;
                                    }
                                }
                            }
                        }
                    }
                }

                // Save new spot in pipeline
                strcpy(pipeline[spp].de, de);
                strcpy(pipeline[spp].dx, dx);
                pipeline[spp].freq = (int)round(freq * 10.0);
                pipeline[spp].snr = snr;
                pipeline[spp].reference = reference;
                pipeline[spp].analyzed = false;
                pipeline[spp].time = spottime;

                // Move pointer and wrap around at top of pipeline
                spp = (spp + 1) % SPOTSWINDOW;
            }
        }
    }

    (void)fclose(fp);

    // Calculate statistics
    firstspot = skimmer[0].first;
    lastspot = skimmer[0].last;
    for (i = 0; i < skimmers; i++)
    {
        skimmer[i].avdev = skimmer[i].accdev / skimmer[i].count;
        skimmer[i].absavdev = fabs(skimmer[i].avdev);
        if (skimmer[i].first < firstspot)
            firstspot = skimmer[i].first;
        if (skimmer[i].last > lastspot)
            lastspot = skimmer[i].last;
    }

    // Sort by absolute average deviation if desired
    if (sort)
    {
        for (i = 0; i < skimmers - 1; ++i)
        {
            for (j = 0; j < skimmers - 1 - i; ++j)
            {
                if (skimmer[j].absavdev > skimmer[j + 1].absavdev)
                {
                    temp = skimmer[j + 1];
                    skimmer[j + 1] = skimmer[j];
                    skimmer[j] = temp;
                }
            }
        }
    }

    stime = *localtime(&firstspot);
    (void)strftime(firsttimestring, STRLEN, FMT, &stime);

    stime = *localtime(&lastspot);
    (void)strftime(lasttimestring, STRLEN, FMT, &stime);

    // Print summary
    sprintf(outstring, "Processed a total of %d RBN spots (%s to %s).\n", 
        totalspots, firsttimestring, lasttimestring);
    printboth(outstring, quiet);
    sprintf(outstring, "The average spot flow was %.0f spots per minute from %d active skimmers.\n", 
        60.0 * totalspots / difftime(lastspot, firstspot), skimmers);
    printboth(outstring, quiet);
    sprintf(outstring, "%d spots were selected for analysis using the following criteria:\n", usedspots);
    printboth(outstring, quiet);
    sprintf(outstring, " * Same callsign spotted by a reference skimmer within the %d most recent spots.\n", SPOTSWINDOW);
    printboth(outstring, quiet);
    sprintf(outstring, " * Same callsign spotted by a reference skimmer within %ds. \n", MAXAPART);
    printboth(outstring, quiet);
    sprintf(outstring, " * SNR is %ddB or higher. \n", MINSNR);
    printboth(outstring, quiet);
    sprintf(outstring, " * Frequency is %dkHz or higher. \n", MINFREQ);
    printboth(outstring, quiet);
    sprintf(outstring, " * Frequency deviation from the reference skimmer spot is %.1fkHz or less.\n", MAXERR / 10.0);
    printboth(outstring, quiet);
    sprintf(outstring, " * %d or more spots available from originating skimmer.\n", MINSPOTS);
    printboth(outstring, quiet);

    (void)time(&stoptime);
    timeinfo = localtime(&stoptime);
    sprintf(outstring, "Total processing time was %.0f seconds.\n", difftime(stoptime, starttime));
    if (isatty(STDOUT_FILENO) == 0)
        printf("%s", outstring);

    // Present results
    for (i = 0; i < skimmers; i++)
    {
        if (skimmer[i].count > MINSPOTS)
        {
            printf("Skimmer %9s average deviation %+5.1fppm over %5d spots (%11.9f)\n", 
                skimmer[i].name, skimmer[i].avdev, skimmer[i].count, 1.0 + skimmer[i].avdev / 1000000.0
                );
        }
    }

    return 0;
}