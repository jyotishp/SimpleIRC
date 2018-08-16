#include <string>
#include <signal.h>

#include "server.hpp"

using namespace Server;

int printUsage()
{
	std::cout << "Usage: ./server num_of_clients [port]" << std::endl;
	exit(1);
}

int main(int argc, char const *argv[])
{
	int port, clients;

	// Input parsing
	if (argc < 2 || argc > 3)
	{
		printUsage();
	}

	// Define default port
	port = (argc == 2) ? 8000 : std::stoi(argv[2]);
	clients = std::stoi(argv[1]);

	try
	{
		Server server(clients, port);
		struct sigaction sigIntHandler;

		sigIntHandler.sa_handler = server.closeServer;
		sigemptyset(&sigIntHandler.sa_mask);
		sigIntHandler.sa_flags = 0;

		sigaction(SIGINT, &sigIntHandler, NULL);
	}
	catch (const char* msg)
	{
		std::cerr << msg << std::endl;
		return 1;
	}

	return 0;
}