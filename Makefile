CXXFLAGS+=-O2 -pipe
ECXX=-Wall -Wextra -pedantic
LIBVAR=-L/usr/local/include -lsqlite3 -lpthread -lboost_regex-mt
INCVAR=-I/usr/local/include
IEXTRAS=-I. -I./ai -I./IRC -I./protocol
V=-DVER="\"$(shell git log -n1 --format=%H HEAD)\"" -DVERDATE="\"$(shell date -u -d@`git log -n1 --format=%at HEAD` +%FT%TZ)\""

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


triplie: ai irc protocol main.o common.o
	${CXX} ${CXXFLAGS} ${ECXX}  -o triplie main.o common.o ai/ai-lib.a irc/IRC.o protocol/triprotomaster-lib.a ${LIBVAR} ${IEXTRAS} ${INCVAR}

cmdtriplie: ai protocol maincmdline.o common.o
	${CXX} ${CXXFLAGS} ${ECXX}  -o cmdtriplie maincmdline.o common.o ai/ai-lib.a protocol/triprotomaster-lib.a ${LIBVAR} ${IEXTRAS} ${INCVAR}

feedtriplie: ai protocol mainfeed.o common.o
	${CXX} ${CXXFLAGS} ${ECXX} -o feedtriplie mainfeed.o common.o ai/ai-lib.a protocol/triprotomaster-lib.a  ${LIBVAR} ${IEXTRAS} ${INCVAR} 

worktriplie: ai protocol mainworker.o common.o
	${CXX} ${CXXFLAGS} ${ECXX} -o worktriplie mainworker.o  common.o protocol/triproto-lib.a ai/ai-lib.a protocol/triprotomaster-lib.a  ${LIBVAR} ${IEXTRAS} ${INCVAR}

markovtriplie: ai protocol mainfeedmarkov.o
	${CXX} ${CXXFLAGS} ${ECXX} -o markovtriplie mainfeedmarkov.o ai/ai-lib.a protocol/triprotomaster-lib.a  ${LIBVAR} ${IEXTRAS} ${INCVAR} 

gentriplie: ai protocol maingenmarkov.o
	${CXX} ${CXXFLAGS} ${ECXX} -o gentriplie maingenmarkov.o ai/ai-lib.a protocol/triprotomaster-lib.a ${LIBVAR} ${IEXTRAS} ${INCVAR}

%.o: %.cc
	$(CXX) $(CXXFLAGS) ${ECXX} ${DEBUG} -c $< -o $@ -I. -I./ai ${INCVAR} ${V}

clean:
	rm -f *.o ai/*.o irc/*.o protocol/*.o botdata/tdb protocol/*.a 2> /dev/null

strip:
	strip -s *triplie
