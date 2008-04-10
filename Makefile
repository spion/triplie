CXXFLAGS=-Wall -g -fno-exceptions

all:triplie cmdtriplie

triplie: main.o ai/tokens.o irc/IRC.o ai/ai.o ai/dictionary.o ai/newmarkov.o ai/graph.o
	${CXX} ${CXXFLAGS} -o triplie main.o ai/ai.o ai/tokens.o irc/IRC.o ai/dictionary.o ai/newmarkov.o ai/graph.o -I. -I./ai -I./IRC

cmdtriplie: maincmdline.o ai/ai.o ai/tokens.o ai/dictionary.o ai/newmarkov.o ai/graph.o
	${CXX} ${CXXFLAGS} -o cmdtriplie maincmdline.o ai/ai.o ai/tokens.o ai/dictionary.o ai/newmarkov.o ai/graph.o -I. -I./ai

main.o: main.cc
	${CXX} ${CXXFLAGS} -c "main.cc" -o "main.o" -I. -I./ai

maincmdline.o: maincmdline.cc
	${CXX} ${CXXFLAGS} -c "maincmdline.cc" -o "maincmdline.o" -I. -I./ai

clean:
	rm *.o ai/*.o irc/*.o
