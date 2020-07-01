#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#define STRLEN 32
#define LINELEN 128
#define FMT "%Y-%m-%d %H:%M:%S"
#define SPOTSWINDOW 1000
#define USAGE "Usage: %s -f filename -t target_call -d \n"

int main(int argc, char *argv[]) {
	char *goldencall[10] = {"JF2IWL", "AC0C", "WB6BEE" ,"SM7IUN", 
		"DF4UE", "K9IMM", "NW0W", "KM3T", "N2QT", "DF7GB" };
    char filename[STRLEN] = "";

    FILE *fp;
	char targetcall[STRLEN] = "";
	int goldencount = 0, spotcount = 0, c, got, i, matches;
	time_t rawtime;
	struct tm *timeinfo, spottime;
	bool verbose = false, verified, targeted;
	char line[LINELEN], de[STRLEN], dx[STRLEN], timestring[STRLEN];
	int snr, delta, adelta, targetcount = 0, badcount = 0;
	float freq, accreldiff = 0.0;
	char flag;
	// int verifiedthreshold = 10;
	int goldencalls = sizeof(*goldencall);
	
	struct Spot {
		char de[STRLEN];// Skimmer callsign
		char dx[STRLEN];// Spotted call
		time_t time;	// Spot timestamp in epoch format
		int snr;		// SNR for spotcount
		int freq;		// 10x spot frequency
		bool verified;	// Verified boolean
		bool used; 	// If displayed
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
		(void)time(&rawtime);
		timeinfo = localtime(&rawtime);
		fprintf(stderr, "Starting at %s", asctime(timeinfo));
	}
	
	while (fgets(line, LINELEN, fp))
	{
		// callsign,de_pfx,de_cont,freq,band,dx,dx_pfx,dx_cont,mode,db,date,speed,tx_mode
		got = sscanf(line, "%[^,],%*[^,],%*[^,],%f,%*[^,],%[^,],%*[^,],%*[^,],%*[^,],%d,%[^,],%*s", 
			de, &freq, dx, &snr, timestring);
		
		if (got == 5)
		{ 
			spotcount++;

			// matches = 0;
			// for (i = 0; i < SPOTSWINDOW; i++)
			// {
				// if (strcmp(pipeline[i].dx, dx) && pipeline[i].freq == (int)round(freq * 10.0))
					// matches++;
			// }
			// verified = matches > VERIFIEDTHRESHOLD;
			
			verified = false;
			for (i = 0; i < goldencalls; i++)
			{
				if (strcmp(de, goldencall[i]) == 0)
				{
					if (!targeted || strcmp(targetcall, de) != 0)
					{
						verified = true;
						break;
					}
				}
			}

			if (verified)
			{
				if (verbose && false) 
					fprintf(stderr, "Verified spot of %8s by %8s\n", dx, de);
				for (i = 0; i < SPOTSWINDOW; i++)
				{
					if ((strcmp(pipeline[i].dx, dx) == 0 && !pipeline[i].used) && 
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
									adelta != 0 ? "Inaccurate spot of" : "Accurate spot of  ", 
									pipeline[i].dx, pipeline[i].de, pipeline[i].freq / 10.0, 
									freq, delta / 10.0, timestring);
								// badcount++;
							}
							pipeline[i].used = true;
							accreldiff += 100000.0 * delta / freq; // In ppm
							targetcount++;
							// fprintf(stderr, "delta=%d freq=%f accreldiff=%f targetcount=%d\n", delta, freq, accreldiff, targetcount);
						}
					}
				}
			}
		
			strcpy(pipeline[spp].de, de);
			strcpy(pipeline[spp].dx, dx);
			pipeline[spp].freq = (int)round(freq * 10.0);
			pipeline[spp].snr = snr;
			pipeline[spp].verified = verified;
			pipeline[spp].used = false;			

			(void)strptime(timestring, FMT, &spottime);
			pipeline[spp].time = mktime(&spottime);
			
			// if (strcmp(de, goldencall) == 0) 
			// {
				// (void)strftime(timestring, STRLEN, FMT, &spottime);
				// goldencount++;
			// }
			
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
		sprintf(outstring, "Average error of skimmer %s is %+3.2fppm over %d spots = calibration factor %10.9f\n", 
			targetcall, accreldiff / targetcount, targetcount, 1.0 + accreldiff / (targetcount * 1000000.0));
		fprintf(stderr, "%s", outstring);
		printf("%s", outstring);
	}
		
	if (verbose && false) 
	{
		spp = (spp + SPOTSWINDOW - 1) % SPOTSWINDOW; // Back up to last position in buffer
		fprintf(stderr, "Done reading %d spots of which %d golden spots\n", spotcount, goldencount);
		for (i = 0; i < SPOTSWINDOW; i++) 
		{
			spottime = *localtime(&pipeline[i].time);
			(void)strftime(timestring, STRLEN, FMT, &spottime);
			fprintf(stderr, "[%c%03d] DE=%8s DX=%8s QRG=%8.1f SNR=%2ddB TIME=%s\n",
				i == spp ? '*': ' ', i, pipeline[i].de, pipeline[i].dx, 
				pipeline[i].freq / 10.0, pipeline[i].snr, timestring);
		}

	}

	(void)time(&rawtime);
	timeinfo = localtime(&rawtime);
	fprintf(stderr, "Stopped at %s", asctime(timeinfo));

	(void)fclose(fp);

	return 0;
}