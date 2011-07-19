LIBEVENT ?= /usr/local
LIBSIMPLEHTTP ?= /usr/local
LIBZMQ ?= /usr/local

PREFIX ?= /usr/local

CFLAGS = -I. -I$(LIBZMQ)/include -Wall -O2 $(CFLAGS_EXTRA)
LIBS = -L. -L$(LIBZMQ)/lib -lzmq $(LIBS_EXTRA)
SRC ?= src

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: dr-hub dr-recv dr-send

dr-http: $(SRC)/http.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) -I$(LIBEVENT)/include \
		-I$(LIBSIMPLEHTTP)/include -L$(LIBEVENT)/lib -L$(LIBSIMPLEHTTP)/lib -levent \
		-lsimplehttp -lm

dr-hub: $(SRC)/hub.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

dr-recv: $(SRC)/recv.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

dr-send: $(SRC)/send.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean install

install:
	/usr/bin/install -p -D -m 755 dr-hub $(PREFIX)/bin/dr-hub
	/usr/bin/install -p -D -m 755 dr-recv $(PREFIX)/bin/dr-recv
	/usr/bin/install -p -D -m 755 dr-send $(PREFIX)/bin/dr-send

clean:
	rm -f dr-* $(SRC)/*.o
