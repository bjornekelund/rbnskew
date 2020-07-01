gcc = gcc -ggdb

offlineskew:	offlineskew.o  Makefile
	$(gcc) -o offlineskew offlineskew.o -lm

offlineskew.o:	offlineskew.c Makefile
	$(gcc) -c offlineskew.c

clean	:
	rm -f *.o
