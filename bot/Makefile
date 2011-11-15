CC=g++
CFLAGS=-O3 -g -funroll-loops -Wall
#CFLAGS=-g -Wall
LDFLAGS=-O2 -lm
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=MyBot

#Uncomment the following to enable debugging
#CFLAGS+=-g -DDEBUG

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o: *.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f ${EXECUTABLE} ${OBJECTS} *.d
	-rm -f debug.txt

depend:
	makedepend -Y *.cc

submit:
	rm -f submission.zip
	zip submission.zip *.cc *.h

.PHONY: all clean submit depend

# DO NOT DELETE

Ant.o: Ant.h Location.h State.h Food.h Grid.h Square.h Timer.h Score.h
Bot.o: Bot.h State.h Ant.h Location.h Food.h Grid.h Square.h Timer.h Score.h
Grid.o: Grid.h Location.h
MyBot.o: Bot.h State.h Ant.h Location.h Food.h Grid.h Square.h Timer.h
State.o: Score.h Square.h Ant.h Location.h State.h Food.h Grid.h Timer.h
