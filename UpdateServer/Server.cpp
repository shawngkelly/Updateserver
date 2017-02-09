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

int main()
{	
	//Update vars
	int			serverVersion;

	//Connection vars
	WSADATA		wsaData;
	SOCKET		listenSocket;
	SOCKET		acceptSocket;
	SOCKADDR_IN	serverAddr;
	int			port;
	char		sendMessage[STRLEN];
	char		recvMessage[STRLEN];
	bool		done = false;

	//Server checks local version number
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

	// Start listening for incoming connections
	if (listen(listenSocket, 1) == SOCKET_ERROR)
	{
		cerr << "ERROR: Problem with listening on socket\n";
		cleanup(listenSocket);
		return 1;
	}

	cout << "\nWaiting for connections...\n";

	// Accept incoming connection.  Program pauses here until
	// a connection arrives.
	acceptSocket = accept(listenSocket, NULL, NULL);

	// For this program, the listen socket is no longer needed so it will be closed
	closesocket(listenSocket);

	cout << "Connected...\n\n";

	
	while (!done)
	{
		// Wait to receive a message from the remote computer
		cout << "\n\t--WAIT--\n\n";
		int vRecv = recv(acceptSocket, recvMessage, STRLEN, 0);
		if (vRecv > 0)
		{
			recvMessage[vRecv] = '\0';
			if ((int)recvMessage[0] == 1)
			{
				cout << "Client is requesting server version.\n\n";
				cout << "Latest Version is " << serverVersion << "\n\n";
				cout << "Sending version number now\n";
				int iSend = send(acceptSocket, (char*)&serverVersion, strlen((char*)&serverVersion), 0);

				cout << "Closing socket--Waiting for new request\n\n";

				//Closes socket, then reopnes socket to listen for new connections

				closesocket(acceptSocket);

				// Create a new socket to listen for client connections
				listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

				if (listenSocket == INVALID_SOCKET)
				{
					cerr << "ERROR: Cannot create socket\n";
					WSACleanup();
					return 1;
				}

				// Attempt to bind to the port.
				if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
				{
					cerr << "ERROR: Cannot bind to port\n";
					cleanup(listenSocket);
					return 1;
				}

				// Start listening for incoming connections
				if (listen(listenSocket, 1) == SOCKET_ERROR)
				{
					cerr << "ERROR: Problem with listening on socket\n";
					cleanup(listenSocket);
					return 1;
				}

				cout << "\nWaiting for connections...\n";

				// Accept incoming connection.  Program pauses here until
				// a connection arrives.
				acceptSocket = accept(listenSocket, NULL, NULL);

				// For this program, the listen socket is no longer needed so it will be closed
				closesocket(listenSocket);

				cout << "Connected...\n\n";

				if (iSend == SOCKET_ERROR)
				{
					cerr << "ERROR: Failed to send message\n";
					cleanup(acceptSocket);
					return 1;
				}
			}
			recv(acceptSocket, recvMessage, strlen(recvMessage), 0);
			if ((int)recvMessage[0] == 2)
			{
				cout << "Sending updated version " << serverVersion << " to client\n";
				ifstream dataFile;
				openInputFile(dataFile, FILENAME);

				 
			}
		}
		else if (vRecv == 0)
		{
			cout << "Closing connection\n";
			cleanup(acceptSocket);
			return 0;
		}
		else
		{
			cerr << "ERROR: Failed to receive message\n";
			cleanup(acceptSocket);
			return 1;
		}
	}

	return 0;
}

int getLocalVersion()
{
	ifstream dataFile;
	openInputFile(dataFile, FILENAME);

	int version = readInt(dataFile);
	dataFile.close();

	return version;
}

void readData(int& num1, int& num2)
{
	ifstream dataFile;
	openInputFile(dataFile, FILENAME);

	// Read the version number and discard it
	int tmp = num1 = readInt(dataFile);

	// Read the two data values
	num1 = readInt(dataFile);
	num2 = readInt(dataFile);

	dataFile.close();
}

void cleanup(SOCKET socket)
{
	closesocket(socket);
	WSACleanup();
}