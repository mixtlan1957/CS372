CFLAGS = -Wall -lm -std=c99

chatclient.o:
	gcc -c chatclient.c -g $(CFLAGS)

client: chatclient.o
	gcc -o client chatclient.o $(CFLAGS) 	



clean:
	rm -f *.o
	rm -f client

