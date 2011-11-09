CC=g++
CFLAGS=-g
#CFLAGS=-O3 -g -funroll-loops
LDFLAGS=-O2 -lm -Wall
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLES=$(SOURCES:.cc=)

#Uncomment the following to enable debugging
#CFLAGS+=-g -DDEBUG

all: $(EXECUTABLES)

tactics: tactics.o
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o: *.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f ${EXECUTABLES} ${OBJECTS} *.d
	-rm -f debug.txt

depend:
	makedepend -Y *.cc

.PHONY: all clean submit depend

# DO NOT DELETE
