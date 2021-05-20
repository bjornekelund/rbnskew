#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define STRLEN 32
#define LINELEN 256
#define USAGE "Usage: %s -f filename\n"
#define FMT "%Y-%m-%d %H:%M:%S"
#define FMT1 "%Y-%m-%d %H:%M"
#define FMT2 "%H:%M UTC"

#define CONTINENTS {"AF", "AS", "EU", "NA", "OC", "SA" }
#define MAXCONT 6

// #define BANDS { "6m", "10m", "12m", "15m", "17m", "20m", "30m", "40m", "60m", "80m", "160m" }
#define BANDS { "160m", "80m", "60m", "40m", "30m", "20m", "17m", "15m", "12m", "10m", "6m" }
#define MAXBANDS 11

#define MAXCALLS 100000
#define MAXSKIMMERS 500

int main(int argc, char *argv[])
{
    FILE   *fp;
    char   filename[LINELEN] = "", line[LINELEN] = "";
    int c, totalspots = 0;
    time_t firstspot, lastspot;
    struct tm stime;

    struct Counter {
        char call[MAXCALLS][STRLEN];
        char skimmer[MAXSKIMMERS][STRLEN];
        int skimcount;
        int callcount;
    };

    static char bandname[MAXBANDS][STRLEN] = BANDS;
    static char contname[MAXCONT][STRLEN] = CONTINENTS;

    // List of all skimmers found
    static char skimarray[MAXSKIMMERS][STRLEN];
    int totalskimmers = 0;

    // List of all calls found
    static char callarray[2 * MAXCALLS][STRLEN];
    int totalcalls = 0;

    static struct Counter contbandarray[MAXCONT][MAXBANDS];
    static struct Counter contarray[MAXCONT];
    static struct Counter bandarray[MAXBANDS];

    int strindex(char *string, char array[][STRLEN], int size)
    {
        for (int i = 0; i < size; i++)
        {
            if (strcmp(string, array[i]) == 0)
            {
                return i;
            }
        }
        return -1;
    }

    for (int bi = 0; bi < MAXBANDS; bi++)
    {
        contarray[bi].skimcount = 0;
        contarray[bi].callcount = 0;
        for (int ci = 0; ci < MAXCONT; ci++)
        {
            contbandarray[ci][bi].skimcount = 0;
            contbandarray[ci][bi].callcount = 0;
            bandarray[bi].skimcount = 0;
            bandarray[bi].callcount = 0;
        }
    }

    while ((c = getopt(argc, argv, "f:")) != -1)
    {
        switch (c)
        {
            case 'f': // Filename
                strcpy(filename, optarg);
                break;
            case '?':
            default:
                fprintf(stderr, USAGE, argv[0]);
                return 1;
        }
    }

    if (strlen(filename) == 0)
    {
        fprintf(stderr, USAGE, argv[0]);
        return 1;
    }

    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        fprintf(stderr, "Can not open file \"%s\". Abort.\n", filename);
        return 1;
    }

    while (fgets(line, LINELEN, fp) != NULL)
    {
        char band[STRLEN], decont[STRLEN], dxcall[STRLEN], dxcont[STRLEN], mode[STRLEN],
             timestring[LINELEN], decall[STRLEN];
        int snr;
        double freq;
//        time_t spottime = 0;
//        int year, month;


        int got = sscanf(line, "%[^,],%*[^,],%[^,],%lf,%[^,],%[^,],%*[^,],%[^,],%*[^,],%d,%[^,],%*[^,],%s",
            decall, decont, &freq, band, dxcall, dxcont, &snr, timestring, mode);

        if (got == 9) // If parsing is successful
        {
            int bandindex = strindex(band, bandname, MAXBANDS);
            int decontindex = strindex(decont, contname, MAXCONT);
            int dxcontindex = strindex(dxcont, contname, MAXCONT);

            (void)strptime(timestring, FMT, &stime);
            stime.tm_isdst = 0;
            time_t spottime = mktime(&stime);

//            fprintf(stderr, "timestring=%s spottime=%ld year=%d mon=%d day=%d hour=%d min=%d sec=%d\n",
//                    timestring, spottime, stime.tm_year + 1900, stime.tm_mon + 1, stime.tm_mday, stime.tm_hour,
//                    stime.tm_min, stime.tm_sec);

            if (totalspots == 0)
            {
                firstspot = spottime;
                lastspot = spottime;
            }
            else
            {
                if (spottime > lastspot)
                    lastspot = spottime;
                if (spottime < firstspot)
                    firstspot = spottime;
            }

            totalspots++;

            if (dxcontindex != -1 && decontindex != -1 && bandindex != -1)
            {
                // Check if new dx call for continent
                if (strindex(dxcall, contarray[dxcontindex].call, contarray[dxcontindex].callcount) == -1)
                {
                    strcpy(contarray[dxcontindex].call[contarray[dxcontindex].callcount++], dxcall);
                }

                // Check if new dx call for band
                if (strindex(dxcall, bandarray[bandindex].call, bandarray[bandindex].callcount) == -1)
                {
                    strcpy(bandarray[bandindex].call[bandarray[bandindex].callcount++], dxcall);
                }

                // Check if new dx call for band and continent
                if (strindex(dxcall, contbandarray[dxcontindex][bandindex].call, contbandarray[dxcontindex][bandindex].callcount) == -1)
                {
                    strcpy(contbandarray[dxcontindex][bandindex].call[contbandarray[dxcontindex][bandindex].callcount++], dxcall);
                }

                // Check if new dx call
                if (strindex(dxcall, callarray, totalcalls) == -1)
                {
                    strcpy(callarray[totalcalls++], dxcall);
                }

                // Check if new skimmer for continent
                if (strindex(decall, contarray[decontindex].skimmer, contarray[decontindex].skimcount) == -1)
                {
                    strcpy(contarray[decontindex].skimmer[contarray[decontindex].skimcount++], decall);
                }

                // Check if new skimmer for band
                if (strindex(decall, bandarray[bandindex].skimmer, bandarray[bandindex].skimcount) == -1)
                {
                    strcpy(bandarray[bandindex].skimmer[bandarray[bandindex].skimcount++], decall);
                }

                // Check if new skimmer for band and continent
                if (strindex(decall, contbandarray[decontindex][bandindex].skimmer, contbandarray[decontindex][bandindex].skimcount) == -1)
                {
                    strcpy(contbandarray[decontindex][bandindex].skimmer[contbandarray[decontindex][bandindex].skimcount++], decall);
                }

                // Check if new skimmer for any band
                if (strindex(decall, skimarray, totalskimmers) == -1)
                {
                    strcpy(skimarray[totalskimmers++], decall);
                }
            }
            else
            {
                // printf("Ignored: %s", line);
            }
        }
    }

    (void)fclose(fp);

    char firsttimestring[LINELEN], lasttimestring[LINELEN];
    stime = *localtime(&firstspot);
    (void)strftime(firsttimestring, LINELEN, FMT1, &stime);
    stime = *localtime(&lastspot);
    (void)strftime(lasttimestring, LINELEN, FMT2, &stime);

    printf("RBN activity between %s and %s.\n", firsttimestring, lasttimestring);
    printf("%d spots with %d unique callsigns from %d active skimmers.\n",
        totalspots, totalcalls, totalskimmers);

    printf("\n        Unique callsigns spotted per continent and band\n");
    printf("---------------------------------------------------------------\n");

    #define LEFTCOL "%-3s"
    #define COLS "%5s"
    #define COLN "%5d"

    printf(LEFTCOL, "");
    for (int bi = 0; bi < MAXBANDS; bi++)
    {
        if (bandarray[bi].callcount > 9999) printf(" ");
        printf(COLS, bandname[bi]);
    }
    printf("\n");

    int longcont = 0;
    for (int ci = 0; ci < MAXCONT; ci++)
    {
        longcont |= contarray[ci].callcount > 9999;
    }

    for (int ci = 0; ci < MAXCONT; ci++)
    {
        printf(LEFTCOL, contname[ci]);
        for (int bi = 0; bi < MAXBANDS; bi++)
        {
            if (bandarray[bi].callcount > 9999) printf(" ");
            printf(COLN, contbandarray[ci][bi].callcount);
        }
        if (longcont) printf(" ");
        printf(COLN, contarray[ci].callcount);
        printf("\n");
    }

    printf(LEFTCOL, "");
    for (int bi = 0; bi < MAXBANDS; bi++)
    {
        if (bandarray[bi].callcount > 9999) printf(" ");
        printf(COLN, bandarray[bi].callcount);
    }
    printf("\n");

    printf("\n        Active skimmers per continent and band\n");
    printf("---------------------------------------------------------------\n");
    printf(LEFTCOL, "");
    for (int bi = 0; bi < MAXBANDS; bi++)
    {
        if (bandarray[bi].callcount > 9999) printf(" ");
        printf(COLS, bandname[bi]);
    }
    printf("\n");

    for (int ci = 0; ci < MAXCONT; ci++)
    {
        printf(LEFTCOL, contname[ci]);
        for (int bi = 0; bi < MAXBANDS; bi++)
        {
            if (bandarray[bi].callcount > 9999) printf(" ");
            printf(COLN, contbandarray[ci][bi].skimcount);
        }
        if (longcont) printf(" ");
        printf(COLN, contarray[ci].skimcount);
        printf("\n");
    }

    printf(LEFTCOL, "");
    for (int bi = 0; bi < MAXBANDS; bi++)
    {
        if (bandarray[bi].callcount > 9999) printf(" ");
        printf(COLN, bandarray[bi].skimcount);
    }
    printf("\n");

    return 0;
}
