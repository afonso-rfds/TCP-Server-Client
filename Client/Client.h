#pragma once

#ifndef CLIENT_HPP
#define CLIENT_HPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_PORT "49152"
#define NUMBER_OF_COMMANDS 4

class Client
{
public:
	Client();
	~Client();

	/// <summary> Creates numClients amount of clients
	/// <returns> ID number
	void CreateClients(int numClients);


private:
	/// <summary> Initiates use of Winsock
	/// <returns> True if success. False otherwise
	bool StartupConnection();

	/// <summary> Creates a listening socket and binds it with a local address
	/// <returns> True if success. False otherwise
	SOCKET CreateTransmittingSocket();

	/// <summary> Generates random ID between 0 and maxSize
	/// <returns> ID number
	int GenerateRandomValue(int maxSize);

	/// <summary> Transmits data to m_transmitSocket
	/// <returns> ID number
	void TransmitData(SOCKET m_transmitSocket);

	/// <summary> Disconnects client m_transmitSocket
	/// <returns> ID number
	void DisconnectClient(SOCKET m_transmitSocket);

	WSADATA m_wsaData;
	SOCKET m_transmitSocket;
	std::string m_CommandsList[NUMBER_OF_COMMANDS] = { "Add", "Sub", "Mul", "Div"};
};
#endif // CLIENT_HPP

