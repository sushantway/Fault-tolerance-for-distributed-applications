all: server front client clean

server: server.cpp
	g++ server.cpp -o server -lpthread

front: front.cpp
	g++ front.cpp -o front -lpthread

client: client.cpp
	g++ client.cpp -o client

clean:
	rm -rf *o
