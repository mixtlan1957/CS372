CS 372 Program 1 Readme

Program 1 is a simple chat system that works for one pair of users: a chat server and a chat client.

Note: do not include quotes listed in instructions, they are used to denote commands only

Compilation instructions (client portion):
> Make sure to have the Makefile in the same location as "chatclient.c"
> To compile enter "make all"

To Run:
1. Open two different putty instances

2. On one instance start server by entering: "./chatserve.py <port#>"
(As per assignment instructions it is recommended to avoid well-known port numbers)
If a port is already in use, user will be prompted to try on a different port.

3. On a console instance where client will be run, compile the client by entering "make all" if not done so already

4. Run client by entering "./client <server-hostname> <port#> where the port number is the same number
entered in step 2.
Note that the <server-hostname> is "localhost" if the client and server are being run on the same server (e.i. both being run on flip2) otherwise you will need to be on the school network and enter the full server name, (e.g. flip2.engr.oregonstate.edu)

5. To communicate, user on client must first enter a handle of length no greater than 10 characters long

6. User on client then initiates the conversation by entering a message.

7. When the server receives the message, the client will wait for a response from the server.

8. These last two steps will be repeated until either the client or the server can terminate the connection by entering "\quit" 
("\quit" command sent on either side will cause the client to terminate, however server will continue running).

9. To stop server send a SIGNIT by entering the command CTRL+C  
