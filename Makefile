cc			= gcc
gcc			= ${cc} -Wall -ggdb
lint		= cppcheck

all: 		rbnskew cunique

cunique:	cunique.o Makefile
			$(gcc) -o cunique cunique.o -lm

rbnskew:	rbnskew.o Makefile
			$(gcc) -o rbnskew rbnskew.o -lm

cunique.o:	cunique.c Makefile
			$(gcc) -c cunique.c

rbnskew.o:	rbnskew.c Makefile
			$(gcc) -c rbnskew.c

clean:
			rm -f *.o

lint:
			${lint} rbnskew.c
