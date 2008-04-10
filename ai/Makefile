CXXFLAGS=-Wall -g -fno-exceptions

all: ai.o tokens.o dictionary.o graph.o markov.o

ai.o: ai.cc
	${CXX} ${CXXFLAGS} -c "ai.cc" -o "ai.o" -I.

tokens.o: tokens.cc
	${CXX} ${CXXFLAGS} -c "tokens.cc" -o "tokens.o" -I.

dictionary.o: dictionary.cc
	${CXX} ${CXXFLAGS} -c "dictionary.cc" -o "dictionary.o" -I.

graph.o: graph.cc
	${CXX} ${CXXFLAGS} -c "graph.cc" -o "graph.o" -I.

markov.o: markov.cc
	${CXX} ${CXXFLAGS} -c "markov.cc" -o "markov.o" -I.

newmarkov.o: newmarkov.cc
	${CXX} ${CXXFLAGS} -c "newmarkov.cc" -o "newmarkov.o" -I.

clean: 
	rm *.o
