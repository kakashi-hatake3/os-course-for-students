g++ -c client.cpp
g++ -c server.cpp

g++ -o client client.o -lpthread
g++ -o server server.o -lpthread

