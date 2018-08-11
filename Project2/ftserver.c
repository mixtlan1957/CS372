/*********************************************************************
** Program Filename: ftserver.c
** Author: Mario Franco-Munoz
** Due Date: 8/12/2018
** Description:CS 372 Project 2: This file implements the "fserver" program
** as requred for project2. ftserserver waits for client to establish connection
** and upon successful connection, ftserver accepts "-l" command to list files in
** the server directory and "-g" to get a file on a second connection.
*********************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <assert.h>
#include <dirent.h>
#include <arpa/inet.h>  //convert IPv4 and IPv6 addresses from binary to text form
#include <errno.h>

#define CONNECTION_LIMIT 1
#define MAX_FILE_NAME_LEN 256



//function prototypes
int startup(char*);
int handleRequest(char *, struct sockaddr_in *, int);
void primaryLoop(char *);
void sendStatusMsg(int, int); 


//startup
int startup(char* port) {

	int portNumber = atoi(port);
	struct sockaddr_in serverAddress;
	int listenSocketFD, errorFlag = 0;

	//Source: The following code is based on CS344 lecture notes and examples
	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) {
		fprintf(stderr, "ERROR opening socket\n");
		printf("Error opening socket\n");
		goto cleanup2;
	}


	//set up for reuse to avoid problems
	//source: GATECH lecture notes	
	int optval = 1;
	setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));


	//Enable the socket to begin listening
	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {// Connect socket to port
		fprintf(stderr, "ERROR on binding\n");
		printf("Error on binding\n");
		errorFlag = 1;
		goto cleanup2;
	}
	listen(listenSocketFD, CONNECTION_LIMIT); // Flip the socket on - it can now receive up to x connections


	printf("Server open on %s\n", port);


	return listenSocketFD;

	cleanup2:

	if (errorFlag == 1) {
		return -1;
	}
	return -1;
}




void primaryLoop(char* port) {

	struct sockaddr_in clientAddress;
	socklen_t sizeOfClientInfo;
	int errorFlag, listenSocketFD, establishedConnectionFD;
	//int establishedConnectionFD_FT;
	int charsRead; //, fileSent;
	//ssize_t byteSent;
	//pid_t spawnPid;
	char readBuffer[1000];
	

	//setup connection/listener
	listenSocketFD = startup(port);
	if (listenSocketFD == -1) {
		errorFlag = 1;
		goto cleanup;
	}


	while(1) {
		errorFlag = 0;

		// 
		// Accept a connection, blocking if one is not availab
		// le until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) {
			fprintf(stderr, "ERROR on accept\n");
			printf("error on accept\n");
			goto cleanup;
		}

		//read the first chunk of data
		charsRead = recv(establishedConnectionFD, readBuffer, 1000, 0);
		if(charsRead < 0) {
			fprintf(stderr, "ERROR reading from socket\n");
			printf("error reading from socket\n");
			errorFlag = 1;
			goto cleanup;
		}

		//	char *incHost = NULL;
		//getnameinfo((struct sockaddr *)&clientAddress, sizeOfClientInfo, incHost, sizeof(incHost), NULL, NULL, 0);  
		//unsigned long ftclient_Addr = clientAddress.sin_addr.s_addr;
		
		//gethostbyaddr(&clientAddress.sin_addr.s_addr, sizeof(clientAddress), AF_INET);

		

		//call handle request
		handleRequest(readBuffer, &clientAddress, establishedConnectionFD);

	}

	cleanup:
	if (errorFlag == 1) {
		exit(1);
	}

}


void sendStatusMsg(int establishedConnectionFD, int fileSent) {
	char *fileNotFound = "FILE NOT FOUND.";
	char *xferComplete = "inbound";
	ssize_t byteSent;	

	//send status messages
	if (fileSent == 0) {
		byteSent = send(establishedConnectionFD, fileNotFound, strlen(fileNotFound) + 1, 0);
		if (byteSent < 0) {
			fprintf(stderr, "ftserver: send ERROR\n");
		}
	}
	else if (fileSent == 1) {
		byteSent = send(establishedConnectionFD, xferComplete, strlen(xferComplete) + 1, 0);
		if (byteSent < 0) {
			fprintf(stderr, "ftserver: send ERROR\n");
		}
	}	
}




//handleRequest
int handleRequest(char *command, struct sockaddr_in *peerAddr, int establishedFD) {

	int internalError = 0;
	DIR *dirLoadFrom;
	int validInput = 0;
	int displayDirectory = 0;
	int sendFile, errorFlag = 0;
	char fileName[MAX_FILE_NAME_LEN];
	char charPort[56];
	char *fileIndex = NULL;
	int fileIdxSize = 56;
	int sendDataPort;
	int charStored = 0;
	char *token;
	struct dirent *fileInDir = NULL;
	char ftclientName[256];
	//socket variables for sending on data port
	int socketFD;
	struct sockaddr_in clientAddress;
	struct hostent* clientHostInfo;
	int currentSend;
	ssize_t byteSent;
	//variables for sending particular file
	int bytesRead, fileFound = 2;
	FILE *fd = NULL;

	//char tempStr[56];
	//memset(tempStr, '\0', sizeof(tempStr));
	//strcpy(tempStr, command);

	//check for the "-l command"
	token = strtok(command, " ");
	if (token != NULL) {
		if (strcmp(token, "-l") == 0) {
			//get the data port number
			token = strtok(NULL, " ");
			memset(charPort, '\0', sizeof(charPort));
			strcpy(charPort, token);
			sendDataPort = atoi(charPort);
			validInput = 1;
			displayDirectory = 1;
		}
		//check for -g and subsequent file name and port number
		else if (strcmp(token, "-g") == 0) {
			token = strtok(NULL, " ");
			if (token != NULL) {
				memset(fileName, '\0', MAX_FILE_NAME_LEN);
				strcpy(fileName, token);

				//get the port number
				token = strtok(NULL, " \n");
				if (token != NULL) {
					memset(charPort, '\0', sizeof(charPort));
					strcpy(charPort, token);
					sendDataPort = atoi(charPort);
					validInput = 1;
					sendFile = 1;
				}
			}
		}
		else {
			errorFlag = 1;
			goto dataConCleanup;
		}
	}

	
	//setup socket for file transfer connection
	if (validInput == 1) {
		//send the directory listing to client
		//socket code based on CS344 lecture notes
		socketFD = 0;

		//send to ftclient
		memset((char*)&clientAddress, '\0', sizeof(clientAddress)); //Clear out the address struct
		clientAddress.sin_family = AF_INET; //create a network-cable socket (IPV4)
		clientAddress.sin_port = htons(sendDataPort); //Store the port number
		//clientHostInfo = gethostbyname(peerName); //Convert the machine name into a special form of address
		clientHostInfo = gethostbyaddr(&(peerAddr->sin_addr), sizeof(peerAddr), AF_INET);
		//validate the host
		if (clientHostInfo == NULL) {
			fprintf(stderr, "FTSERVER: ERROR, no such host\n");
			errorFlag = 1;
			goto dataConCleanup;
		}
	

		//copy in the address
		//memcpy((char*)&clientAddress.sin_addr.s_addr, (char*)clientHostInfo->h_addr, clientHostInfo->h_length);
		memcpy((char*)&clientAddress.sin_addr.s_addr, (char*)clientHostInfo->h_addr, clientHostInfo->h_length); 

		//set up the socket
		socketFD = socket(AF_INET, SOCK_STREAM, 0); //create the socket
		if (socketFD < 0) {
			fprintf(stderr, "ftserver: ERROR opening socket\n");
			errorFlag = 1;
			goto dataConCleanup;
		}

		//connect to ftclient
		if (connect(socketFD, (struct sockaddr*)&clientAddress, sizeof(clientAddress)) < 0 ) { //connect socket to address
			fprintf(stderr, "ftserver: ERROR connecting\n");
			errorFlag = 1;
			goto dataConCleanup;
		}
	}

	//get the client name
	//this was incredibly annoying to find:
	//source: http://www.microhowto.info/howto/convert_an_ip_address_to_the_corresponding_domain_name_in_c.html
	//getpeername() is not what we need here! getnameinfo() is!
	memset(ftclientName, '\0', sizeof(ftclientName));
	getnameinfo((struct sockaddr*)&clientAddress, sizeof(clientAddress), ftclientName, sizeof(ftclientName), 0, 0, 0);


	//send working directory if applicable
	if (validInput == 1 && displayDirectory == 1) {
		printf("List directory requested on port %s.\n", charPort);
		printf("Sending directory contents to %s: %s\n", ftclientName, charPort);

		//allocate string for sending directory contents
		fileIdxSize = 1000;
		charStored = 0;
		fileIndex = malloc(sizeof(char) * fileIdxSize);
		if (fileIndex == NULL) {
			internalError = 1;
			goto dataConCleanup;
		}
		else {
			memset(fileIndex, '\0', fileIdxSize);
		}


		//open base directory
		dirLoadFrom = opendir(".");
		int firstRead = 0;
		//save all the file names
		if (dirLoadFrom > 0) {
			//iterate through files in base directory
			while((fileInDir = readdir(dirLoadFrom)) != 0) {
				//skip over sub directories and parent directory
				if (strcmp(fileInDir->d_name, "..") == 0) {
					continue;
				}
				if (strcmp(fileInDir->d_name, ".") == 0) {
					continue;
				}


				//store file name (file name only, full file path will have to be appended later)
				if (firstRead == 0) {
					strcpy(fileIndex, fileInDir->d_name);
					strcat(fileIndex, "\n");
					firstRead = 1;
				}

				else {
					strcat(fileIndex, fileInDir->d_name);
					strcat(fileIndex, "\n");
				}
				charStored += strlen(fileInDir->d_name);
				charStored++;   //newline character
			
				
				//reallocate if necessary
				if (charStored >= fileIdxSize) {
					char *temp = malloc(sizeof(char) * fileIdxSize *2);
					memset(temp , '\0', fileIdxSize * 2);
					memcpy(temp, fileIndex, fileIdxSize);
					free(fileIndex);
					fileIndex = temp;
					fileIdxSize *= 2;
				}

			}
		}
	 	charStored++; //one last increment for null terminator	
	
		char charMsgSize[256];
		memset(charMsgSize, '\0', 256);
		sprintf(charMsgSize, "%d", charStored); 

	 	//send the size of the message we are going to send
	 
	 	byteSent = send(socketFD, charMsgSize, 256, 0);
	 	if (byteSent < 0) {
			fprintf(stderr, "ftserver: Send ERROR\n");
			errorFlag = 1;
			goto dataConCleanup;
		}


		//send ftclient the list of files in the path directory as one continous string
		byteSent = send(socketFD, fileIndex, 1000, 0);
		if (byteSent < 0) {
			fprintf(stderr, "ftserver: Send ERROR\n");
			errorFlag = 1;
			goto dataConCleanup;
		}
		//send contents of directory in 1000 byte chunks until it is completely delivered
		while(byteSent < charStored) {
			currentSend = send(socketFD, &fileIndex[byteSent], 1000, 0); //send out next chunk of data

			//check for send errors
			if (currentSend < 0) {
				fprintf(stderr, "ftserver: Send ERROR\n");
				errorFlag = 1;
				goto dataConCleanup;
			}
			//if our send was successful, tally up the bytes to move the pointer
			else {
				byteSent += currentSend;
			}
		}
	}


	//if "-g filename portno" was entered:
	else if (validInput == 1 && sendFile == 1) {
		printf("Connection from %s.\n", ftclientName);
		printf("File \"%s\" requested on port %s.\n", fileName, charPort);


		if ((fd = fopen(fileName, "r")) == NULL) {
			printf("File not found. Sending error message to %s: %s\n", ftclientName, charPort);
			fileFound = 0;

			//tell client we did not find the file
			sendStatusMsg(establishedFD, 0);
			
			goto dataConCleanup;
		}
		sendStatusMsg(establishedFD	, 1);

		//sorce: https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
		//get size of file
		fseek(fd, 0L, SEEK_END);
		int sz = ftell(fd);
		char sizeChar[256];
		memset(sizeChar, '\0', 256);
		sprintf(sizeChar, "%d", sz);
		rewind(fd);

		//tell the client how big the file is that we are sending:
	 	byteSent = send(socketFD, sizeChar, 256, 0);
	 	if (byteSent < 0) {
			fprintf(stderr, "ftserver: Send ERROR\n");
			errorFlag = 1;
			goto dataConCleanup;
		}

		//read and send in 1000 byte chunks
		unsigned char fileDataBuffer[1000];
		memset(fileDataBuffer, 0, sizeof(fileDataBuffer));
		bytesRead = fread(fileDataBuffer, 1, 1000, fd);
		
		//keep reading and sending until entire file has been sent
		//my wife helped me with this part
		while(bytesRead != 0) {
			if (bytesRead > 0) {
				currentSend = send(socketFD, fileDataBuffer, 1000, 0);
				if (currentSend < 0) {
					fprintf(stderr, "ftserver: Send ERROR\n");
					errorFlag = 1;
					goto dataConCleanup;
				}
			//	while(currentSend < bytesRead) {
				//	currentSend += send(socketFD, &fileDataBuffer[currentSend], 1000, 0);
			//	}	
			}
			memset(fileDataBuffer, 0, sizeof(fileDataBuffer));
			bytesRead = fread(fileDataBuffer, 1, 1000, fd);
		}
		fileFound = 1;

		fclose(fd);
	}

	else {
		fprintf(stderr, "Bad request from client recieved.\n");
	}

	
	if (internalError == 1) {
		fprintf(stderr, "Internal server error...\n");
	}


	//cleanup goes here *****
	dataConCleanup:
	if (fileIndex != NULL) {
		free(fileIndex);
	}

	if (socketFD != 0) {
		close(socketFD);
	}

	if (errorFlag == 1) {
		exit(1);
	}

	return fileFound;

}





int main(int argc, char *argv[]) {
	//make sure that two arguments are being passed: program name, port number
	if (argc != 2) {
		fprintf(stderr, "Incorrect number of arguments passed to function. "); 
		fprintf(stderr, "Please make sure port number is included.\n");
		exit(1);
	}
	else {
		if (argv[1] == NULL) {
			fprintf(stderr, "input port cannot be NULL.\n");
			exit(1);
		}
		primaryLoop(argv[1]);
	}


	return 0;
}
