# *********************************************************
# Makefile: build test executable when user executes "make" 
# *********************************************************
# Parameters to control Makefile operation

CC = g++
CFLAGS = -Wall -Werror

# *********************************************************
# Entries to bring the executable up to date
# *********************************************************

test: HuffmanCoding.cpp
	$(CC) $(CFLAGS) HuffmanCoding.cpp -o test

clean:
	rm -f test  *~

