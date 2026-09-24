cc		= gcc
gcc		= ${cc} -Wall
lint		= cppcheck

all:		rbnskew

rbnskew:	rbnskew.c Makefile
		$(gcc) -o rbnskew rbnskew.c -lm

clean:
		rm -f *.o *~ rbnskew
		rm -f rbndata/2*.txt rbndata/rbndata.* webfiles/*.txt webfiles/done webfiles/rbnskew.csv

lint:
		${lint} rbnskew.c
