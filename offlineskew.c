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
#define USAGE "Usage: %s -f filename -t target_call -d \n"

int main(int argc, char *argv[]) {
	char *referenceskimmer[10] = {"JF2IWL", "AC0C", "WB6BEE" ,"SM7IUN", 
		"DF4UE", "K9IMM", "NW0W", "KM3T", "N2QT", "DF7GB" };

    FILE *fp;
	char targetcall[STRLEN] = "", filename[STRLEN] = "";
	int goldencount = 0, spotcount = 0, c, got, i, matches;
	time_t starttime, stoptime;
	struct tm *timeinfo, spottime;
	bool verbose = false, reference, targeted;
	char line[LINELEN], de[STRLEN], dx[STRLEN], timestring[STRLEN];
	int snr, delta, adelta, targetcount = 0, badcount = 0;
	float freq, accreldiff = 0.0;
	char flag;
	int referenceskimmers = sizeof(*referenceskimmer);
	
	struct Spot 
	{
		char de[STRLEN];// Skimmer callsign
		char dx[STRLEN];// Spotted call
		time_t time;	// Spot timestamp in epoch format
		int snr;		// SNR for spotcount
		int freq;		// 10x spot frequency
		bool reference;	// Originates from a reference skimmer
		bool analyzed; 	// Already analyzed
	};
	
	struct Spot pipeline[SPOTSWINDOW];
	int spp = 0;
	
	for (i = 0; i < SPOTSWINDOW; i++)
		pipeline[i].freq = 0;
	
    while ((c = getopt(argc, argv, "t:df:")) != -1)
    {
        switch (c)
        {
            case 'f':
                strcpy(filename, optarg);
                break;
            case 't':
                strcpy(targetcall, optarg);
				for (i = 0; i < strlen(targetcall); i++)
					targetcall[i] = toupper(targetcall[i]);
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

    targeted = strlen(targetcall) != 0;
	
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
					if (!targeted || strcmp(targetcall, de) != 0)
					{
						reference = true;
						break;
					}
				}
			}

			if (reference) // If it is reference spot, use it to verify all un-analyzed, non-reference spots in the pipeline
			{
				if (verbose && false) 
					fprintf(stderr, "Reference spot of %8s by %8s\n", dx, de);
				for (i = 0; i < SPOTSWINDOW; i++)
				{
					if ((strcmp(pipeline[i].dx, dx) == 0 && !pipeline[i].analyzed && !pipeline[i].reference) && 
						((targeted && strcmp(pipeline[i].de, targetcall) == 0) || !targeted))
					{
						delta = pipeline[i].freq - (int)round(freq * 10.0);
						adelta = delta > 0 ? delta : -delta;
						if (adelta < 5)
						{
							if ((adelta >= (targeted ? 0 : 3)) && (strcmp(pipeline[i].de, targetcall) == 0 || !targeted))
							{
								spottime = *localtime(&pipeline[i].time);
								(void)strftime(timestring, STRLEN, FMT, &spottime);
								printf("%s %8s by %8s at %7.1f (was %7.1f) off by %+3.1f @ %s\n",
									adelta != 0 ? "Deviating spot of" : "Accurate spot of ", 
									pipeline[i].dx, pipeline[i].de, pipeline[i].freq / 10.0, 
									freq, delta / 10.0, timestring);
							}
							pipeline[i].analyzed = true; // To only analyze each spot once
							accreldiff += 100000.0 * delta / freq; // Average the relative deviation in pm
							targetcount++;
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

			(void)strptime(timestring, FMT, &spottime);
			pipeline[spp].time = mktime(&spottime);
		
			if (verbose && false)
			{
				spottime = *localtime(&pipeline[spp].time);
				(void)strftime(timestring, STRLEN, FMT, &spottime);
				fprintf(stderr, "[%6d] DE=%8s DX=%8s QRG=%8.1f SNR=%2ddB TIME=%s\n",
					spp, pipeline[spp].de, pipeline[spp].dx, 
					pipeline[spp].freq / 10.0, pipeline[spp].snr, timestring);
				fprintf(stderr, "%s\n", line);
			}
			spp = (spp + 1) % SPOTSWINDOW;
		}
	}

	if (targeted)
	{
		char outstring[LINELEN];
		sprintf(outstring, "---\nAverage deviation for skimmer %s is %+3.2fppm measured over %d spots.\nSuggested calibration adjustment factor is %10.9f\n", 
			targetcall, accreldiff / targetcount, targetcount, 1.0 + accreldiff / (targetcount * 1000000.0));
		if (!isatty(STDOUT_FILENO))
			fprintf(stderr, "%s", outstring);
		printf("%s", outstring);
	}
		
	if (verbose && false) 
	{
		spp = (spp + SPOTSWINDOW - 1) % SPOTSWINDOW; // Back up to last position in buffer
		fprintf(stderr, "Done reading %d spots of which %d reference spots\n", spotcount, goldencount);
		for (i = 0; i < SPOTSWINDOW; i++) 
		{
			spottime = *localtime(&pipeline[i].time);
			(void)strftime(timestring, STRLEN, FMT, &spottime);
			fprintf(stderr, "[%c%03d] DE=%8s DX=%8s QRG=%8.1f SNR=%2ddB TIME=%s\n",
				i == spp ? '*': ' ', i, pipeline[i].de, pipeline[i].dx, 
				pipeline[i].freq / 10.0, pipeline[i].snr, timestring);
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