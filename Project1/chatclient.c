/*********************************************************************
** Program Filename: chatclient.c
** Author: Mario Franco-Munoz
** Due Date: 7/29/2018
** Description:CS 372 Project 1: chatclient file.
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
void sendMessage(char*, char*);
char* getHandle();
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




/* The following is the main driver function for the client. It prompts the user to first
enter a handle name and then prompts the user to enter chat messages until the "\quit" command is entered
*/
void sendMessage(char *hostName, char *portNo) {
	//socket specific variables
	int socketFD, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	//other variables
	int inputCheck = 0;
	int errorFlag = 0;
	char sendMsgBuffer[500];
	char readBuffer[500];
		//int currentRead = 0;
		//int currentSend = 0;
	//convert port number to int
	int portNumber = atoi(portNo);
	char *instr = "Please enter your handle name. (Maximum 10 characters): ";
	char handle[11];
	size_t tmpSz, byteSent;



	//propt user to enter handle
	memset(handle, '\0', sizeof(handle));
	inputCheck = getUserInput(instr, handle, sizeof(handle));
	while(inputCheck != 0) {
		printf("I'm sorry that handle input was invalid please try again.\n");
		inputCheck = getUserInput(instr, handle, sizeof(handle));
	}



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



	//loop indefinetly until client enters "\quit"
	while(1) {
		tmpSz = sizeof(sendMsgBuffer);
		memset(readBuffer, '\0', tmpSz);
		inputCheck = getUserInput(handle, sendMsgBuffer, tmpSz);

		//recieve and validate user input.
		if (inputCheck != OK) {
			fprintf(stderr, "CLIENT: Bad string input.\n");
			continue;
		}


		//check if quit command was entered
		if (strcmp(readBuffer, "\\quit") == 0) {
			break;
		}
		//send message
		byteSent = 0;
		byteSent = send(socketFD, sendMsgBuffer, tmpSz, 0);
		if (byteSent < 0) {
			fprintf(stderr, "CLIENT: Send ERROR\n");
			errorFlag = 1;
			goto cleanup;
		}

		//recieve message
		charsRead = recv(socketFD, readBuffer, 500, 0);

		
		//error handling
		if (charsRead == -1) {
			fprintf(stderr, "Error recieving message from server\n");
			errorFlag = 1;
			goto cleanup;
		}

		//display message
		printf("%s\n", readBuffer);
		fflush(stdout);
	}

	cleanup:
	//free any malloc'd strings here
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
		sendMessage(argv[1], argv[2]);

	}


	return 0;
}
