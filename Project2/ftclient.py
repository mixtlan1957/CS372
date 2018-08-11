#!/usr/bin/python

#*********************************************************************
#** Program Filename: ftclient.py
#** Author: Mario Franco-Munoz
#** Due Date: 8/12/18
#** Description: Program 2 CS 372: ftclient File. This file implements the
#** ftclient portion of assignment.
#*********************************************************************/

import socket
import string
import os
import sys
from os import path
from time import sleep



#initiateContact
#this function establishes the first communication connection with ftserver for communication
#source: https://docs.python.org/2/library/socket.html#socket.socket.listen
def initiateContact(serverName, port):
	socketFD = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	socketFD.connect((serverName, port))
	return socketFD



#makeRequest
def makeRequest(socketFD, req):
	socketFD.sendall(req)



#recieveFile
def recieveFile(rec_socketFD, fileName, portNo, comSocket, hostName):
	connectionSocket, addr = rec_socketFD.accept()

	#check if file already exists in current directory
	#source: https://stackoverflow.com/questions/82831/how-do-i-check-whether-a-file-exists
	if path.isfile(fileName):
		fileName = fileName.split(".")[0] + "_copy1.txt"
		fcount = 1
		while path.isfile(fileName) == True:
			fcount = fcount + 1
			fileName =  fileName.split("_")[0] + "_copy" + str(fcount) + ".txt" 

	print("Receiving \"" + fileName + "\" from " + hostName + ":" + str(portNo))


	#file length will be in "header" aka first four bytes of data		
	fileLen_CString = connectionSocket.recv(4)
	fileLen = int(fileLen_CString.strip('\0'))


	fileBuffer = ""

	while len(fileBuffer) < fileLen:
		packet = connectionSocket.recv(fileLen - len(fileBuffer))
		if not packet:
			return None
		fileBuffer += packet

	with open(fileName, 'w') as f:
		f.write(fileBuffer)


	#small sleep interval before getting status msg
	sleep(1)


	#recieve status message
	status_C = comSocket.recv(4)
	statusMsgLen = int(status_C.strip('\0'))
	

	statusMsg = ""

	while len(statusMsg) < statusMsgLen:
		packet = comSocket.recv(statusMsgLen - len(statusMsg))
		if not packet:
			return None
		statusMsg += packet

	print(statusMsg)

	connectionSocket.close()
	#rec_socketFD.close()
	


	
#recieveFileList
#source: https://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data
def recieveFileList(rec_socketFD, portNo, hostName):
	connectionSocket, addr = rec_socketFD.accept()

	#https://stackoverflow.com/questions/22747152/converting-string-to-int-in-serial-connection
	#file length will be in "header" aka first four bytes of data
	msgLen_CString = connectionSocket.recv(4)
	msgLen = int(msgLen_CString.strip('\0'))
	#print("Message length: " + str(msgLen))

	recMsg = ""

	while len(recMsg) < msgLen:
		packet = connectionSocket.recv(msgLen - len(recMsg))
		if not packet:
			return None
		recMsg += packet
	

	#display file listing (should already have new line characters)
	print("Recieving directory structure from " +  hostName + ": " + str(portNo))

	#display file structured
	print(recMsg)

	connectionSocket.close()
	#rec_socketFD.close()




#open a new port listener for server response
def ftConnectionSetup(portNo):
	#ftclient server socket
	serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	#socket reuse:
	#source:  https://stackoverflow.com/questions/6380057/python-binding-socket-address-already-in-use
	serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

	serverSocket.bind(('', portNo))
	serverSocket.listen(1)

	return serverSocket






def main():
	request = ""
	if len(sys.argv) < 5 or len(sys.argv) > 6:
		print("Incorrect or missing arguments. Usage: ./ftclient <Server Host Name> <communication port number>")
		print(" <command> <data transfer port number> or <command> followed by <file name> and <data trasnfer port number>")
	else:
		portnumCom = int(sys.argv[2])
		server = sys.argv[1]
		command = sys.argv[3]
		if len(sys.argv) == 5 and command == "-l":
			xferPort = int(sys.argv[4])
			request = "listFiles"
			
		elif len(sys.argv) == 6 and command == "-g":
			fileName = sys.argv[4]
			xferPort = int(sys.argv[5])
			request = "requestFile"

		else:
			print("Bad input. Please try again")
			print("Usage: ./ftclient <Server Host Name> <communication port number>")
			print(" <command> <data transfer port number> or <command> followed by <file name> & <data trasnfer port number>")
	if request == "listFiles" or request == "requestFile":
		#handle the engr.oregonstate.edu extension if we are working with flip or os servers....
		if server == "os1" or server == "os2" or server == "flip1" or server == "flip2" or server == "flip3":
			ftserverName = server + ".engr.oregonstate.edu"
		else:
			ftserverName = server 



		#important distinction here, socketFD is the socket used for sending status messages
		#ft_socketFD is "connection Q" and used for sending directory and the files
		#build request string
		errorFlag = 0;
		if request == "listFiles":
			req = command + " " + str(xferPort) + " " + "\0"
		else:
			req = command + " " + fileName + " " + str(xferPort) + "\0"	
	
		print(ftserverName)
		socketFD = initiateContact(ftserverName, portnumCom)
		makeRequest(socketFD, req)
		ft_socketFD = ftConnectionSetup(xferPort)

		if request == "listFiles":
			recieveFileList(ft_socketFD, xferPort, ftserverName)
		else:
			recieveFile(ft_socketFD, fileName, xferPort, socketFD, ftserverName)

		#close the FIRST (communication) socket
		socketFD.close()
		#close the SECOND (FT) socket	
		ft_socketFD.close()



if __name__ == '__main__':
	main()
