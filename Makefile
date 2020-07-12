cc			= gcc
gcc			= ${cc} -Wall
lint		= cppcheck

all:		rbnskew cunique

cunique:	cunique.c Makefile
			$(gcc) -o cunique cunique.c -lm

rbnskew:	rbnskew.c Makefile
			$(gcc) -o rbnskew rbnskew.c -lm

clean:
			rm -f *.o *~

lint:
			${lint} rbnskew.c
			${lint} cunique.c
