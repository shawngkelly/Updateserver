#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include "FileHelper.h"
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

const char FILENAME[] = "data.bin";
const char IPADDR[] = "127.0.0.1";
const int  PORT = 50000;
const int  QUERY = 1;
const int  UPDATE = 2;
const int  STRLEN = 256;

// Closes the socket and performs the WSACleanup

void cleanup(SOCKET socket);

// Returns the version number from the data file

int getLocalVersion();

//Function to read the 3 ints of the a data file.

void readData(int& num1, int& num2, int& num3);

//Program starts server and listens for incoming connection. When connection is made
//programs waits for one of two requests. If 1, the server will respond with 
//it's version number. If 2, the server will send read file and send the contents. 


int main()
{
	//Update vars for the process of updating the file. 

	int			serverVersion;
	int			num1 = 0;
	int			num2 = 0;
	int			num3 = 0;

	//Connection vars

	WSADATA		wsaData;
	SOCKET		listenSocket;
	SOCKET		acceptSocket;
	SOCKADDR_IN	serverAddr;
	int			recvMessage;
	bool		done = false;
	int         requests =0;

	//Server checks and stores the  local version number

	serverVersion = getLocalVersion();


	// Loads Windows DLL (Winsock version 2.2) used in network programming

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		cerr << "ERROR: Problem with WSAStartup\n";
		return 1;
	}

	// Create a new socket to listen for client connections

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSocket == INVALID_SOCKET)
	{
		cerr << "ERROR: Cannot create socket\n";
		WSACleanup();
		return 1;
	}

	// Setup a SOCKADDR_IN structure which will be used to hold address
	// and port information. Notice that the port must be converted
	// from host byte order to network byte order.

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IPADDR, &serverAddr.sin_addr);

	// Attempt to bind to the port.

	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cerr << "ERROR: Cannot bind to port\n";
		cleanup(listenSocket);
		return 1;
	}

	// Start listening for incoming connections, notifiys of connection or shuts down if an error. 

	if (listen(listenSocket, 1) == SOCKET_ERROR)
	{
		cerr << "ERROR: Problem with listening on socket\n";
		cleanup(listenSocket);
		return 1;
	}

	cout << "Update server is up and running. \n\n";
	cout << "Current version available is " << serverVersion << "\n\n";
	cout << " " << requests << " requests have been made. \n\n";
	
	//Server enters loop to wait for incoming requests. 

	while (!done)
	{

		cout << "Waiting for incoming requests. \n\n";

		// Waiting to accept incoming connection.  Program pauses here until
		// a connection arrives. Progam will accept connection and recieve request.

		acceptSocket = accept(listenSocket, NULL, NULL);
		cout << "Connected to client. \n\n";
		cout << "\n\t--WAIT--\n\n";
		int vRecv = recv(acceptSocket, (char*)&recvMessage, sizeof((char*)&recvMessage), 0);
		
		//Once message has receieved program will compare recvMessage to predefined constants
		//to direct program to next step. 

		if (recvMessage == QUERY)
		{
			//Client has requested server version number. Program will return version number  and close accept socket.
			//Will go back to top of loop and wait for next request.

			cout << "Client is requesting server version.\n\n";
			cout << "Sending version number now\n\n";
			int iSend = send(acceptSocket, (char*)&serverVersion, sizeof((char*)&serverVersion), 0);
			if (iSend == SOCKET_ERROR)
			{
				cerr << "ERROR: FAILED TO SEND UPDATE\n";
				closesocket(acceptSocket);
			}

			cout << "Closing socket--Waiting for new request\n\n";

			//Closes socket, then reopnes socket to listen for new connections

			closesocket(acceptSocket);
		}

		if (recvMessage == 2)
		{	
			//Client has requested updated version from server. Program will open file, store the data and send the data to 
			//client. 

			cout << "Client has outdate version.\n\n";
			cout << "Sending  version " << serverVersion << " to client\n\n";
			ifstream dataFile;
			openInputFile(dataFile, FILENAME);
			readData(num1, num2, num3);
			int sendNum1 = send(acceptSocket, (char*)&num1, sizeof((char*)&num1), 0);
			int sendNum2 = send(acceptSocket, (char*)&num2, sizeof((char*)&num2), 0);
			int sendNum3 = send(acceptSocket, (char*)&num3, sizeof((char*)&num3), 0);
			if (sendNum1 == SOCKET_ERROR || sendNum2 == SOCKET_ERROR || sendNum3 == SOCKET_ERROR)
			{
				cerr << "ERROR: FAILED TO SEND UPDATE\n";
				cleanup(acceptSocket);
			}
			else
			{
				cout << "Data transmitted successfully\n\n";
			}

			cout << "Closing socket--Waiting for new request\n\n";

			//Closes socket, then reopnes socket to listen for new connections

			closesocket(acceptSocket);

		}

		//Counts number of requests made.

		requests++;
		cout << "Number of requests = " << requests << "\n\n";
	}
}

	


int getLocalVersion()
{
	ifstream dataFile;
	openInputFile(dataFile, FILENAME);

	int version = readInt(dataFile);
	dataFile.close();

	return version;
}

void readData(int& num1, int& num2, int& num3)
{
	ifstream dataFile;
	openInputFile(dataFile, FILENAME);

	// Read the version number and discard it
	   num1 = readInt(dataFile);

	// Read the two data values
	num2 = readInt(dataFile);
	num3= readInt(dataFile);

	dataFile.close();
}

void cleanup(SOCKET socket)
{
	closesocket(socket);
	WSACleanup();
}