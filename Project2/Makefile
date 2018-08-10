CFLAGS = -Wall -lm -std=c99

all: client

client: chatclient.c
	gcc -g $(CFLAGS) -o client chatclient.c 	

clean:
	rm -f *.o
	rm -f client

