#*********************************************************************
#** Program Filename: 
#** Author: Mario Franco-Munoz
#** Due Date: 7/29/18
#** Description: Program 1 CS 372: Server File
#*********************************************************************/

from socket import *
import string










serverPOrt = 30056
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(('',serverPort))
serverSocket.listen(1)
print("The Server is ready to recieve")
while 1:
	connectionSocket, addr = serverSocket.accept()

	sentence = connectionSocket.recv(1024)
	capitalizedSentence = sentence.upper()
	connectionSocket.send(capitalizedSentence)
	connectionSocket.close()

