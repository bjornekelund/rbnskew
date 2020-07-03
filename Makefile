gcc = gcc -ggdb

rbnskew:		rbnskew.o Makefile
				$(gcc) -o rbnskew rbnskew.o -lm

rbnskew.o:		rbnskew.c Makefile
				$(gcc) -c rbnskew.c

offlineskew:	offlineskew.o  Makefile
				$(gcc) -o offlineskew offlineskew.o -lm

offlineskew.o:	offlineskew.c Makefile
				$(gcc) -c offlineskew.c

clean:
				rm -f *.o 
