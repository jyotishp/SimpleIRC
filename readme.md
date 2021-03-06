# Simple IRC Client-Server
A simple IRC implementation in C++.

## Compiling and running on single node
The `server.sh` file has the commands to compile both server and client. To start the server,
```
./server.sh <num_of_clients>
```

To launch clients,
```
./client.sh <name_of_client>
```

## Running on multiple nodes
### Compile the code
To compile server, run
```
g++ -fconcepts -pthread server.cpp -o server
```

To compile the client, run
```

g++ -fconcepts -pthread client.cpp -o client
```

### Running the program
To run the server,
```
./server <num_of_clients> <server_port>
```

To run the client,
```
./client <server_ip> <server_port> <username_of_client>
```

#### Running using telnet
To connect to server using telnet,
```
telnet <server_ip> <server_port>
```
Then give the username of the client as soon as it connects. You will get prompts from there after.

## Usage
On clients,
### Outside chatroom
- `list chatrooms` -- List of available chatrooms on server.
- `create chatroom <cr_name>` -- Create a chatroom with the name `cr_name` and join it.
- `join chatroom <cr_name>` -- Join an existing chatroom with the name `cr_name`.
- `exit` -- Close the connection with the server.

### Inside chatroom
- `list users` -- List of users in the existing chatroom.
- `reply <msg>` -- Broadcast `msg` to everyone in the chatroom.
- `reply tcp <file_path>` -- Send file to everyone in the chatroom (Doesn't work on telnet).
- `exit chatroom` -- Exits the chatroom.

## Features
- Single server, multiple clients based model.
- Broadcasts messages to all the clients in the chatroom.

## To-do
- When client exits, the pointer still remains in the clients variable. Need to remove the exited client.
- In `client.cpp`, the output needs to be printed at once (it stops after a linebreak).
- In `client.cpp`, handle triggering `exit` command when inside a chatroom.
- Implement unique usernames.
- Chatroom chat history.
- Improve server side logging.
- Add user connected to the server to a chatroom.
- Client-Client direct messaging.
