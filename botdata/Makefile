V=-DVER="\"$(shell git log -n1 --format=%H HEAD)\"" -DVERDATE="\"$(shell date -u -d@`git log -n1 --format=%at HEAD` +%FT%TZ)\""

all: tdb

.PHONY:bootstrap clean

bootstrap: tdb
	./tdb

regenbootstrap:tdb
	./tdb noindex

tdb: transferdb.cc common.o
	${CXX} ${CXXFLAGS} transferdb.cc -o tdb ../common.o -lsqlite3 -lpthread -L/usr/local/lib -I/usr/local/include ${V}

drawbrain: drawbrain.o zpr.o
	${CXX} ${CXXFLAGS} drawbrain.o zpr.o -o drawbrain -lGL -lglut -lsqlite3

drawbrain.o: drawbrain.cpp zpr.h
	${CXX} ${CXXFLAGS} -c drawbrain.cpp -o drawbrain.o

zpr.o: zpr.c zpr.h
	${CXX} ${CXXFLAGS} -c zpr.c -o zpr.o

common.o: ../common.cc
	${CXX} ${CXXFLAGS} -c ../common.cc -o ../common.o

clean: 
	rm -f *.o 2> /dev/null
