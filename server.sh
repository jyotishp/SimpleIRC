#!/bin/bash

g++ -fconcepts -pthread server.cpp -o server
g++ -fconcepts -pthread client.cpp -o client
./server $1 8888
