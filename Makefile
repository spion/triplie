CXXFLAGS=
ECXX=-Wall
C=gcc
LIBVAR=-L/usr/local/lib -lpthread -lsqlite3
INCVAR=-I/usr/local/include
IEXTRAS=-I. -I./ai -I./IRC -I./protocol

.PHONY: ai irc protocol botdata strip


all: triplie cmdtriplie feedtriplie worktriplie botdata


ai: 
	make -C ai

irc:
	make -C irc

protocol: 
	make -C protocol

botdata:
	make -C botdata

bootstrap: 
	make -C botdata bootstrap

triplie: main.o ai irc protocol
	${CXX} ${CXXFLAGS} ${ECXX} ${LIBVAR} -o triplie main.o ai/ai.o ai/tokens.o irc/IRC.o ai/dictionary.o ai/newmarkov.o ai/graph.o ai/context.o protocol/triprotomaster-lib.a ${IEXTRAS} ${INCVAR}

cmdtriplie: maincmdline.o ai protocol
	${CXX} ${CXXFLAGS} ${ECXX} ${LIBVAR} -o cmdtriplie maincmdline.o ai/ai.o ai/tokens.o ai/dictionary.o ai/newmarkov.o ai/graph.o ai/context.o protocol/triprotomaster-lib.a ${IEXTRAS} ${INCVAR}

feedtriplie: mainfeed.o ai protocol
	${CXX} ${CXXFLAGS} ${ECXX} ${LIBVAR} -o feedtriplie mainfeed.o ai/ai.o ai/tokens.o ai/dictionary.o ai/newmarkov.o ai/graph.o ai/context.o protocol/triprotomaster-lib.a ${IEXTRAS} ${INCVAR}

worktriplie: mainworker.o ai protocol
	${CXX} ${CXXFLAGS} ${ECXX} ${LIBVAR} -o worktriplie mainworker.o protocol/triproto-lib.a ai/ai.o ai/tokens.o ai/dictionary.o ai/newmarkov.o ai/graph.o ai/context.o protocol/triprotomaster-lib.a ${IEXTRAS} ${INCVAR}



main.o: main.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "main.cc" -o "main.o" -I. -I./ai ${INCVAR}

maincmdline.o: maincmdline.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "maincmdline.cc" -o "maincmdline.o" -I. -I./ai ${INCVAR}

mainfeed.o: mainfeed.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "mainfeed.cc" -o "mainfeed.o" -I. -I./ai ${INCVAR}

mainworker.o: mainworker.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "mainworker.cc" -o "mainworker.o" -I. -I./ai ${INCVAR}



clean:
	rm *.o ai/*.o irc/*.o protocol/*.o botdata/tdb protocol/*.a 2> /dev/null

strip:
	strip -s *triplie
