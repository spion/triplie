all: tdb

.PHONY:bootstrap clean

bootstrap: tdb
	./tdb

regenbootstrap:tdb
	./tdb noindex

tdb: transferdb.cc
	${CXX} ${CXXFLAGS} transferdb.cc -o tdb -lsqlite3 -lpthread -L/usr/local/lib -I/usr/local/include

triplie.db: tdb
	./tdb

drawbrain: drawbrain.o zpr.o
	${CXX} ${CXXFLAGS} drawbrain.o zpr.o -o drawbrain -lGL -lglut -lsqlite3

drawbrain.o: drawbrain.cpp zpr.h
	${CXX} ${CXXFLAGS} -c drawbrain.cpp -o drawbrain.o

zpr.o: zpr.c zpr.h
	${CXX} ${CXXFLAGS} -c zpr.c -o zpr.o


clean: 
	rm *.o 2> /dev/null
