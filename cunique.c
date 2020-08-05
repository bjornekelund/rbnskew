#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define STRLEN 16
#define LINELEN 128
#define USAGE "Usage: %s -f filename\n"
#define FMT "%Y-%m-%d %H:%M:%S"

#define CONTINENTS {"AF", "AS", "EU", "NA", "OC", "SA" }
#define MAXCONT 6

#define BANDS { "6m", "10m", "12m", "15m", "17m", "20m", "30m", "40m", "60m", "80m", "160m" }
#define MAXBANDS 11

#define MAXCALLS 12000
#define MAXSKIMMERS 400

int main(int argc, char *argv[])
{
    FILE   *fp;
    char   filename[LINELEN] = "", line[LINELEN] = "";
    int c, totalspots = 0;
    time_t firstspot, lastspot;
    struct tm stime;

    struct Bandcount {
        char call[MAXCALLS][STRLEN];
        char skimmer[MAXSKIMMERS][STRLEN];
        int skimcount;
        int callcount;
    };

    struct Contcount {
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
    static char callarray[4 * MAXCALLS][STRLEN];
    int totalcalls = 0;

    static struct Bandcount bandarray[MAXCONT][MAXBANDS];

    static struct Contcount contarray[MAXCONT];

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
            bandarray[ci][bi].skimcount = 0;
            bandarray[ci][bi].callcount = 0;
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

        // printf("%s", line);

        int got = sscanf(line, "%[^,],%*[^,],%[^,],%lf,%[^,],%[^,],%*[^,],%[^,],%*[^,],%d,%[^,],%*[^,],%s",
            decall, decont, &freq, band, dxcall, dxcont, &snr, timestring, mode);

        if (got == 9) // If parsing is successful
        {
            int bandindex = strindex(band, bandname, MAXBANDS);
            int decontindex = strindex(decont, contname, MAXCONT);
            int dxcontindex = strindex(dxcont, contname, MAXCONT);

            (void)strptime(timestring, FMT, &stime);
            time_t spottime = mktime(&stime);

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
                    // printf("New call %s\n", dxcall);
                    strcpy(contarray[dxcontindex].call[contarray[dxcontindex].callcount++], dxcall);
                }

                // Check if new dx call for band and continent
                if (strindex(dxcall, bandarray[dxcontindex][bandindex].call, bandarray[dxcontindex][bandindex].callcount) == -1)
                {
                    // printf("New call %s\n", dxcall);
                    strcpy(bandarray[dxcontindex][bandindex].call[bandarray[dxcontindex][bandindex].callcount++], dxcall);
                }

                // Check if new dx call for any band
                if (strindex(dxcall, callarray, totalcalls) == -1)
                {
                    strcpy(callarray[totalcalls++], dxcall);
                }

                // Check if new skimmer for continent
                if (strindex(decall, contarray[decontindex].skimmer, contarray[decontindex].skimcount) == -1)
                {
                    // printf("New skimmer %s\n", decall);
                    strcpy(contarray[decontindex].skimmer[contarray[decontindex].skimcount++], decall);
                }

                // Check if new skimmer for band and continent
                if (strindex(decall, bandarray[decontindex][bandindex].skimmer, bandarray[decontindex][bandindex].skimcount) == -1)
                {
                    // printf("New skimmer %s\n", decall);
                    strcpy(bandarray[decontindex][bandindex].skimmer[bandarray[decontindex][bandindex].skimcount++], decall);
                }

                // Check if new skimmer for any band
                if (strindex(decall, skimarray, totalskimmers) == -1) 
                {
                    // printf("New skimmer %s\n", decall);
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
    (void)strftime(firsttimestring, LINELEN, FMT, &stime);
    stime = *localtime(&lastspot);
    (void)strftime(lasttimestring, LINELEN, FMT, &stime);

    printf("RBN data between %s and %s.\n", firsttimestring, lasttimestring);
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
        printf(COLS, bandname[bi]);
    }
    printf(COLS, "Tot");
    printf("\n");

    for (int ci = 0; ci < MAXCONT; ci++)
    {
        printf(LEFTCOL, contname[ci]);
        for (int bi = 0; bi < MAXBANDS; bi++)
        {
            printf(COLN, bandarray[ci][bi].callcount);
        }
        printf(COLN, contarray[ci].callcount);
        printf("\n");
    }

    printf("\n        Active skimmers per continent and band\n");
    printf("---------------------------------------------------------------\n");
    printf(LEFTCOL, "");
    for (int bi = 0; bi < MAXBANDS; bi++)
    {
        printf(COLS, bandname[bi]);
    }
    printf(COLS, "Tot");
    printf("\n");

    for (int ci = 0; ci < MAXCONT; ci++)
    {
        printf(LEFTCOL, contname[ci]);
        for (int bi = 0; bi < MAXBANDS; bi++)
        {
            printf(COLN, bandarray[ci][bi].skimcount);
        }
        printf(COLN, contarray[ci].skimcount);
        printf("\n");
    }
    printf("\n");

    return 0;
}
