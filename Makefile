cc			= gcc
gcc			= ${cc} -Wall -ggdb
lint		= splint -unrecog -warnposix -bufferoverflowhigh -formatconst -compdef -nullpass -usedef

rbnskew:	rbnskew.o Makefile
			$(gcc) -o rbnskew rbnskew.o -lm

rbnskew.o:	rbnskew.c Makefile
			$(gcc) -c rbnskew.c

clean:
			rm -f *.o

lint:
			${lint} rbnskew.c
