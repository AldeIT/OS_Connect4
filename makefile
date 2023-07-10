
CC=gcc

F4: F4Server.c F4Client.c
	$(CC) F4Server.c functions.c -o F4Server -Wall
	$(CC) F4Client.c functions.c -o F4Client -Wall
