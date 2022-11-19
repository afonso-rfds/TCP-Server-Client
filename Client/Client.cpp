#include "Client.h"
#include <random>
#include <string>


Client::Client()
{
	if (StartupConnection())
	{
		std::cout << "********************** Client *********************" << std::endl;
		std::cout << "****************** Data to transmit ***************" << std::endl;
	}
	else
		std::cout << "Error: Client startup  unsuccessful!" << std::endl;
	
}




Client::~Client()
{
	WSACleanup();
}





void Client::CreateClients(int numClients)
{
	int loopIterator = 0;

	for (; loopIterator < numClients; loopIterator++)
	{
		TransmitData(CreateTransmittingSocket());
	}

	std::cout << std::endl << " --- Created: " << loopIterator << " clients ---" << std::endl;
}






bool Client::StartupConnection()
{
	bool returnValue = false;

	int startupResult = WSAStartup(MAKEWORD(2, 2), &m_wsaData);

	if (NO_ERROR != startupResult)
	{
		std::cout << "Error at getaddrinfo. Error code: " << WSAGetLastError() << std::endl;
		returnValue = false;
	}
	else
	{
		returnValue = true;
	}

	return returnValue;
}






SOCKET Client::CreateTransmittingSocket()
{

	SOCKET m_transmitSocket = INVALID_SOCKET;

	PCSTR nodeName = "Server";

	struct addrinfo* clientInfo = NULL, * serverInfo = NULL;
	struct addrinfo socketHints;



	// Zero socketHints
	memset(&socketHints, 0, sizeof(socketHints));

	// AF_INET     -> IPv4
	// SOCK_STREAM -> sequenced, reliable, two-way, connection-based byte stream
	// IPPROTO_TCP -> TCP protocol
	socketHints.ai_family = AF_INET;
	socketHints.ai_socktype = SOCK_STREAM;
	socketHints.ai_protocol = IPPROTO_TCP;

	// Resolve the local address and port to be used by the server
	if (NO_ERROR != getaddrinfo(NULL, SERVER_PORT, &socketHints, &clientInfo))
	{
		std::cout << "Error at getaddrinfo(). Error code: " << WSAGetLastError() << std::endl;
		WSACleanup();
	}
	else // Socket creation was successful
	{
		// Create socket to connect to server
		m_transmitSocket = socket(clientInfo->ai_family, clientInfo->ai_socktype, clientInfo->ai_protocol);

		if (INVALID_SOCKET == m_transmitSocket)
		{
			std::cout << "Error at socket creation. Error code: " << WSAGetLastError() << std::endl;
			WSACleanup();
		}
		else // Socket creation was successful
		{
			// Connect socket to server
			if (NO_ERROR != connect(m_transmitSocket, clientInfo->ai_addr, (int)clientInfo->ai_addrlen))
			{
				std::cout << "Error at socket connection. Error code: " << WSAGetLastError() << std::endl;
				closesocket(m_transmitSocket);
				m_transmitSocket = INVALID_SOCKET;
				freeaddrinfo(clientInfo);
				WSACleanup();
			}
			else // Socket connect was successful
			{
				freeaddrinfo(clientInfo);
			}
		}
	}

	return m_transmitSocket;
}






int Client::GenerateRandomValue(int maxSize)
{
	uint8_t generatedValue;
	bool continueLoop = true;

	// Random device that will seed the generator
	std::random_device seeder;

	// Make a mersenne twister engine
	std::mt19937 engine(seeder());

	// Distribution
	std::uniform_int_distribution<uint16_t> dist(0, maxSize);

	generatedValue = dist(engine);
	

	return generatedValue;

}





void Client::TransmitData(SOCKET m_transmitSocket)
{
	uint32_t bytesTransmitted;

	int numCommandsToSend = GenerateRandomValue(10);

	// Generate random commands and operands numCommandsToSend amounts of time
	for (int loopCounter = 0; loopCounter < numCommandsToSend; loopCounter++)
	{
		// Generate random command || NUMBER_OF_COMMANDS - 1 due to array[0] being is the first)
		std::string messageToSend = m_CommandsList[GenerateRandomValue(NUMBER_OF_COMMANDS - 1)].c_str();
		// Generate random values and assign them to the command
		messageToSend =
			messageToSend + "(" +
			std::to_string(GenerateRandomValue(1000)) + "," + // Operand 1
			std::to_string(GenerateRandomValue(1000)) + ")";  // Operand 2

		// Send command
		bytesTransmitted = send(m_transmitSocket, messageToSend.c_str(), 14, 0);

		// Error handling
		if (bytesTransmitted == SOCKET_ERROR)
		{
			std::cout << "Error at transmitting data. Error code: " << WSAGetLastError() << std::endl;
			closesocket(m_transmitSocket);
			return;
		}

		std::cout << messageToSend << std::endl;
	}

	DisconnectClient(m_transmitSocket);
}






void Client::DisconnectClient(SOCKET m_transmitSocket)
{
	// Shutdown the connection since no more data will be sent
	if (SOCKET_ERROR == shutdown(m_transmitSocket, SD_BOTH))
	{
		std::cout << "Error at shutting down. Error code: " << WSAGetLastError() << std::endl;
	}

	std::cout << "Connection closed!" << std::endl;

	closesocket(m_transmitSocket);
}
