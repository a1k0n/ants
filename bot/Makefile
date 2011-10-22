CC=g++
CFLAGS=-O3 -g -funroll-loops -c
LDFLAGS=-O2 -lm
SOURCES=Bot.cc MyBot.cc State.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=MyBot

#Uncomment the following to enable debugging
#CFLAGS+=-g -DDEBUG

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o: *.h
	$(CC) $(CFLAGS) $< -o $@

clean: 
	-rm -f ${EXECUTABLE} ${OBJECTS} *.d
	-rm -f debug.txt

submit:
	rm -f submission.zip
	zip submission.zip *.cc *.h

.PHONY: all clean submit

