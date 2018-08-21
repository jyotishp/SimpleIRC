#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <vector>

#include "utils.hpp"

class Server
{

public:
	int port, clients_capacity, server_socket;
	struct sockaddr_in server_addr;
	socklen_t server_addr_size;
	bool exitServer = false;
	// std::thread master_thread;
	std::vector<std::unique_ptr <std::thread> > client_threads;
	std::map<int, Client*> clients;
	std::vector<int> client_fds;
	std::map<std::string, ChatRoom*> chatrooms;
	// std::vector<ChatRoom> chatrooms;

	int acceptClient()
	{
		int client_conn;
		struct sockaddr_in client_addr;
		socklen_t client_addr_size;

		client_conn = accept(
			server_socket,
			(struct sockaddr*) &client_addr,
			&server_addr_size
		);

		if (client_conn < 0)
		{
			printCurrentTime();
			std::cout << "Couldn't accept connection" << std::endl;
		}

		printCurrentTime();
		std::cout << "OPEN | " << inet_ntoa(client_addr.sin_addr) << " | "
			<< ntohs(client_addr.sin_port) << std::endl;

		return client_conn;
	}

	Server()
	{

	}

	Server(int clients_cap, int server_port = 8000)
	{

		// Exit if wrong number of clients are specified
		if (clients_cap < 1)
		{
			throw "Number of clients should be greater than 0.";
		}

		// Exit if usign a port less than 1000
		if (server_port < 1000)
		{
			throw "Using privilaged ports is not allowed. Use a port greater than 1000.";
		}

		// Save clients capacity and server port to the server object
		clients_capacity = clients_cap;
		port = server_port;

		// Get a TCP socket for server
		server_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (server_socket < 0)
		{
			throw "Socket could not be created.";
		}
		std::cout << "Starting server on 0.0.0.0:" << port << "..." << std::endl;

		// Define server address
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htons(INADDR_ANY);
		server_addr.sin_port = htons(port);
		server_addr_size = sizeof(server_addr);

		// Try to bind on given socket
		int bind_result;
		bind_result = bind(
			server_socket,
			(struct sockaddr*)& server_addr,
			server_addr_size
		);

		if (bind_result < 0)
		{
			throw "Error establishing connection. Port already in use.";
		}

		listen(server_socket, 5);
		std::cout << "Waiting for clients..." << std::endl;

		// Accept client connections
		struct sockaddr_in client_addr;
		socklen_t client_addr_size;
		int client_conn;

		while (!exitServer)
		{
			client_fds.push_back(accept(
				server_socket,
				(struct sockaddr*) &client_addr,
				&server_addr_size
			));

			if (client_fds.back() < 0)
			{
				printCurrentTime();
				std::cout << "Couldn't accept connection" << std::endl;
			}

			printCurrentTime();
			std::cout << "OPEN | " << inet_ntoa(client_addr.sin_addr) << " | "
				<< ntohs(client_addr.sin_port) << std::endl;
			
			// Accept only till max clients cap is reached
			if (client_threads.size() < clients_capacity)
			{
				clients.insert(std::make_pair(client_fds.back(), new Client(client_fds.back(), client_addr)));
				client_threads.emplace_back(new std::thread(startServerSession, client_fds.back(), &chatrooms, &clients));
			}

			// Reject if more than client max cap
			else
			{
				printCurrentTime();
				std::cout << "Maximum client connections reached!" << std::endl;
			}
		}

		close(server_socket);

	}

	~Server()
	{
		std::cout << "Server on 0.0.0.0:" << port << " stopped" << std::endl;
	}

	void closeServer()
	{
		std::cout << "Received shutdown. Shutting down server..." << std::endl;
		exitServer = true;

	}
	
};