cc		= gcc
gcc		= ${cc} -Wall
lint		= cppcheck

all:		rbnskew

rbnskew:	rbnskew.c Makefile
		$(gcc) -o rbnskew rbnskew.c -lm

clean:
		rm -f *.o *~ rbnskew

lint:
		${lint} rbnskew.c
