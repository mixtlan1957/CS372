CS 372 Program 2 Readme

Program 2 is a simple file transfer system that works for one pair of users: a ft server and a ft client.
The ftclient requests to see the working directory of the ft server and can request for a file to be transfered.

Note: do not include quotes listed in instructions, they are used to denote commands only

Compilation instructions (server portion):
> Make sure to have the Makefile in the same location as "ftserver.c"
> To compile enter "make all"

To Run:
1. Open two different putty instances

2. On console where server instance will be run, compile the client by entering "make all" if not done so already

3. Start server instance by entering "./ftserver <port#: messaging port>
(As per assignment instructions it is recommended to avoid well-known port numbers, if you encounter a port
in use error, simply start again and try a different port).

4. On console where client will be run enter 
./ftclient.py <server-hostname> <port#: messaging port> -l <port #: data transfer port>
to obtain a list of all the files in the server's working directory.

Note that the <server-hostname> is "localhost" if the client and server are being run on the same server (e.i. both being run on flip2) otherwise if contacting engr server from engr (say flip1 to flip2), simply entering
"flip1" or "flip2" will be sufficient as in the homework instructions example execution:
> ftclient flip1 30021 -l 30020

5. To transfer a particular file enter:
./ftclient.py <server-hostname> <port#: messaging port> -g <file name> <port#: data transfer port>

6. Server will continue running until stopped (i.e. via SIGINT: CTRL + C)

