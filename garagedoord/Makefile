##
## Purpose: Makefile to compile the garagedoord
## File:    Makefile
## Author:  smanders
## Date:    Feb 6, 2013
##

CC=gcc
LIBS=-lrt -lbcm2835
CFLAGS=

all: garagedoord

garagedoord: garagedoord.o
	$(CC) $(CFLAGS) -o garagedoord garagedoord.o $(LIBS) 

garagedoord.o: garagedoord.c
	$(CC) $(CFLAGS) -c garagedoord.c

install: garagedoord
	cp garagedoord /usr/sbin/
	cp garagedoord.sh /etc/init.d/
	

clean:
	rm -f garagedoord *.o
