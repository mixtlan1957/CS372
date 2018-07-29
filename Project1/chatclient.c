/*********************************************************************
** Program Filename: chatclient.c
** Author: Mario Franco-Munoz
** Due Date: 7/29/2018
** Description:CS 372 Project 1: chatclient file. This file establishes chat comunication
** with chatserve.py that allows for messages for up to 500 characters long to be exchanged.
** (Note: effective message length is 499 characters so as to allow for null terminating character.
*********************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <assert.h>

#define OK 0
#define NO_INPUT 1
#define TOO_LONG 2
#define MSG_LIMIT 500
//for backward compability. 
//https://stackoverflow.com/questions/11405819/does-struct-hostent-have-a-field-h-addr
//I did not have this problem in CS344 no idea why I am having it now
//#define h_addr h_addr_list[0]
//update: apparently defining _GNU_SOURCE also fixes this problem


//function prototypes
int sendMessage(int, char*, char*);
int recvMessage(int, char*);
void initiateContact(char *, char*);
static int getUserInput(char *, char *, size_t sz);


/*The following function gets user input to inter a message or handle name
Source: https://stackoverflow.com/questions/4023895/how-do-i-read-a-string-entered-by-the-user-in-c
*/
static int getUserInput(char *prompt, char *buffer, size_t sz) {
	int ch, extra;

    // Get line with buffer overrun protection.
    if (prompt != NULL) {
        printf ("%s", prompt);
        fflush (stdout);
    }
    if (fgets (buffer, sz, stdin) == NULL)
        return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buffer[strlen(buffer)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

 

    // Otherwise remove newline and give string back to caller.
    buffer[strlen(buffer)-1] = '\0';
    return OK;
}




//sendMessge sends a mesage to the established socket file descriptor and returns
//the number of bits sent.
//input: socket file descriptor 
//output: returns the number of bytes sent. A negative return value indicates there was a send error
int sendMessage(int socketFD, char *handle, char *msg_out) {
	int byteSent = 0;

	//prepend the handle to the message prior to sending
	char *tempStr = malloc(sizeof(char)*512);
	memset(tempStr, 0, 512);

	strcpy(tempStr, handle);
	strcat(tempStr, msg_out);
	strcat(tempStr, "\0");

	//send message on established socket file descriptor
	byteSent = send(socketFD, tempStr, 512, 0);

	free(tempStr);

	return byteSent;
}



//recieveMessage recieves a message on the established socket file descriptor and returns
//the number of bits recieved.
//input: socket file descriptor
//output: returns the number of bytes recieved. A negative return values there was a recv error
int recvMessage(int socketFD, char *readBuffer) {
	int charsRead;

	//call recv to recieve message from server
	charsRead = recv(socketFD, readBuffer, 1024, 0);

	return charsRead;
} 



/* The following is the main driver function for the client. It prompts the user to first
enter a handle name and then prompts the user to enter chat messages until the "\quit" command is entered
Upon user entering "\quit", the client sends this as the last message to the chatserve.py server and exits loop.
*/
void initiateContact(char *hostName, char *portNo) {
	//socket specific variables
	int socketFD, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	int firstMsg = 1;

	//other variables
	int inputCheck = 0;
	int errorFlag = 0;
	char sendMsgBuffer[500];
	char readBuffer[500];

	//convert port number to int
	int portNumber = atoi(portNo);
	char *instr = "Please enter your handle name. (Maximum 10 characters): ";
	char handle[11];
	char displayName[13];
	size_t tmpSz, byteSent;



	//propt user to enter handle
	memset(handle, '\0', sizeof(handle));
	inputCheck = getUserInput(instr, handle, sizeof(handle));
	while(inputCheck != 0) {
		printf("I'm sorry that handle input was invalid please try again.\n");
		inputCheck = getUserInput(instr, handle, sizeof(handle));
	}
	//adjust the handle so that it is displayed as "handle> " on I/O
	memset(displayName, '\0', sizeof(displayName));
	strcpy(displayName, handle);
	strcat(displayName, "> ");	


	//The following code is based on lecture notes from Operating Systems class CS344
	memset((char*)&serverAddress, '\0', sizeof(serverAddress));  //Clear out the address struct
	serverAddress.sin_family = AF_INET;  //create a network-cable socket (IPV4)
	serverAddress.sin_port = htons(portNumber); // Store the port number	
	serverHostInfo = gethostbyname(hostName); // Convert the machine name into a special form of address
	//validate host
	if (serverHostInfo == NULL) { 
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		errorFlag = 1; 
		goto cleanup;
	}
	//copy in the address
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

	//Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) { 
		fprintf(stderr, "CLIENT: ERROR opening socket\n");
		errorFlag = 1;
		goto cleanup;
	}


	//connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { // Connect socket to address
		fprintf(stderr, "CLIENT: ERROR connecting\n");
		errorFlag = 1;
		goto cleanup;
	}



	//loop indefinetly until client enters "\quit" or server sends "\quit"
	while(1) {

		//get message to send to server unless this is this is the first message being sent
		//in which case PORTNUM needs to be sent
		tmpSz = sizeof(sendMsgBuffer);
		memset(sendMsgBuffer, '\0', tmpSz);
		if (firstMsg != 1) {
			inputCheck = getUserInput(displayName, sendMsgBuffer, tmpSz);
		}
		else {
			strcpy(sendMsgBuffer, portNo);
			firstMsg = 0;
		}

		//recieve and validate user input.
		if (inputCheck != OK) {
			fprintf(stderr, "CLIENT: Bad string input.\n");
			continue;
		}

		
		//send message
		byteSent = 0;
		byteSent = sendMessage(socketFD, displayName, sendMsgBuffer);
		if (byteSent < 0) {
			fprintf(stderr, "CLIENT: Send ERROR\n");
			errorFlag = 1;
			goto cleanup;
		}

		//check if quit command was entered
		//exit loop after sending '\quit' command if quit command was entered
		if (strncmp(sendMsgBuffer, "\\quit", 5) == 0) {
			break;
		}


		//recieve message
		tmpSz = sizeof(readBuffer);
		memset(readBuffer, '\0', tmpSz);
		charsRead = recvMessage(socketFD, readBuffer);

		
		//error handling
		if (charsRead == -1) {
			fprintf(stderr, "Error recieving message from server\n");
			errorFlag = 1;
			goto cleanup;
		}

		//display message
		printf("%s\n", readBuffer);
		fflush(stdout);

		//exit if necessary
		if (strstr(readBuffer, "\\quit") != NULL) {
			break;
		}

	}

	cleanup:
	//if error flag was set exit with value of 1
	if (errorFlag == 1) {
		exit(1);
	}


	//close the connection
	close(socketFD);
}

int main(int argc, char *argv[]) {
	//make sure that two arguments are being passed: program name, port number
	if (argc != 3) {
		fprintf(stderr, "Incorrect number of arguments passed to function. "); 
		fprintf(stderr, "Please make sure host name followed by port number is included.\n");
	}
	else {
		initiateContact(argv[1], argv[2]);

	}


	return 0;
}
