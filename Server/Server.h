#pragma once


#ifndef SERVER_HPP
#define SERVER_HPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

#include "LogFile.h"
#include "ClientSessionIDs.h"

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_PORT "49152"
#define RECEIVE_BUFFER_LENGTH 14




// Operation to perfomand operands
struct Command
{
	std::string Operation;
	int Operand1;
	int Operand2;
};



class Server
{
public:
	Server();
	~Server();
private:
	/// <summary> Initial socket setup 
	/// <returns> True if success. False otherwise
	bool InitialSocketSetup();

	/// <summary> Initiates use of Winsock
	/// <returns> True if success. False otherwise
	bool StartupConnection();

	/// <summary> Creates a listening socket and binds it with a local address
	/// <returns> True if success. False otherwise
	bool OpenListeningSocket();

	/// <summary> Listens for connections
	/// <returns> True if success. False otherwise
	bool ListenForConnections();

	/// <summary> Receives and processes data
	/// <returns> non
	void ProcessData(SOCKET clientSocket, ClientSessionIDs clientSession);

	/// <summary> Process the ClientSession IDs
	/// <returns> non
	void ProcessIDs(ClientSessionIDs* pClientSession, std::thread::id currentThread);

	/// <summary> Handles the client connection
	/// <returns> non
	void HandleConnection(SOCKET clientSocket, ClientSessionIDs* pClientSession, std::thread* pCurrentThread);

	/// <summary> Disconnects server
	/// <returns> Non
	void CloseSocket(SOCKET clientSocket);

	/// <summary> Generates random ID between 0 and MAX_UINT8 that does not collide with existing IDs
	/// <returns> ID number
	uint8_t GenerateRandomAllowedID();

	/// <summary> Parses the message
	/// <returns> Struct containg the operation to perfom and operands
	struct Command ParseMessage(std::string message);

	/// <summary> Calculates the command received
	/// <returns> Operation result
	int CalculateCommand(struct Command commandReceived);





	// ---------- Non-shared resources ----------
	// Structure with information about the Windows Sockets implementation.
	WSADATA m_wsaData;

	// Server listening socket.
	SOCKET m_listenSocket;

	// Maximum amount of clients allowed;
	int numMaxClients;
	// --------------------------------------






	// ---------- Shared resources ----------
	// Object that controls the log file.
	LogFile m_logFile;

	// Amount of clients (also threads) connected.
	int m_connectedClients = 0;

	// Array of client sessions
	std::vector<ClientSessionIDs> m_clientSession;
	// --------------------------------------




	// ---------- Mutexes -------------------
	// Mutex to log file access;
	std::mutex m_mutexLogFile;

	// Mutex to m_connectedClients access;
	std::mutex m_mutexConnectedClients;

	// Mutex to m_clientSession access;
	std::mutex m_mutexClientSession;

	// Mutex to UI access;
	std::mutex m_mutexUI;
	// --------------------------------------
};

#endif // SERVER_HPP