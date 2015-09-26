CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

all: main.cpp
	g++ $(CFLAGS) $(LIBS) -o $@ $<
