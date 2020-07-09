#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>

#define STRLEN 32
#define LINELEN 256
#define USAGE "Usage: %s -f filename\n"
#define FMT "%Y-%m-%d %H:%M:%S"

#define CONTINENTS {"AF", "AS", "EU", "NA", "OC", "SA" }
#define MAXCONT 6

#define BANDS { "6m", "10m", "12m", "15m", "17m", "20m", "30m", "40m", "60m", "80m", "160m" }
#define MAXBANDS 11

#define MAXCALLS 5000
#define MAXSKIMMERS 400

int main(int argc, char *argv[])
{
    FILE   *fp;
    char   filename[STRLEN] = "", line[LINELEN] = "";
    int c;
   
    struct Bandcount {
        char call[MAXCALLS][STRLEN];
        char skimmer[MAXSKIMMERS][STRLEN];
        int skimcount;
        int callcount;
    };
    
    char bandname[MAXBANDS][STRLEN] = BANDS;
    char contname[MAXCONT][STRLEN] = CONTINENTS;
    
    char skimarray[MAXSKIMMERS][STRLEN];
    int totalskimmers = 0;
    char callarray[MAXCALLS * MAXBANDS][STRLEN];
    int totalcalls = 0, totalspots = 0;
    time_t firstspot, lastspot;    
    struct tm stime;

    static struct Bandcount bandarray[MAXCONT][MAXBANDS];

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

    for (int i = 0; i < MAXCONT; i++) 
    {
        for (int j = 0; j < MAXBANDS; j++)
        {
            bandarray[i][j].skimcount = 0;
            bandarray[i][j].callcount = 0;
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

    fp = fopen(filename, "r");
    
    if (fp == NULL) 
    {
        fprintf(stderr, "Can not open file \"%s\". Abort.\n", filename);
        return 1;
    }  

    while (fgets(line, LINELEN, fp) != NULL)
    {
        char band[STRLEN], decont[STRLEN], dxcall[STRLEN], dxcont[STRLEN], mode[STRLEN],
             timestring[STRLEN], decall[STRLEN];
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

    char firsttimestring[STRLEN], lasttimestring[STRLEN];
    stime = *localtime(&firstspot);
    (void)strftime(firsttimestring, STRLEN, FMT, &stime);
    stime = *localtime(&lastspot);
    (void)strftime(lasttimestring, STRLEN, FMT, &stime);

    
    printf("RBN data between %s and %s.\n", firsttimestring, lasttimestring);
    printf("%d spots from %d active skimmers with %d unique calls.\n", totalspots, totalskimmers, totalcalls);
 
    printf("\n        Unique callsigns spotted per continent and band\n");
    #define LEFTCOL "%-3s"
    #define COLS "%5s"
    #define COLN "%5d"
    printf(LEFTCOL, "");
    for (int i = 0; i < MAXBANDS; i++)
    {
        printf(COLS, bandname[i]);
    }
    printf("\n");
    
    for (int i = 0; i < MAXCONT; i++) 
    {
        printf(LEFTCOL, contname[i]);
        for (int j = 0; j < MAXBANDS; j++)
        {
            printf(COLN, bandarray[i][j].callcount);
        }
        printf("\n");
    }

    printf("\n\n        Active skimmers per continent and band\n");
    printf(LEFTCOL, "");
    for (int i = 0; i < MAXBANDS; i++)
    {
        printf(COLS, bandname[i]);
    }
    printf("\n");
    
    for (int i = 0; i < MAXCONT; i++) 
    {
        printf(LEFTCOL, contname[i]);
        for (int j = 0; j < MAXBANDS; j++)
        {
            printf(COLN, bandarray[i][j].skimcount);
        }
        printf("\n");
    }
    printf("\n");




    // Sort by callsign, or average deviation if desired
    // for (i = 0; i < skimmers - 1; ++i)
    // {
        // for (j = 0; j < skimmers - 1 - i; ++j)
        // {
            // if (sort ? (worst ? skimmer[j].absavdev < skimmer[j + 1].absavdev : skimmer[j].absavdev > skimmer[j + 1].absavdev )
                // : strcmp(skimmer[j].name, skimmer[j + 1].name) > 0)
            // {
                // temp = skimmer[j + 1];
                // skimmer[j + 1] = skimmer[j];
                // skimmer[j] = temp;
            // }
        // }
    // }

    // if (isatty(STDOUT_FILENO) == 0 && !forweb)
        // printf("Skimmer accuracy analysis based on RBN offline data.\n\n");

    // // List reference skimmers
    // strcpy(outstring, "Reference skimmers: ");
    // printf("%s", outstring);
    // int column = (int)strlen(outstring);
    // for (i = 0; i < referenceskimmers; i++)
    // {
        // sprintf(outstring, i == referenceskimmers - 1 ? "and %s" : "%s, ", referenceskimmer[i]);
        // printf("%s", outstring);
        // column += strlen(outstring);
        // if (column > 60 && i < referenceskimmers - 1)
        // {
            // printf("\n");
            // column = 5;
        // }
    // }
    // printf(".\n\n");

    // // Print results
    // char firsttimestring[STRLEN], lasttimestring[STRLEN];
    // stime = *localtime(&firstspot);
    // (void)strftime(firsttimestring, STRLEN, FMT, &stime);
    // stime = *localtime(&lastspot);
    // (void)strftime(lasttimestring, STRLEN, FMT, &stime);
    // sprintf(outstring, "%d RBN spots between %s and %s\n", totalspots, firsttimestring, lasttimestring);
    // printboth(outstring, quiet);

    // sprintf(outstring, "processed of which %d spots (%.1f%%) were reference spots.\n", refspots, 100.0 * refspots / totalspots);
    // printboth(outstring, quiet);

    // if (targeted) {
        // stime = *localtime(&skimmer[0].first);
        // (void)strftime(firsttimestring, STRLEN, FMT, &stime);
        // stime = *localtime(&skimmer[0].last);
        // (void)strftime(lasttimestring, STRLEN, FMT, &stime);
        // sprintf(outstring, "The selected skimmer produced an average of %.0f qualified spots per hour\n    between %s and %s.\n", 
            // 3600.0 * skimmer[0].count / difftime(skimmer[0].last, skimmer[0].first), firsttimestring, lasttimestring);
    // }
    // else
    // {
        // sprintf(outstring, "The average total spot flow was %.0f per minute with %d active\n%s skimmers.\n",
        // 60 * totalspots / difftime(lastspot, firstspot), skimmers, spotmode);
    // }
    // printboth(outstring, quiet);

    // int qualskimcount = 0;
    // for (i = 0; i < skimmers; i++)
    // {
        // if (skimmer[i].count >= minspots)
        // {
            // qualskimcount++;
        // }
    // }

    // if (forweb)
        // printf("\n");

    // sprintf(outstring, "%d spots from %d skimmers qualified for analysis by meeting\nthe following criteria:\n",
        // (targeted && usedspots <= minspots) ? 0 : usedspots, qualskimcount);
    // printboth(outstring, quiet);

    // if (targeted)
        // printboth(" * Spotted by the selected skimmer.\n", quiet);

    // sprintf(outstring, " * Mode of spot is %s.\n" , spotmode);
    // printboth(outstring, quiet);

    // sprintf(outstring, " * Also spotted by a reference skimmer within %d most recent spots.\n", SPOTSWINDOW);
    // printboth(outstring, quiet);

    // sprintf(outstring, " * Also spotted by a reference skimmer within %ds. \n", maxapart);
    // printboth(outstring, quiet);

    // sprintf(outstring, " * SNR is %ddB or higher. \n", minsnr);
    // printboth(outstring, quiet);

    // sprintf(outstring, " * Frequency is %dkHz or higher. \n", MINFREQ);
    // printboth(outstring, quiet);

    // sprintf(outstring, " * Frequency deviation from reference skimmer is %.1fkHz or less.\n", MAXERR / 10.0);
    // printboth(outstring, quiet);

    // sprintf(outstring, " * At least %d spots from same skimmer in data set.\n", minspots);
    // printboth(outstring, quiet);

    // (void)time(&stoptime);

    // if (forweb)
    // {
        // printf("\n");
    // }
    // else if (isatty(STDOUT_FILENO) == 0)
    // {
        // sprintf(outstring, "Total processing time was %.0f seconds.\n\n", difftime(stoptime, starttime));
        // printboth(outstring, quiet);
    // }

    // // Present results for each skimmer
    // printf("  Skimmer   [ppm]  Spots    Adjustment \n");
    // printf("  -------------------------------------\n");

    // for (i = 0; i < skimmers; i++)
    // {
        // if (skimmer[i].count >= minspots)
        // {
            // printf("# %-9s %+5.1f %6d %13.9f\n",
                // skimmer[i].name, skimmer[i].avdev, skimmer[i].count, skimmer[i].accdev / skimmer[i].count);
        // }
    // }

    // if (forweb)
    // {
        // strftime(outstring, LINELEN, "%Y-%m-%d %H:%M:%S UTC", gmtime(&stoptime));
        // printf("\nLast updated %s\n", outstring);
    // }

    return 0;
}
