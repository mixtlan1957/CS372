#!/usr/bin/python

#*********************************************************************
#** Program Filename: chatserve.py
#** Author: Mario Franco-Munoz
#** Due Date: 7/29/18
#** Description: Program 1 CS 372: Server File. This file implements the server
#** portion for the Project 1 assignment. Accepts a connection from the client
#** (written in c) after connection is established messages up to 500 characters
#** in lengh can be exchanged between the client and the server.
#*********************************************************************/

import socket
import string
import os
import sys


#globals
BUFFER_SIZE = 500
HANDLE = "Server"


#this function recieves the message
#input: initialized server socket
def recieveMessage(con_socket):
	global BUFFER_SIZE
	return con_socket.recv(BUFFER_SIZE)



#this function sends message/reply to client
#input: is user provided keyboard input, and connection socket where communication has been established
def sendMessage(msg, con_socket):
	con_socket.send(msg)


#sets up the server's listening port and returns the intialized socket
#resources: https://docs.python.org/2/library/socket.html#socket.socket.listen
#input: is portnumber provided in comand line argument when starting program.
def startUp(portNo):
	#setup server socket
	serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	serverSocket.bind(('', portNo))
	serverSocket.listen(1) 	#accept only 1 connection

	return serverSocket



#this function handles the connection for the server side. This function also serves as the
#primary connection loop. Server will continue to listen until user enters "\quit"
def connectionLoop(portNo):
	global HANDLE

	#call setup server socket
	serverSocket = startUp(portNo)

	connctionSocket, addr = serverSocket.accept()
	while 1:
		rec_msg = recieveMessage(connectionSocket)

		#get reply from user and send to user
		reply = raw_input(HANDLE +"> ")
		sendMessage(reply)

		#if user entered "\quit" exit loop
		if reply == "\quit":
			break

	connectionSocket.close()


def main():
	if len(sys.argv) != 2:
		print("Usage: ./chatserve <port number>")
	else:
		portnum = sys.argv[1]
		connectionLoop(portnum)



if __name__ == '__main__':
	main()