.PHONY:all
all:client server	
server:NetCalServer.cc
	g++ -o $@ $^ -std=c++17 -lpthread -ljsoncpp
client:NetCalClient.cc
	g++ -o $@ $^ -std=c++17 -lpthread -ljsoncpp
.PHONY:clean
clean:
	rm -f server client
