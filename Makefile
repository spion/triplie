CC=g++
CFLAGS=-Wall
DEBUG=-g

all:triplie cmdtriplie

triplie: main.o ai/tokens.o irc/IRC.o ai/ai.o ai/dictionary.o ai/markov.o ai/graph.o
	g++ -Wall ${DEBUG} -o triplie main.o ai/ai.o ai/tokens.o irc/IRC.o ai/dictionary.o ai/markov.o ai/graph.o -I. -I./ai -I./IRC

cmdtriplie: maincmdline.o ai/ai.o ai/tokens.o ai/dictionary.o ai/markov.o ai/graph.o
	g++ -Wall ${DEBUG} -o cmdtriplie maincmdline.o ai/ai.o ai/tokens.o ai/dictionary.o ai/markov.o ai/graph.o -I. -I./ai

main.o: main.cc
	g++ -Wall ${DEBUG} -c "main.cc" -o "main.o" -I. -I./ai

maincmdline.o: maincmdline.cc
	g++ -Wall ${DEBUG} -c "maincmdline.cc" -o "maincmdline.o" -I. -I./ai

clean:
	rm *.o ai/*.o irc/*.o
