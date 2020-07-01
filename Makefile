gcc = gcc -ggdb

skew:	skew.o  Makefile
	$(gcc) -o skew skew.o -lm

skew.o:	skew.c Makefile
	$(gcc) -c skew.c
	
clean	:
	rm -f *.o
