CXXFLAGS=-O2 -pipe
ECXX=-Wall -Wextra -pedantic
C=gcc
LIBVAR=-L/usr/local/lib -lpthread -lsqlite3
INCVAR=-I/usr/local/include
IEXTRAS=-I. -I./ai -I./IRC -I./protocol

.PHONY: ai irc protocol botdata strip


all: triplie cmdtriplie worktriplie botdata

complete: triplie cmdtriplie worktriplie feedtriplie botdata strip

markovs: markovtriplie gentriplie

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

triplie: ai irc protocol main.o 
	${CXX} ${CXXFLAGS} ${ECXX} ${LIBVAR} -o triplie main.o ai/ai-lib.a irc/IRC.o protocol/triprotomaster-lib.a ${IEXTRAS} ${INCVAR}

cmdtriplie: ai protocol maincmdline.o 
	${CXX} ${CXXFLAGS} ${ECXX} ${LIBVAR} -o cmdtriplie maincmdline.o ai/ai-lib.a protocol/triprotomaster-lib.a ${IEXTRAS} ${INCVAR}

feedtriplie: ai protocol mainfeed.o
	${CXX} ${CXXFLAGS} ${ECXX} ${LIBVAR} -lboost_regex-mt -o feedtriplie mainfeed.o ai/ai-lib.a protocol/triprotomaster-lib.a ${IEXTRAS} ${INCVAR}

worktriplie: ai protocol mainworker.o
	${CXX} ${CXXFLAGS} ${ECXX} ${LIBVAR} -o worktriplie mainworker.o protocol/triproto-lib.a ai/ai-lib.a protocol/triprotomaster-lib.a ${IEXTRAS} ${INCVAR}

markovtriplie: ai protocol mainfeedmarkov.o
	${CXX} ${CXXFLAGS} ${ECXX} ${LIBVAR} -o markovtriplie mainfeedmarkov.o ai/ai-lib.a protocol/triprotomaster-lib.a ${IEXTRAS} ${INCVAR}

gentriplie: ai protocol maingenmarkov.o
	${CXX} ${CXXFLAGS} ${ECXX} ${LIBVAR} -o gentriplie maingenmarkov.o ai/ai-lib.a protocol/triprotomaster-lib.a ${IEXTRAS} ${INCVAR}


mainfeedmarkov.o: mainfeedmarkov.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "mainfeedmarkov.cc" -o "mainfeedmarkov.o" -I. -I./ai ${INCVAR}

maingenmarkov.o: maingenmarkov.cc
	${CXX} ${CXXFLAGS} ${ECXX} -c "maingenmarkov.cc" -o "maingenmarkov.o" -I. -I./ai ${INCVAR}

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
