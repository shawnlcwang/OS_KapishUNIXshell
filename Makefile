
kapish: kapish.o
	gcc kapish.o -o kapish
		
kapish.o: kapish.o
	gcc -c kapish.c -o kapish.o -Wall -Werror
