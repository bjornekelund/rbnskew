gcc = gcc -ggdb

all:			offlineskew skimlist

skimlist:		skimlist.o Makefile
				$(gcc) -o skimlist skimlist.o -lm

skimlist.o:		skimlist.c
				$(gcc) -c skimlist.c

offlineskew:	offlineskew.o  Makefile
				$(gcc) -o offlineskew offlineskew.o -lm

offlineskew.o:	offlineskew.c Makefile
				$(gcc) -c offlineskew.c

clean:
				rm -f *.o
