#include <iostream>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
#include <cstring>
#include <map>
#include <sstream>
#include <iterator>
#include <algorithm>

#include "../../utils.hpp"

class Client
{
public:
	int fd;
	std::string username;
	struct sockaddr_in addr;

	Client(int disc, struct sockaddr_in caddr)
	{
		fd = disc;
		addr = caddr;
	}
};

class ChatRoom
{
public:
	// std::vector<int*> clients;
	std::map<std::string, Client*> clients;
	std::string name;
	std::vector<std::string> history;

	ChatRoom(std::string cr_name)
	{
		name = cr_name;
	}

	ChatRoom() {}

	void sendMessage(std::string msg)
	{
		std::map<std::string, Client*>::iterator itr;
		for (itr = this->clients.begin(); itr != this->clients.end(); ++itr)
		{
			send(itr->second->fd, msg.c_str(), strlen(msg.c_str()), 0);
		}
	}

	void removeClient(std::string user)
	{
		this->clients.erase(user);
	}

	void sendData(int client, const char *data, int buffsize)
	{
		std::map<std::string, Client*>::iterator itr;
		for (itr = this->clients.begin(); itr != this->clients.end(); ++itr)
		{
			if (itr->second->fd != client)
			{
				send(itr->second->fd, data, buffsize, 0);
			}
		}
	}

	void sendFile(int client, std::string filename, std::string username)
	{
		debugLog("Started");
		// Get file size
		long file_size;
		std::stringstream fstmp;
		char tbuffer[1];
		do
		{
			recv(client, tbuffer, 1, 0);
			debugLog(tbuffer);
			if (*tbuffer == '!')
				break;
			fstmp << tbuffer;
		} while (true);
		debugLog("Loop e");
		debugLog(fstmp.str());
		file_size = std::stoi(fstmp.str().c_str());

		// Loop for buffer size till file size is reached
		int buffer_size = 1024;
		char buffer[buffer_size];
		std::map<std::string, Client*>::iterator itr;

		// Notify clients about file reception
		std::stringstream msg;
		msg << "FILE " << filename;
		sendData(client, msg.str().c_str(), strlen(msg.str().c_str()));
		msg.str(std::string());
		msg << file_size << "!";
		sendData(client, msg.str().c_str(), strlen(msg.str().c_str()));

		while (file_size > 0)
		{
			// Receive a chunk, send it to all
			recv(client, buffer, std::min((long)buffer_size, file_size), 0);
			sendData(client, buffer, std::min((long)buffer_size, file_size));

			file_size -= std::min((long)buffer_size, file_size);	
		}

		msg.str(std::string());
		msg << getCurrentTime() << " Server: ";
		msg << this->name << std::endl;
		msg << "Received file: " << filename << std::endl;
		msg << "From: " << username << std::endl;
		msg << ">> ";
		sendData(client, msg.str().c_str(), strlen(msg.str().c_str()));

	}

	// bool operator<(const ChatRoom& t) const
 //    {
 //        return strcmp(this->name, t.name);
 //    }

	// ~ChatRoom();
	
};

ChatRoom* joinChatRoom(
	std::string chatroom_name,
	std::map<std::string, ChatRoom*> *chatrooms,
	Client *client
	)
{
	std::map<std::string, ChatRoom*>::iterator tmp;
	auto search = chatrooms->find(chatroom_name);

	std::stringstream ss;
	ss << std::endl << getCurrentTime() << " Server: ";
	
	if (search != chatrooms->end())
	{
		ss << "Joined " << chatroom_name.c_str();
		ss.clear();
		ss << std::endl << ">> ";
		send(client->fd, ss.str().c_str(), strlen(ss.str().c_str()), 0);
		search->second->clients.insert(std::make_pair(client->username, client));
		return search->second;
	}
	else
	{
		ss << "No such chatroom found!" << std::endl << ">> ";
		send(client->fd, ss.str().c_str(), strlen(ss.str().c_str()), 0);
		return nullptr;
	}

}

void startServerSession(
	int client,
	std::map<std::string, ChatRoom*> *chatrooms,
	std::map<int, Client*> *clients
	)
{
	std::string tbuffer = "Connected to server.\n";
	send(client, tbuffer.c_str(), strlen(tbuffer.c_str()), 0);

	bool kill_client = false, initialized = false;
	int buffer_size = 1024;
	char buffer[buffer_size];
	std::string username;
	std::map <std::string, ChatRoom*> :: iterator cr;
	ChatRoom *current_chatroom;
	// const std::map <std::string, ChatRoom*> :: const_iterator NullIterator(NULL);

	while (client > 0 && !kill_client)
	{
		// Get username from client
		while (!initialized)
		{
			memset(buffer, 0, sizeof buffer);
			recv(client, buffer, buffer_size, 0);

			username = std::string(buffer);
			auto search = clients->find(client);
			if (search != clients->end())
			{
				// username[strlen(username.c_str()) - 1] = '\0';
				search->second->username = username;
				initialized = true;
			}
			else
			{
				std::stringstream ss;
				ss << std::endl << getCurrentTime() << " Server: ";
				ss << "Something went wrong with your connection. Reconnect and try again.";
				ss << std::endl << ">> ";
				send(client, ss.str().c_str(), strlen(ss.str().c_str()), 0);
				close(client);
			}
			// debugLog(search->second->username);
		}


		// Get command from client
		do
		{
			memset(buffer, 0, sizeof buffer);
			recv(client, buffer, buffer_size, 0);
			// std::cout << buffer;

			// If not in a chatroom
			if (!current_chatroom)
			{

				// Exit condition
				if ( strncmp(buffer, "exit", 4) == 0 )
				{	
					// Send Good bye message
					std::string msg = "Bye!\n";
					send(client, msg.c_str(), strlen(msg.c_str()), 0);

					// Stop the loop
					*buffer = '*';
					kill_client = true;

				}
				// Get list of chatrooms
				else if ( strncmp(buffer, "list chatrooms", 14) == 0 )
				{
					// Send list of active chat rooms

					// No Chatrooms
					if (chatrooms->empty())
					{
						std::stringstream ss;
						ss << std::endl << getCurrentTime() << " Server: ";
						ss << "There are no active chat rooms available." << std::endl << ">> ";
						send(client, ss.str().c_str(), strlen(ss.str().c_str()), 0);
					}

					// Send available chatrooms
					else
					{
						std::stringstream ss;
						ss << std::endl << getCurrentTime() << " Server:" << std::endl;
						for (cr = chatrooms->begin(); cr != chatrooms->end(); ++cr)
						{
							ss << "  - " << cr->first.c_str();// << std::endl;
							// std::cout << cr->first.c_str() << std::endl;
						}
						ss << std::endl << ">> ";
						send(client, ss.str().c_str(), strlen(ss.str().c_str()), 0);
					}

				}

				// Join chatroom
				else if ( strncmp(buffer, "join chatroom", 13) == 0 )
				{
					// String of the "create" word and tailing "\n"
					char *tmp = buffer + 14;
					std::string cr_name(tmp);
					// cr_name[cr_name.length() - 1] = '\0';

					current_chatroom = joinChatRoom(cr_name, chatrooms, clients->find(client)->second);
				}

				// Create chatroom
				else if ( strncmp(buffer, "create chatroom", 15) == 0 )
				{
					// String of the "create" word and tailing "\n"
					char *tmp = buffer + 16;
					std::string cr_name(tmp);
					// cr_name[cr_name.length() - 1] = '\0';

					// Push it into available chatrooms
					chatrooms->insert(std::make_pair(cr_name, new ChatRoom(cr_name)));
					current_chatroom = joinChatRoom(cr_name, chatrooms, clients->find(client)->second);

				}

				// Unknown command
				else
				{
					std::stringstream ss;
					ss << std::endl << getCurrentTime() << " Server: ";
					ss << "Unkown command received!" << std::endl << ">> ";
					send(client, ss.str().c_str(), strlen(ss.str().c_str()), 0);
				}
			}

			// Already in a chat room
			else
			{
				// Exit chatroom
				if ( strncmp(buffer, "exit chatroom", 13) == 0 )
				{	
					// Remove user from chatroom
					current_chatroom->removeClient(username);

					// Send Good bye message
					std::stringstream ss;
					ss << std::endl << getCurrentTime() << " Server: ";
					// std::cout << current_chatroom->name.c_str() << std::endl;
					ss << "Exiting chatroom!" << std::endl << ">> ";
					send(client, ss.str().c_str(), strlen(ss.str().c_str()), 0);

					// Exit chatroom
					current_chatroom = nullptr;

				}

				// List of all users in chatroom
				else if ( strncmp(buffer, "list users", 10) == 0 )
				{
					std::map<std::string, Client*>::iterator i;
					std::stringstream ss;

					ss << std::endl << getCurrentTime() << " Server: " << current_chatroom->name;
					
					for (i = current_chatroom->clients.begin(); i != current_chatroom->clients.end(); ++i)
					{
						ss << "  - " << i->second->username;
					}
					ss << std::endl << ">> ";
					// debugLog(ss.str());

					send(client, ss.str().c_str(), strlen(ss.str().c_str()), 0);
				}

				// Add someone to the chatroom

				// Send file to all users in chatroom over TCP
				else if ( strncmp(buffer, "reply tcp", 9) == 0 )
				{
					char *tmp = buffer + 10;
					std::string filename(tmp);
					debugLog(filename);
					std::cout << "lalalalalal" << std::endl;
					current_chatroom->sendFile(client, filename, username);
				}
				
				// Send message to all users in chatroom
				else if ( strncmp(buffer, "reply", 5) == 0 )
				{
					char *tmp = buffer + 6;
					std::string msg(tmp);

					std::stringstream ss;
					ss << "At " << getCurrentTime() << ", " << username;
					ss << "   " << msg << std::endl << ">> ";
					// debugLog(ss.str());
					current_chatroom->sendMessage(ss.str());
				}

				// Unkown command
				else
				{
					std::stringstream ss;
					ss << std::endl << getCurrentTime() << " Server: ";
					ss << current_chatroom->name;
					ss.clear();
					ss << std::endl;
					ss << "Unknown command received!" << std::endl << ">> ";
					send(client, ss.str().c_str(), strlen(ss.str().c_str()), 0);
				}

			}

		} while (*buffer != '*');

		// Send status and data from server

		// Get clients response
		// This can be data or ackno
	}

	close(client);
}
