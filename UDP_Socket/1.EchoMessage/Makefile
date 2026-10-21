.PHONY:all
all:client server	
server:Server.cc
	g++ -o $@ $^ -std=c++17 -lpthread
client:EchoClient_udp.cc
	g++ -o $@ $^ -std=c++17 -lpthread
.PHONY:clean
clean:
	rm -f server client
