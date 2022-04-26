#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include "UserAccount.cpp"
#include <vector>
#include<thread>

using namespace std::this_thread;

#pragma comment (lib, "ws2_32.lib")

using namespace std;

// A job of thread will be processed in here
	string performTask(string word)
	{
		string bestSimilarWord = "no suggestion";
		int bestCharSimilar = 0;

		string line;
		ifstream myfile("wordDictionary.txt");

		if (myfile.is_open())
		{
			while (myfile.good())
			{
				//Get each line in directory
				getline(myfile, line);
				//Compare length of misspelt word and selected line. Accept table length is -1,0,+1. To prevent too long/too short misspelt word.
				if ((line.length() == word.length()) || ((line.length() + 1) == word.length()) || ((line.length() - 1) == word.length())) {
					//Compare first char of both words
					if (line.substr(0, 1).compare(word.substr(0, 1)) == 0) {
						int similarChar = 0;
						string copyWord = line;
						for (int i = 0; i < word.length(); i++) {
							if (copyWord.find(word.substr(i,(i+1))) != std::string::npos) {
								//cout << copyWord <<"-"<<word.substr(i,(i+1))<<"-" << copyWord.find_first_of(word.substr(i, (i+1))) << endl;
								int a = copyWord.find_first_of(word.substr(i, (i+1)));
								copyWord.erase( a, (a+1));
								similarChar++;

							}
						}
						if (similarChar > bestCharSimilar) {
							bestSimilarWord = line;
							bestCharSimilar = similarChar;
						}

					}

				}
			}
			myfile.close();
			//cout << bestSimilarWord<<"-"<<bestCharSimilar << endl;
		}

		else cout << "Unable to open file";
		return bestSimilarWord;
	}

	void performThread(string listOfWord,int fromLine, int toLine,int threadNumber) {
		//Write to file
		ofstream myfile;
		string fileName = "ThreadNumber" + to_string(threadNumber) + ".txt";
		
		myfile.open(fileName);
		for (int i = fromLine; i < toLine+1; i++){
			stringstream ss(listOfWord);
			int currentLine = 1;
			for (std::string line; std::getline(ss, line); )
			{
				currentLine++;
				if (currentLine == i) {
					string suggestionWord = performTask(line);
					myfile << suggestionWord << endl;
				}
			}

		}

		//Write databack

		//myfile << output;
		myfile.close();

	}


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
						/*
						//send to client to accept
						if (UserAccount::accountLoggin(newAccount)) {
							//Succesfull register account
							loginResult = "/AcceptLoggin";
						}
						else {
							loginResult = "/RejectLoggin";
						}
						*/
						//By pass login
						loginResult = "/AcceptLoggin";


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
				else if (completeMessage.find("/File") != std::string::npos) {
					//get Number of thread:
					int totalThreads = 1;	//Default threads
					totalThreads = (int)completeMessage[0];	//Get total threads from client
					try {	//Check incase invalid threads. If Invalid
						cout << completeMessage[0] << endl;
						totalThreads =completeMessage[0] -48;	//Convert string to int
					}
					catch (const std::exception& e) {
						cout << "invalid number of thread from client. Use only 1 thread" << endl;
					}

					completeMessage = completeMessage.erase(0, 8);	//Remove some system-syntax symbol.


					//Write to local file as backup
					ofstream myfile;
					myfile.open("backup.txt");


					//convert to lower string
					for (int i = 0; i < completeMessage.length(); i++)
						completeMessage[i] = tolower(completeMessage[i]);
					//Get total line and save data to local file to backupfile
					int totalLine = 0;
					stringstream ss(completeMessage);
					for (std::string line; std::getline(ss, line); )
					{
						myfile << line;
						totalLine ++;
					}
					myfile.close();



					//Divine job to each threads. Job each thread= TotalLine % total threads. And the last thread take the reminder Line
					int jobForThread = (totalLine -(totalLine % totalThreads))/totalThreads;
					//cout <<totalLine<<"-"<<totalThreads<<"-" << jobForThread << endl;
					//AssignJobTo Threads.
					int lineNumber = 1;
					int totalAssignedLine = 0;
					for (int i = 0; i < totalThreads; i++) {
						lineNumber += jobForThread;
						if (i != totalThreads - 1) {
							thread th1(performThread, completeMessage, i * jobForThread + 1, (i + 1) * jobForThread,i+1);
							th1.detach();

						}
						else {
							//Last threads will take reminder
							thread th1(performThread, completeMessage, i* jobForThread + 1, (i + 1)* jobForThread+ (totalLine % totalThreads)+1,i+1);
							cout << "Wait for all threads complete" << endl;
							th1.join();	//The program will wait the last thread executated to continue
							//Last thread took the most jobs so the system just need to wait the last threads complete jobs <-> all threads complete.

						}
					}
					cout << "All threads completed" << endl;
					//Wait for the last thread finish
					boolean treadhFinishJob = false;
					while (!treadhFinishJob) {
						ifstream lastThreadFileOutPut;
						string fileName = "ThreadNumber" + to_string(totalThreads) + ".txt";
						lastThreadFileOutPut.open(fileName);

							treadhFinishJob = true;
							//Merge all outfile from threads:
							ofstream outputFile;
							remove("output.txt");
							outputFile.open("output.txt");
							string outputFileData = "/WordSuggestionOutput\\n";
							for (int i = 1; i < totalThreads+1; i++) {
								string threadOutputFileName = "ThreadNumber" + to_string(i) + ".txt";
								ifstream fromThreadOutputFile(threadOutputFileName);
								string lineData;
								while (std::getline(fromThreadOutputFile, lineData))
								{
									outputFileData += lineData + "-";
									outputFile << lineData << endl;
								}

								fromThreadOutputFile.close();
							}
							outputFile.close();

							//Send output file to all clients
							cout << "Sending Output to Client..." << endl;
									ostringstream ss;
									ss << outputFileData << "\r\n";
									string strOut = ss.str();
									send(sock, strOut.c_str(), strOut.size() + 1, 0);
									cout << "Sent completed." << endl;


					}



				}

				if (bytesIn <= 0)
				{
					// Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}

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
							cout << "a" << endl;
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

	//system("pause");
	return 0;
}

