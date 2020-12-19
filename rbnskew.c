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
#define STRLEN 16
// Standard length of time strings
#define TSLEN 32
// Standard length of lines
#define LINELEN 256
// Time format in RBN data file
#define FMT "%Y-%m-%d %H:%M:%S"
// Number of most recent spots considered in analysis
#define SPOTSWINDOW 1000
// Maximum number of reference skimmers
#define MAXREF 300
// Maximum number of skimmers supported
#define MAXSKIMMERS 600
// Usage string
#define USAGE "Usage: %s -f file [-dshqrw] [-t call] [-n N] [-m N] [-x sec] [-c file]\n"
// Max number of seconds apart from a reference spot
#define MAXAPART 30
// Minimum SNR required for spot to be used
#define MINSNR 3
// Minimum frequency for spot to be used
#define MINFREQ 7000
// Minimum number of spots to be analyzed
#define MINSPOTS 5
// Maximum difference from reference spot times 100Hz
#define MAXERR 5
// Name of file containing callsigns of reference skimmmers
#define REFFILENAME "reference"
// Mode of spots
#define MODE "CW"

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
        double accdev;     // Accumulated absolute deviation
        double avdev;      // Average deviation in ppm
        double absavdev;   // Absolute average deviation in ppm
        int count;         // Number of analyzed spots
        time_t first;      // Earliest spot
        time_t last;       // Latest spot
        bool reference;    // If a reference skimmer
    };

    FILE   *fp, *fr;
    time_t starttime, stoptime, firstspot, lastspot;
    struct tm *timeinfo, stime;
    char   filename[LINELEN] = "", target[STRLEN] = "", line[LINELEN] = "",
           outstring[LINELEN], referenceskimmer[MAXREF][STRLEN], *spotmode = "CW",
           reffilename[STRLEN] = REFFILENAME;
    bool   verbose = false, worst = false, reference, sort = false,
           targeted = false, quiet = false, forweb = false;
    int    i, j, referenceskimmers = 0, totalspots = 0, usedspots = 0, c,
           spp = 0, refspots = 0, minsnr = MINSNR, skimmers = 0,
           minspots = MINSPOTS, maxapart = MAXAPART;

    static struct Spot pipeline[SPOTSWINDOW];
    static struct Skimmer skimmer[MAXSKIMMERS], temp;

    // Avoid that unitialized entries in pipeline are used
    for (i = 0; i < SPOTSWINDOW; i++)
        pipeline[i].analyzed = true;

    while ((c = getopt(argc, argv, "dshqrwt:x:f:m:n:c:")) != -1)
    {
        switch (c)
        {
            case 'c': // Reference filename
                strcpy(reffilename, optarg);
                break;
            case 'f': // Filename
                strcpy(filename, optarg);
                break;
            case 't': // Callsign selected for analysis
                if (strlen(optarg) == 0)
                {
                    fprintf(stderr, USAGE, argv[0]);
                    return 1;
                }
                for (i = 0; i < (int)strlen(optarg) + 1; i++)
                    target[i] = toupper(optarg[i]);
                targeted = true;
                break;
            case 'd': // Verbose debug mode
                verbose = true;
                break;
            case 'w': // Format for web
                forweb = true;
                break;
            case 'h': // Sort on ppm deviation, worst first
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
            case 'x': // Maximum difference in time to a reference spot
                maxapart = atoi(optarg);
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

    fr = fopen(reffilename, "r");

    if (fr == NULL)
    {
        fprintf(stderr, "Can not open file \"%s\". Abort.\n", reffilename);
        return 1;
    }

    while (fgets(line, LINELEN, fr) != NULL)
    {
        char tempstring[LINELEN];
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
                    (void)fclose(fr);
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

    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        fprintf(stderr, "Can not open file \"%s\". Abort.\n", filename);
        return 1;
    }

    while (fgets(line, LINELEN, fp) != NULL)
    {
        char de[STRLEN], dx[STRLEN], timestring[LINELEN], mode[STRLEN];
        double freq;
        int snr;
        time_t spottime;

        // callsign,de_pfx,de_cont,freq,band,dx,dx_pfx,dx_cont,mode,db,date,speed,tx_mode
        int got = sscanf(line, "%[^,],%*[^,],%*[^,],%lf,%*[^,],%[^,],%*[^,],%*[^,],%*[^,],%d,%[^,],%*[^,],%s",
            de, &freq, dx, &snr, timestring, mode);

        if (got == 6 ) // If parsing is successful
        {
            (void)strptime(timestring, FMT, &stime);
            spottime = mktime(&stime);

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

            // If SNR is sufficient and frequency OK and mode is right
            if (snr >= minsnr && freq >= MINFREQ && strcmp(mode, spotmode) == 0)
            {
                reference = false;
                // Check if this spot is from a reference skimmer
                for (i = 0; i < referenceskimmers; i++)
                {
                    if (strcmp(de, referenceskimmer[i]) == 0)
                    {
                        reference = true;
                        break;
                    }
                }

                // If it is reference spot, use it to check all un-analyzed,
                // non-reference spots in the pipeline
                if (reference)
                {
                    refspots++;

                    for (i = 0; i < SPOTSWINDOW; i++)
                    {
                        if (!pipeline[i].analyzed &&
                            strcmp(pipeline[i].dx, dx) == 0 &&
                            abs((int)difftime(pipeline[i].time, spottime)) <= maxapart &&
                            !(targeted && strcmp(pipeline[i].de, target) != 0))
                        {
                            int delta = pipeline[i].freq - (int)round(freq * 10.0);
                            int adelta = delta > 0 ? delta : -delta;

                            pipeline[i].analyzed = true; // To only analyze each spot once

                            if (adelta <= MAXERR) // Only consider spots less than MAXERR off from reference skimmer
                            {
                                usedspots++;

                                // Print outliers if in debug mode
                                if (adelta > 2 && verbose && !quiet)
                                {
                                    stime = *localtime(&pipeline[i].time);
                                    (void)strftime(timestring, LINELEN, FMT, &stime);
                                    fprintf(stderr,
                                        "Outlier spot of %8s by %8s at %7.1f (was %7.1f) off by %+3.1f @ %s\n",
                                        pipeline[i].dx, pipeline[i].de, pipeline[i].freq / 10.0,
                                        freq, delta / 10.0, timestring);
                                }

                                // Check if this skimmer is already in list
                                int skimpos = -1;
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
                                    skimmer[skimpos].accdev += pipeline[i].freq / (10.0 * freq);
                                    skimmer[skimpos].count++;
                                    if (pipeline[i].time > skimmer[skimpos].last)
                                        skimmer[skimpos].last = pipeline[i].time;
                                    if (pipeline[i].time < skimmer[skimpos].first)
                                        skimmer[skimpos].first = pipeline[i].time;
                                }
                                else // If new skimmer, add it to list
                                {
                                    if (skimmers >= MAXSKIMMERS) {
                                        fprintf(stderr, "Overflow: More than %d skimmers found.\n", MAXSKIMMERS);
                                        (void)fclose(fp);
                                        return 1;
                                    }
                                    strcpy(skimmer[skimmers].name, pipeline[i].de);
                                    skimmer[skimmers].accdev = pipeline[i].freq / (10.0 * freq);
                                    skimmer[skimmers].count = 1;
                                    skimmer[skimmers].first = pipeline[i].time;
                                    skimmer[skimmers].last = pipeline[i].time;
                                    skimmer[skimmers].reference = pipeline[i].reference;
                                    skimmers++;
                                    if (verbose && !quiet)
                                        fprintf(stderr, "Found skimmer #%d: %s \n", skimmers, pipeline[i].de);
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
    for (i = 0; i < skimmers; i++)
    {
        skimmer[i].avdev = 1000000.0 * (skimmer[i].accdev / skimmer[i].count - 1.0);
        skimmer[i].absavdev = fabs(skimmer[i].avdev);
    }

    // Sort by callsign, or average deviation if desired
    for (i = 0; i < skimmers - 1; ++i)
    {
        for (j = 0; j < skimmers - 1 - i; ++j)
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

    if (isatty(STDOUT_FILENO) == 0 && !forweb)
        printf("Skimmer accuracy analysis based on RBN offline data.\n\n");

    // List reference skimmers
    // strcpy(outstring, "Reference skimmers: ");
    // printf("%s", outstring);
    // int column = (int)strlen(outstring);
    // for (i = 0; i < referenceskimmers; i++)
    // {
        // snprintf(outstring, LINELEN, i == referenceskimmers - 1 ? "and %s" : "%s, ", referenceskimmer[i]);
        // printf("%s", outstring);
        // column += strlen(outstring);
        // if (column > 60 && i < referenceskimmers - 1)
        // {
            // printf("\n");
            // column = 5;
        // }
    // }
    // printf(".\n\n");

    // Print results
    char firsttimestring[TSLEN], lasttimestring[TSLEN];
    stime = *localtime(&firstspot);
    (void)strftime(firsttimestring, LINELEN, "%Y-%m-%d %H:%M", &stime);
    stime = *localtime(&lastspot);
    (void)strftime(lasttimestring, LINELEN, "%H:%M UTC", &stime);
    snprintf(outstring, LINELEN, "%d RBN spots between %s and %s.\n", totalspots, firsttimestring, lasttimestring);
    printboth(outstring, quiet);

    snprintf(outstring, LINELEN, "%d spots (%.1f%%) were from reference skimmers (*).\n",
        refspots, 100.0 * refspots / totalspots);
    printboth(outstring, quiet);

    if (targeted) {
        stime = *localtime(&skimmer[0].first);
        (void)strftime(firsttimestring, LINELEN, "%Y-%m-%d %H:%M:%S", &stime);
        stime = *localtime(&skimmer[0].last);
        (void)strftime(lasttimestring, LINELEN, "%Y-%m-%d %H:%M:%S", &stime);
        snprintf(outstring, LINELEN, "The selected skimmer produced an average of %.0f qualified spots per hour\nbetween %s and %s.\n",
            3600.0 * skimmer[0].count / difftime(skimmer[0].last, skimmer[0].first), firsttimestring, lasttimestring);
    }
    else
    {
        snprintf(outstring, LINELEN, "Average spot flow was %.0f per minute from %d active %s skimmers.\n",
            60 * totalspots / difftime(lastspot, firstspot), skimmers, spotmode);
    }
    printboth(outstring, quiet);

    int qualskimcount = 0;
    for (i = 0; i < skimmers; i++)
    {
        if (skimmer[i].count >= minspots)
        {
            qualskimcount++;
        }
    }

    if (forweb)
        printf("\n");

    snprintf(outstring, LINELEN, "%d spots from %d skimmers qualified for analysis by meeting\nthe following criteria:\n",
        (targeted && usedspots <= minspots) ? 0 : usedspots, qualskimcount);
    printboth(outstring, quiet);

    if (targeted)
        printboth(" * Spotted by the selected skimmer.\n", quiet);

    snprintf(outstring, LINELEN, " * Mode of spot is %s.\n" , spotmode);
    printboth(outstring, quiet);

    snprintf(outstring, LINELEN, " * Also spotted by a reference skimmer within %d spots.\n", SPOTSWINDOW);
    printboth(outstring, quiet);

    snprintf(outstring, LINELEN, " * Also spotted by a reference skimmer within %ds. \n", maxapart);
    printboth(outstring, quiet);

    snprintf(outstring, LINELEN, " * SNR is %ddB or higher. \n", minsnr);
    printboth(outstring, quiet);

    snprintf(outstring, LINELEN, " * Frequency is %dkHz or higher. \n", MINFREQ);
    printboth(outstring, quiet);

    snprintf(outstring, LINELEN, " * Frequency deviation from reference skimmer is %.1fkHz or less.\n", MAXERR / 10.0);
    printboth(outstring, quiet);

    snprintf(outstring, LINELEN, " * At least %d spots from same skimmer in data set.\n", minspots);
    printboth(outstring, quiet);

    (void)time(&stoptime);

    if (forweb)
    {
        printf("\n");
    }
    else if (isatty(STDOUT_FILENO) == 0)
    {
        snprintf(outstring, LINELEN, "Total processing time was %.0f seconds.\n\n", difftime(stoptime, starttime));
        printboth(outstring, quiet);
    }

    // Present results for each skimmer
    printf("  Skimmer    [ppm]  Spots    Adjustment \n");
    printf("  -------------------------------------\n");

    for (i = 0; i < skimmers; i++)
    {
        if (skimmer[i].count >= minspots)
        {
            printf("# %-10s %+5.1f %6d %13.9f\n",
                strcat(skimmer[i].name, skimmer[i].reference ? "*" : ""), 
                skimmer[i].avdev, skimmer[i].count, skimmer[i].accdev / skimmer[i].count);
        }
    }

    if (forweb)
    {
        strftime(outstring, LINELEN, "%Y-%m-%d %H:%M:%S UTC", gmtime(&stoptime));
        printf("\nLast updated %s\n", outstring);
    }

    return 0;
}
