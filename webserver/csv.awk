BEGIN {
	printf("# Calculated %s\n", strftime("%Y-%m-%d %H:%M:%S"));
	printf("Callsign,Skew,Spots,Correction factor\n");
}
{
    if ($1 == "#") {
        printf("%s,%s,%s,%s\n", $2, $3, $4, $5);
    }
}
END {}
