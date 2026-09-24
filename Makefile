cc		= gcc
gcc		= ${cc} -Wall
lint		= cppcheck

all:		rbnskew

rbnskew:	rbnskew.c Makefile
		$(gcc) -o rbnskew rbnskew.c -lm

clean:
		rm -f *.o *~ rbnskew
		rm -f rbnfiles/[0-9]* webfiles/*.txt 

lint:
		${lint} rbnskew.c
