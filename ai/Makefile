CXXFLAGS=
ECXX=-Wall
LIBD=-L/usr/local/lib -lpthread -lsqlite3
INCLUDED=-I/usr/local/include

all: ai-lib.a
#libsqlitefunctions.so

ai.o: ai.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "ai.cc" -o "ai.o" -I. ${INCLUDED}

tokens.o: tokens.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "tokens.cc" -o "tokens.o" -I. ${INCLUDED}

dictionary.o: dictionary.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "dictionary.cc" -o "dictionary.o" -I. ${INCLUDED}

graph.o: graph.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "graph.cc" -o "graph.o" -I. ${INCLUDED}

newmarkov.o: newmarkov.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "newmarkov.cc" -o "newmarkov.o" -I. ${INCLUDED}

context.o: context.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "context.cc" -o "context.o" -I. ${INCLUDED}

libsqlitefunctions.so: extension-functions.c
	${CC} ${CXXFLAGS} ${ECXX} -fPIC -lm ${LIBD} -shared extension-functions.c -o libsqlitefunctions.so ${INCLUDED}

clean: 
	rm *.o

ai-lib.a: ai.o tokens.o dictionary.o graph.o newmarkov.o context.o
	ar rcs ai-lib.a ai.o tokens.o dictionary.o graph.o newmarkov.o context.o
