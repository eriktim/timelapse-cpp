CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`

all: main.cpp
	g++ $(CFLAGS) $(LIBS) -o $@ $<

line: line.cpp
	g++ $(CFLAGS) $(LIBS) -o $@ $<

main3: main3.cpp
	g++ $(CFLAGS) $(LIBS) -o $@ $<
