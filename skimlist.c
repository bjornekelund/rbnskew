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
#define LINELEN 128
#define FMT "%Y-%m-%d %H:%M:%S"
#define SPOTSWINDOW 200
#define MAXSKIMMERS 1000
#define USAGE "Usage: %s -f filename -t target_call -d \n"

int main(int argc, char *argv[]) {
	char *referenceskimmer[10] = {"JF2IWL", "AC0C", "WB6BEE" ,"SM7IUN", 
		"DF4UE", "K9IMM", "NW0W", "KM3T", "N2QT", "DF7GB" };

    FILE *fp;
	char filename[STRLEN] = "";
	int goldencount = 0, spotcount = 0, c, got, i, j, matches, spp = 0;
	time_t starttime, stoptime;
	struct tm *timeinfo, stime;
	bool verbose = false, reference;
	char line[LINELEN], de[STRLEN], dx[STRLEN], timestring[STRLEN];
	char firstimestring[STRLEN], lasttimestring[STRLEN];
	int snr, delta, adelta, skimmers = 0, skimpos;
	float freq, accreldiff = 0.0;
	char flag;
	int referenceskimmers = sizeof(*referenceskimmer);
	
	struct Spot 
	{
		char de[STRLEN];	// Skimmer callsign
		char dx[STRLEN];	// Spotted call
		time_t time;		// Spot timestamp in epoch format
		int snr;			// SNR for spotcount
		int freq;			// 10x spot frequency
		bool reference;		// Originates from a reference skimmer
		bool analyzed; 		// Already analyzed
	};
	
	struct Skimmer 
	{
		char name[STRLEN];	// Skimmer callsign
		double accdev;		// Accumulated deviation in ppm
		int count; 			// Number of analyzed spots
		time_t first; 		// Earliest spot
		time_t last; 		// Latest spot
	};
	
	struct Spot pipeline[SPOTSWINDOW];
	struct Skimmer skimmer[MAXSKIMMERS];
	
	for (i = 0; i < SPOTSWINDOW; i++)
		pipeline[i].freq = 0;
	
    while ((c = getopt(argc, argv, "df:")) != -1)
    {
        switch (c)
        {
            case 'f':
                strcpy(filename, optarg);
                break;
			case 'd':
				verbose = true;
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

	if (verbose) 
	{
		(void)time(&starttime);
		timeinfo = localtime(&starttime);
		fprintf(stderr, "Starting at %s", asctime(timeinfo));
	}
	
	if (!isatty(STDOUT_FILENO))
		printf("Analysis results can be found in last lines of file.\n---\n");

	
	while (fgets(line, LINELEN, fp))
	{
		// callsign,de_pfx,de_cont,freq,band,dx,dx_pfx,dx_cont,mode,db,date,speed,tx_mode
		got = sscanf(line, "%[^,],%*[^,],%*[^,],%f,%*[^,],%[^,],%*[^,],%*[^,],%*[^,],%d,%[^,],%*s", 
			de, &freq, dx, &snr, timestring);
		
		if (got == 5) // If parsing is successful
		{ 
			spotcount++;
		
			reference = false;
			// Check if this spot is by a golden skimmer
			for (i = 0; i < referenceskimmers; i++)
			{
				if (strcmp(de, referenceskimmer[i]) == 0)
				{
					reference = true;
					break;
				}
			}

			if (reference) // If it is reference spot, use it to verify all un-analyzed, non-reference spots in the pipeline
			{
				if (verbose && false) 
					fprintf(stderr, "Reference spot of %8s by %8s\n", dx, de);
				
				for (i = 0; i < SPOTSWINDOW; i++)
				{
					if (strcmp(pipeline[i].dx, dx) == 0 && !pipeline[i].analyzed && !pipeline[i].reference)
					{
						delta = pipeline[i].freq - (int)round(freq * 10.0);
						adelta = delta > 0 ? delta : -delta;

						pipeline[i].analyzed = true; // To only analyze each spot once
						
						if (adelta < 5) // Only consider spots less than 0.5kHz off from reference skimmer
						{
							if (adelta > 1 && verbose) // Print if very deviating
							{
								stime = *localtime(&pipeline[i].time);
								(void)strftime(timestring, STRLEN, FMT, &stime);
								printf("%s %8s by %8s at %7.1f (was %7.1f) off by %+3.1f @ %s\n",
									adelta != 0 ? "Deviating spot of" : "Accurate spot of ", 
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
							
							if (skimpos != -1) // if in the list
							{
								skimmer[skimpos].accdev += 100000.0 * delta / freq;
								skimmer[skimpos].count++;
								if (pipeline[i].time > skimmer[skimpos].last)
									skimmer[skimpos].last = pipeline[i].time;
								if (pipeline[i].time < skimmer[skimpos].first)
									skimmer[skimpos].first = pipeline[i].time;
							}
							else // If new skimmer
							{
								if (verbose)
									fprintf(stderr, "Found new skimmer %s \n", pipeline[i].de);
								strcpy(skimmer[skimmers].name, pipeline[i].de);
								skimmer[skimmers].accdev = 100000.0 * delta / freq;
								skimmer[skimmers].count = 1;
								skimmer[skimmers].first = pipeline[i].time;
								skimmer[skimmers].last = pipeline[i].time;
								skimmers++;
							}
						}
					}
				}
			}
		
			strcpy(pipeline[spp].de, de);
			strcpy(pipeline[spp].dx, dx);
			pipeline[spp].freq = (int)round(freq * 10.0);
			pipeline[spp].snr = snr;
			pipeline[spp].reference = reference;
			pipeline[spp].analyzed = false;			

			(void)strptime(timestring, FMT, &stime);
			pipeline[spp].time = mktime(&stime);
		
			if (verbose && false)
			{
				stime = *localtime(&pipeline[spp].time);
				(void)strftime(timestring, STRLEN, FMT, &stime);
				fprintf(stderr, "[%6d] DE=%8s DX=%8s QRG=%8.1f SNR=%2ddB TIME=%s\n",
					spp, pipeline[spp].de, pipeline[spp].dx, 
					pipeline[spp].freq / 10.0, pipeline[spp].snr, timestring);
				fprintf(stderr, "%s\n", line);
			}
			spp = (spp + 1) % SPOTSWINDOW;
		}
	}

	for (i = 0; i < skimmers; i++)
	{
		if (skimmer[i].count > 50)
		{
			stime = *localtime(&skimmer[i].first);
			(void)strftime(firstimestring, STRLEN, FMT, &stime);

			stime = *localtime(&skimmer[i].last);
			(void)strftime(lasttimestring, STRLEN, FMT, &stime);

			fprintf(stderr, "Skimmer %10s average deviation %+6.2fppm over %4d spots between %s and %s\n", 
			skimmer[i].name, skimmer[i].accdev / skimmer[i].count, skimmer[i].count, firstimestring, lasttimestring);
		}
	}
	
	if (verbose)
	{
		(void)time(&stoptime);
		timeinfo = localtime(&stoptime);
		fprintf(stderr, "Execution took %.0f seconds\n", difftime(stoptime, starttime));
	}

	(void)fclose(fp);

	return 0;
}