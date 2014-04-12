# Makefile for List ADT
#
# make             makes ListTest and Shuffle
# make clean       removes object files
#

All : Server.o Client.o 
	gcc -o Client Client.o 
	gcc -o Server Server.o

Server.o : Server.c
	gcc -c -Wall Server.c

Client.o : Client.c
	gcc -c -Wall Client.c

clean :
	rm -f Server.o Client.o Server Client 

