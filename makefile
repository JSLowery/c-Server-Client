# Makefile for socket programs


all: Server Client


Server: Server.c
	gcc -o Server Server.c -lnsl
	cp -p $(HOME)/NetProg/LastServerProj/Server.c $(HOME)/NetProg/LastServerProj/Server.cbu
Client: Client.c
	gcc -o Client Client.c -lnsl
	cp -p $(HOME)/NetProg/LastServerProj/Client.c $(HOME)/NetProg/LastServerProj/Client.cbu
clean:
	/bin/rm -rf Client Server
