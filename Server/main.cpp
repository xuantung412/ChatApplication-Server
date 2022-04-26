#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include "UserAccount.cpp"
#include <vector>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

int main()
{
	std::cout << "Sever is running..." << '\n';

	//Initialize array of UserAccount
	vector<UserAccount> listAccount;

	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		return 99;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Library error!. Quitting...." << endl;
		return 99;
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	std::cout << "Wating for client connection..." << '\n';
	// Tell Winsock the socket is for listening 
	listen(listening, SOMAXCONN);

	// Create the master file descriptor set and zero it
	//The file will have all current connection to this sever. 
	fd_set master;
	FD_ZERO(&master);
	FD_SET(listening, &master);
	bool running = true;

	while (running)
	{
		//This block of code is running the sever. The sever will wait for connection/ message( from connected clients). 
		//Then It proceed and send command back to client.


		fd_set copy = master;

		// See who's talking to us
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Loop through all the current connections / potential connect
		for (int i = 0; i < socketCount; i++)
		{
			// Makes things easy for us doing this assignment
			SOCKET sock = copy.fd_array[i];

			// Is it an inbound communication?
			if (sock == listening)
			{
				// Accept a new connection. Always accept.
				SOCKET client = accept(listening, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);
				std::cout  << "Receive Connection"<< '\n';

			}
			else // It's an inbound message
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);
				string completeMessage = "";
				for (int i = 0; i < bytesIn; i++) {
					completeMessage += buf[i];		
				}

				//Check whether It is command
				if (completeMessage.find("/CheckUser") != std::string::npos){
					//Command check user
					//The command to check user will have format: /CheckUser-(Username)-(Password)
					
					//Split string to get data from formatted message.
					istringstream stringstream(completeMessage);
					string splittedString;
					std::getline(stringstream, splittedString, '-');
					//Get UserName
						std::getline(stringstream, splittedString, '-');
						string userName = splittedString;

					//Get Password
						std::getline(stringstream, splittedString, '-');
						string password = splittedString;

					//Create clone UserAccount to call static method to check account.
						UserAccount newAccount(userName, password);
						string loginResult = "";

						//send to client to accept
						if (UserAccount::accountLoggin(newAccount)) {
							//Succesfull register account
							loginResult = "/AcceptLoggin";
						}
						else {
							loginResult = "/RejectLoggin";
						}

						//Loop through all connections in master to find the sent message client to send message back to it.
						SOCKET clientSock;

						for (u_int i = 0; i < master.fd_count; i++)
						{
							if (master.fd_array[i] == listening || master.fd_array[i] == sock)
							{
								clientSock = master.fd_array[i];
							}
						}
						send(clientSock, loginResult.c_str(), loginResult.size() + 1, 0);
				}
				else if(completeMessage.find("/RegisterAccount") != std::string::npos) {
					string newAccountUserName;
					string newAccountPassword;
					istringstream stringstream(completeMessage);
					string splittedString;	// Split string to get data from formatted message.

					std::getline(stringstream, splittedString, '-');

					//Get UserName
					std::getline(stringstream, splittedString, '-');
					newAccountUserName= splittedString;

					//Get Password
					std::getline(stringstream, splittedString, '-');
					newAccountPassword= splittedString;

					//Try to create clone User to call static method for registration.
					UserAccount newAccount(newAccountUserName, newAccountPassword);
					string registrationResult = "";
					if (UserAccount::registerNewAccount(newAccount)) {
						//Succesfull register account
						registrationResult = "/AcceptNewRegistration";
					}
					else {
						registrationResult = "/RejectRegistration";
					}

					//Loop through all connections to select correct clients to send message.
					SOCKET clientSock;	
					for (u_int i = 0; i < master.fd_count; i++)
					{
						if (master.fd_array[i] == listening || master.fd_array[i] == sock)
						{
							clientSock = master.fd_array[i];
						}
					}
					send(clientSock, registrationResult.c_str(), registrationResult.size() + 1, 0);
				}

				if (bytesIn <= 0)
				{
					// Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				//From there, Message is not a command. It is a normal message from client to send to other clients.

				else
				{
					// Check to see if it's a command. \quit kills the server
					if (buf[0] == '\\')
					{
						// Is the command quit? 
						string cmd = string(buf, bytesIn);
						if (cmd == "\\quit")
						{
							running = false;
							break;
						}

						// Unknown command
						continue;
					}

					// Send message to other clients, and definiately NOT the listening socket

					for (u_int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if (outSock != listening && outSock != sock && (completeMessage.find("/") == std::string::npos))
						{

							ostringstream ss;
							ss<< completeMessage << "\r\n";
							string strOut = ss.str();
							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	FD_CLR(listening, &master);
	closesocket(listening);

	// Message to let users know what's happening.
	string msg = "SERVER:Server is shutting down. Goodbye\r\n";

	while (master.fd_count > 0)
	{
		// Get the socket number
		SOCKET sock = master.fd_array[0];

		// Send the goodbye message
		send(sock, msg.c_str(), msg.size() + 1, 0);

		// Remove it from the master file list and close the socket
		FD_CLR(sock, &master);
		closesocket(sock);
	}

	// Cleanup winsock
	WSACleanup();

	system("pause");
	return 0;
}

