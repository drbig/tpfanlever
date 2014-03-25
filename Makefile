CFLAGS=-Wall -O2

all: tpfanlever

debug: CFLAGS+=-DDEBUG
debug: all

clean:
	rm -f tpfanlever
