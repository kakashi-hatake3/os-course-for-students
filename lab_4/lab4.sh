g++ -c parent.cpp
g++ -c child.cpp

g++ -o parent parent.o
g++ -o child child.o

chmod +x ./parent 
chmod +x ./child 
export PATH="${PATH}:$(pwd)"
