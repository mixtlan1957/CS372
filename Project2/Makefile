CFLAGS = -Wall -lm -std=c99

all: server

server: ftserver.c
	gcc -g $(CFLAGS) -o client ftserver.c 	

clean:
	rm -f *.o
	rm -f client

