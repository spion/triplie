DEBUG=-g

all: ai.o tokens.o dictionary.o graph.o markov.o

ai.o: ai.cc
	g++ -Wall ${DEBUG} -c "ai.cc" -o "ai.o" -I.

tokens.o: tokens.cc
	g++ -Wall ${DEBUG} -c "tokens.cc" -o "tokens.o" -I.

dictionary.o: dictionary.cc
	g++ -Wall ${DEBUG} -c "dictionary.cc" -o "dictionary.o" -I.

graph.o: graph.cc
	g++ -Wall ${DEBUG} -c "graph.cc" -o "graph.o" -I.

markov.o: markov.cc
	g++ -Wall ${DEBUG} -c "markov.cc" -o "markov.o" -I.

clean: 
	rm *.o
