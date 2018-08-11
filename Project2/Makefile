CFLAGS = -Wall -lm -std=c99

all: server

server: ftserver.c
	gcc -g $(CFLAGS) -o ftserver ftserver.c 	

clean:
	rm -f *.o
	rm -f client

