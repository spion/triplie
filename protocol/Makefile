

all: triproto-lib.a triprotomaster-lib.a

triproto-lib.a: triproto.o clientsocket.o serversocket.o socket.o
	ar rcs triproto-lib.a triproto.o clientsocket.o serversocket.o socket.o

triprotomaster-lib.a: triprotomaster.o socket.o
	ar rcs triprotomaster-lib.a triprotomaster.o socket.o

triproto.o: triproto.cc
	${CXX} ${CXXFLAGS} -c triproto.cc -o triproto.o

clientsocket.o: ClientSocket.cpp 
	${CXX} ${CXXFLAGS} -c ClientSocket.cpp -o clientsocket.o

serversocket.o: ServerSocket.cpp 
	${CXX} ${CXXFLAGS} -c ServerSocket.cpp -o serversocket.o

socket.o: Socket.cpp 
	${CXX} ${CXXFLAGS} -c Socket.cpp -o socket.o

triprotomaster.o: triprotomaster.cc
	${CXX} ${CXXFLAGS} -c triprotomaster.cc -o triprotomaster.o 
