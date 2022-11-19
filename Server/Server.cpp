#include "Server.h"
#include <random>
#include <string>
#include <climits>
#include <sstream>





Server::Server()
{


	if (InitialSocketSetup())
	{
		// Calculate maximum amount of clients allowed
		numMaxClients = 10 * std::thread::hardware_concurrency();


		std::cout << "********************** Server *********************" << std::endl;
		std::cout << "************** Listening for connections **********" << std::endl;

		std::cout << std::endl << "------------> Data received:" << std::endl;
		//Main infinite loop
		while (true)
		{

			// Listen for client connection
			SOCKET clientSocket = accept(m_listenSocket, NULL, NULL);



			// Access shared member - m_connectedClients
			{
				// Protect m_connected_clients access
				std::lock_guard<std::mutex> lockGuard(m_mutexConnectedClients);
				// Increment amount of clients
				m_connectedClients++;

				// Increment m_connectedClients and check if surpassed maximum limit
				if (m_connectedClients > numMaxClients)
				{
					CloseSocket(clientSocket);

					{
						// Protect m_connected_clients access
						std::lock_guard<std::mutex> lockGuard(m_mutexUI);
						std::cout << "Server busy! New client rejected." << std::endl;
					}

					// Access shared member - log file
					{
						// Protect log file access
						std::lock_guard<std::mutex> lockGuard(m_mutexLogFile);
						// Write to log file
						m_logFile.WriteString(std::string("Server busy!New client rejected.") + "\0");
					}

				}
			}

			

			//Connection was successful
			if (m_connectedClients <= numMaxClients)
			{
				// Create client thread and client session
				std::thread clientThread;
				struct ClientSessionIDs clientSession;


				// Access shared member - m_clientSession
				{
					// Protect m_clientSession access
					std::lock_guard<std::mutex> lockGuard(m_mutexClientSession);
					// Store both thread ID and client ID
					m_clientSession.push_back(clientSession);
				}

				// Create client thread
				clientThread = std::thread(
					&Server::HandleConnection,
					this,
					clientSocket,			   // Socket to receive data from
					&(m_clientSession.back()), // Pointer to ClientSessionIDs
					&clientThread			   // Pointer to thread
				);
				clientThread.detach();
			}

		}//while (true)
	}//if (InitialSocketSetup())

}




Server::~Server()
{
	CloseSocket(m_listenSocket);
	WSACleanup();
}






bool Server::InitialSocketSetup()
{
	bool returnValue = false;

	// Startup connection
	if (StartupConnection()) 
		returnValue = true;
	else 
		returnValue = false;

	// Open listening socket
	if (OpenListeningSocket() && (true == returnValue))
		returnValue = true;
	else
		returnValue = false;

	// Listen for connections
	if (ListenForConnections() && (true == returnValue))
		returnValue = true;
	else
		returnValue = false;


	return returnValue;
}







bool Server::StartupConnection()
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
		returnValue =  true;
	}

	return returnValue;
}







bool Server::OpenListeningSocket()
{
	bool returnValue = false;

	PCSTR nodeName = "Server";

	struct addrinfo* serverInfo = NULL;
	struct addrinfo socketHints;

	// Zero socketHints
	memset(&socketHints, 0, sizeof(socketHints));

	// AF_INET     -> IPv4
	// SOCK_STREAM -> sequenced, reliable, two-way, connection-based byte stream
	// IPPROTO_TCP -> TCP protocol
	// AI_PASSIVE  -> Socket address structure will be used in a call to the bind function
	socketHints.ai_family = AF_INET;
	socketHints.ai_socktype = SOCK_STREAM;
	socketHints.ai_protocol = IPPROTO_TCP;
	socketHints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	if (NO_ERROR != getaddrinfo(NULL, SERVER_PORT, &socketHints, &serverInfo))
	{
		// Error handling
		std::cout << "Error at getaddrinfo(). Error code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		returnValue = false;
	}
	else //getaddrinfo was successful
	{
		// Create listening socket
		m_listenSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

		// Error handling
		if (INVALID_SOCKET == m_listenSocket)
		{
			std::cout << "Error at socket creation. Error code: " << WSAGetLastError() << std::endl;
			WSACleanup();
			returnValue = false;
		}
		else // Socket creation was successful
		{
			// Bind socket to local address
			if (NO_ERROR != bind(m_listenSocket, serverInfo->ai_addr, (int)serverInfo->ai_addrlen))
			{
				// Error handling
				std::cout << "Error at socket binding. Error code: " << WSAGetLastError() << std::endl;
				freeaddrinfo(serverInfo);
				WSACleanup();
				return false;
			}
			else // Socket binding was successful
			{
				freeaddrinfo(serverInfo);
				returnValue = true;
			}
		}
	}

	return returnValue;
}








bool Server::ListenForConnections()
{
	bool returnValue = false;

	// Listen for connections
	if (SOCKET_ERROR == listen(m_listenSocket, 1))
	{
		// Error handling
		std::cout << "Error at listening for connections. Error code: " << WSAGetLastError() << std::endl;
		closesocket(m_listenSocket);
		WSACleanup();
		returnValue = false;
	}
	else // Listen() was successful
	{
		returnValue = true;
	}

	return returnValue;
}












void Server::ProcessData(SOCKET clientSocket, ClientSessionIDs clientSession)
{
	uint32_t bytesReceived;
	struct Command commandReceived;
	int operationResult;

	// Buffer to store received data.
	char m_receiveBuffer[RECEIVE_BUFFER_LENGTH];


	// Do this while data keeps comming
	do
	{
		// Receive data
		bytesReceived = recv(clientSocket, m_receiveBuffer, RECEIVE_BUFFER_LENGTH, 0);
		if (bytesReceived > 0) 
		{

			std::string receivedMessage = m_receiveBuffer;

			// Parse the message
			commandReceived = ParseMessage(receivedMessage);
			// Calculate command received
			operationResult = CalculateCommand(commandReceived);


			// Access shared member - UI
			{
				// Protect UI access
				std::lock_guard<std::mutex> lockGuard(m_mutexUI);
				// Write to UI
				std::cout <<
					std::to_string(clientSession.ClientSession_threadID) + ":" +
					std::to_string(clientSession.ClientSession_ID) + ":" +
					std::string(receivedMessage.substr(0, receivedMessage.find(")") + 1)) + ":" +
					std::to_string(operationResult)
				<< std::endl;
			}


			// Access shared member - log file
			{
				// Protect log file access
				std::lock_guard<std::mutex> lockGuard(m_mutexLogFile);
				// Write to log file
				m_logFile.WriteString(
					std::to_string(clientSession.ClientSession_threadID) + ":" +
					std::to_string(clientSession.ClientSession_ID) + ":" +
					std::string(receivedMessage.substr(0, receivedMessage.find(")") + 1)) + ":" +
					std::to_string(operationResult) + "\0"
				);
			}
		}
		// Client closed connection
		else if (bytesReceived == 0)
		{
			// Close client Socket
			CloseSocket(clientSocket);


			// Access shared member - log file
			{
				// Protect log file access
				std::lock_guard<std::mutex> lockGuard(m_mutexLogFile);
				// Write to log file
				m_logFile.WriteString(
					std::to_string(clientSession.ClientSession_threadID) + ":" +
					std::to_string(clientSession.ClientSession_ID) + ":" +
					"Client has disconnected"
				);

			}

			//For debug purposes only! Otherwise rate of creation/destruction is bigger than time it takes to reach 
			// maximum clients value
			Sleep(1000);
			// Access shared member - m_connectedClients
			{
				// Protect m_connectedClients access
				std::lock_guard<std::mutex> lockGuard(m_mutexConnectedClients);
				// Decrement amount of connected clients
				m_connectedClients--;
			}
			
		}
		// Error handling
		else 
		{
			std::cout << "Error: Reception failed. Error code: " << WSAGetLastError() << std::endl;
			CloseSocket(clientSocket);
			return;
		}
	} while (bytesReceived > 0);
}






void Server::ProcessIDs(ClientSessionIDs* pClientSession, std::thread::id threadID)
{
	pClientSession->ClientSession_ID = GenerateRandomAllowedID();
	pClientSession->ClientSession_threadID = *reinterpret_cast<unsigned int*>(&threadID);
}







void Server::HandleConnection(SOCKET clientSocket, ClientSessionIDs* pClientSession, std::thread* pCurrentThread)
{
	ProcessIDs(pClientSession, std::this_thread::get_id());
	ProcessData(clientSocket, *pClientSession);
}







void Server::CloseSocket(SOCKET socketToClose)
{

	if(SOCKET_ERROR == shutdown(socketToClose, SD_SEND))
	{
		// Protect UI access
		std::lock_guard<std::mutex> lockGuard(m_mutexUI);
		// Write to UI
		std::cout << "Error: Shutdown failed. Error code: " << WSAGetLastError() << std::endl;
	}

	closesocket(socketToClose);

	return;
}









uint8_t Server::GenerateRandomAllowedID()
{
	uint8_t generatedID;
	bool continueLoop = true;

	// Random device that will seed the generator
	std::random_device seeder;

	// Make a mersenne twister engine
	std::mt19937 engine(seeder());

	// Distribution
	std::uniform_int_distribution<uint16_t> dist(0, UINT8_MAX);




	// Access shared member - m_clientSession
	{
		// Protect m_clientSession access
		std::lock_guard<std::mutex> lockGuard(m_mutexClientSession);
		// If there are already clients
		if (!m_clientSession.empty())
		{
			while (continueLoop)
			{
				// Generate ID
				generatedID = dist(engine);

				continueLoop = false;

				// Check if there is any collision
				for (const auto& iterator : m_clientSession)
				{
					if (iterator.ClientSession_ID == generatedID)
						continueLoop = true;
				}
			}
		}
		else // If there are no clients yet
		{
			// Generate ID
			generatedID = dist(engine);
		}
	}


	return generatedID;
}










struct Command Server::ParseMessage(std::string message)
{
	struct Command messageParsed;
	std::stringstream stringStream;

	// Operation
	messageParsed.Operation = message.substr(0, 3);

	// Operand 1
	stringStream << message.substr((message.find("(") + 1), (message.find(",") - message.find("(") - 1) );
	stringStream >> messageParsed.Operand1;

	stringStream.clear();

	// Operand 2
	stringStream << message.substr((message.find(",") + 1), message.find(")"));
	stringStream >> messageParsed.Operand2;

	return messageParsed;
}








int Server::CalculateCommand(struct Command commandReceived)
{
	int commandCalculated = -1;

	// Add
	if (commandReceived.Operation == "Add")
		commandCalculated = commandReceived.Operand1 + commandReceived.Operand2;
	// Sub
	else if (commandReceived.Operation == "Sub")
		commandCalculated = commandReceived.Operand1 - commandReceived.Operand2;

	// Mul
	else if (commandReceived.Operation == "Mul")
		commandCalculated = commandReceived.Operand1 * commandReceived.Operand2;

	// Div
	else if (commandReceived.Operation == "Div")
		commandCalculated = commandReceived.Operand1 / commandReceived.Operand2;

	return commandCalculated;
}








