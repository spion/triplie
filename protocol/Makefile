ECXX="-Wall"

all: triproto-lib.a triprotomaster-lib.a

triproto-lib.a: triproto.o clientsocket.o serversocket.o socket.o
	ar rcs triproto-lib.a triproto.o clientsocket.o serversocket.o socket.o

triprotomaster-lib.a: triprotomaster.o socket.o
	ar rcs triprotomaster-lib.a triprotomaster.o socket.o

triproto.o: triproto.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c triproto.cc -o triproto.o

clientsocket.o: ClientSocket.cpp 
	${CXX} ${CXXFLAGS} ${ECXX} -c ClientSocket.cpp -o clientsocket.o

serversocket.o: ServerSocket.cpp 
	${CXX} ${CXXFLAGS} ${ECXX} -c ServerSocket.cpp -o serversocket.o

socket.o: Socket.cpp 
	${CXX} ${CXXFLAGS} ${ECXX} -c Socket.cpp -o socket.o

triprotomaster.o: triprotomaster.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c triprotomaster.cc -o triprotomaster.o 
