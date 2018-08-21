#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>
#include <thread>
#include <signal.h>
#include <algorithm>

#include "utils.hpp"

int client;
bool isExit = false, stop_receiver = false;

void printUsage()
{
	std::cout << "Usage: ./client <server_ip> <port> <username>" << std::endl;
	exit(1);
}

void receiver(bool *exit, int *sclient)
{
	int buffer_size = 1024;
	char buffer[buffer_size];

	while (!isExit)
	{
		memset(buffer, 0, sizeof buffer);
		recv(*sclient, buffer, buffer_size, 0);
		// debugLog(buffer);

		if ( strncmp(buffer, "FILE", 4) == 0 )
		{
			char *tmp = buffer + 5;
			std::string filename(tmp);
			// filename[filename.length() - 1] = '\0';
			// debugLog(filename);

			// memset(buffer, 0, sizeof buffer);
			// recv(*sclient, buffer, buffer_size, 0);
			// debugLog(buffer);

			long file_size;
			std::stringstream tmps;
			do
			{
				memset(buffer, 0, sizeof buffer);
				recv(client, buffer, 1, 0);
				// debugLog(buffer);
				if (*buffer == '!')
					break;
				tmps << buffer;
			} while (true);

			file_size = std::stoi(tmps.str().c_str());
			// debugLog(file_size);

			FILE *fd = fopen(filename.c_str(), "wb");

			if (fd != NULL)
			{

				while (file_size > 0)
				{
					recv(*sclient, buffer, std::min(file_size, (long)buffer_size), 0);
					// debugLog(std::min(file_size, (long)buffer_size));
					// debugLog(buffer);
					
					int offset = 0;
					
					do
					{
						size_t written_size = fwrite(&buffer[offset], 1, std::min(file_size, (long)buffer_size)-offset, fd);
						offset += written_size;
					} while (offset < std::min(file_size, (long)buffer_size));

					file_size -= std::min(file_size, (long)buffer_size);
				}
				
			}

			fclose(fd);
			memset(buffer, 0, sizeof buffer);
			std::cout << ">> ";

		}
		else
			std::cout << buffer;
	}
}

void closeClient(int s)
{
	std::cout << "Exiting client..." << std::endl;
	isExit = true;
	std::string msg = "exit";
	send(client, msg.c_str(), strlen(msg.c_str()), 0);
	// close(client);
}

int main(int argc, char const *argv[])
{

	signal(SIGINT, closeClient);
	// struct sigaction sigIntHandler;
	// sigIntHandler.sa_handler = closeServer;
	// sigemptyset(&sigIntHandler.sa_mask);
	// sigIntHandler.sa_flags = 0;
	// sigaction(SIGINT, &sigIntHandler, NULL);

	if (argc != 4)
	{
		printUsage();
	}

	// int client;
	int server_port = std::stoi(argv[2]);; 
	// bool isExit = false, stop_receiver = false;
	int bufsize = 1024;
	char buffer[bufsize];
	std::string server_ip(argv[1]);
	std::string username(argv[3]);

	struct sockaddr_in server_addr;

	client = socket(AF_INET, SOCK_STREAM, 0);

	if (client < 0) 
	{
		std::cout << "\nError establishing socket..." << std::endl;
		exit(1);
	}

	std::cout << "\n=> Socket client has been created..." << std::endl;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);

	inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);
	if (connect(client,(struct sockaddr *)&server_addr, sizeof(server_addr)) == 0)
		std::cout << "=> Connection to the server port number: " << server_port << std::endl;

	std::cout << "=> Awaiting confirmation from the server..." << std::endl;
	recv(client, buffer, bufsize, 0);
	std::cout << "=> Connection confirmed, you are good to go..." << std::endl;
	std::cout << std::endl << ">> ";


	std::stringstream userss;
	userss << username << std::endl;
	send(client, userss.str().c_str(), strlen(userss.str().c_str()), 0);
	std::thread receiver_thread(receiver, &stop_receiver, &client);

	do {
		std::string msg;
		getline(std::cin, msg);

		std::stringstream s;
		s << msg;
		// std::cin >> std::ws;

		// std::cout << msg << std::endl;
		if (strncmp(msg.c_str(), "reply tcp", 9) == 0)
		{

			const char *tmp = msg.substr(10,msg.length()).c_str();
			// std::string filename(tmp);

			FILE *fd = fopen(tmp, "rb");

			fseek(fd, 0, SEEK_END);
			long file_size = ftell(fd);
			rewind(fd);

			std::stringstream ss;
			ss << file_size << "!";
			send(client, s.str().c_str(), strlen(s.str().c_str()), 0);
			send(client, ss.str().c_str(), strlen(ss.str().c_str()), 0);

			char buffer[1024];
	        
			do
			{
				size_t curr = std::min(file_size, (long)1024);
				curr = fread(buffer, 1, curr, fd);
				send(client, buffer, curr, 0);
				file_size -= curr;
			}
			while (file_size > 0);
			fclose(fd);

		}

		else if (!msg.empty())
		{
			s << std::endl;
			send(client, s.str().c_str(), strlen(s.str().c_str()), 0);
		}	

		if (strcmp(msg.c_str(), "exit") == 0)
		{
			isExit = true;
			recv(client, buffer, bufsize, 0);
		}

	} while (!isExit);

	std::cout << "\n=> Connection terminated.\nGoodbye...\n";
	exit(0);
	return 0;
}