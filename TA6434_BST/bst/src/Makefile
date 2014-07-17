# Makefile: build prog executable when user executes "make" 
# *********************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -Werror -g

# *********************************************************
# Entries to bring the executable up to date

prog: prog.o lib.o
	$(CC) $(CFLAGS) -o prog prog.o lib.o

prog.o: prog.c lib.h
	$(CC) $(CFLAGS) -c prog.c

lib.o: lib.c lib.h
	$(CC) $(CFLAGS) -c lib.c

clean:
	rm -f prog *.o *~



