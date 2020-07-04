gcc = gcc -Wall

rbnskew:	rbnskew.o Makefile
		$(gcc) -o rbnskew rbnskew.o -lm

rbnskew.o:	rbnskew.c Makefile
		$(gcc) -c rbnskew.c

clean:
		rm -f *.o
