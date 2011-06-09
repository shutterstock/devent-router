LIBEVENT ?= /usr/local
LIBSIMPLEHTTP ?= /usr/local
LIBZMQ ?= /usr/local

PREFIX ?= /usr/local

CFLAGS = -I. -I$(LIBZMQ)/include -Wall -O2 $(CFLAGS_EXTRA)
LIBS = -L. -L$(LIBZMQ)/lib -lzmq $(LIBS_EXTRA)
SRC ?= src

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: zlog-hub zlog-recv zlog-send

zlog-http: $(SRC)/http.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) -I$(LIBEVENT)/include \
		-I$(LIBSIMPLEHTTP)/include -L$(LIBEVENT)/lib -L$(LIBSIMPLEHTTP)/lib -levent \
		-lsimplehttp -lm

zlog-hub: $(SRC)/hub.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

zlog-recv: $(SRC)/recv.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

zlog-send: $(SRC)/send.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean install

install:
	/usr/bin/install -p -D -m 755 zlog-hub $(PREFIX)/bin/zlog-hub
	/usr/bin/install -p -D -m 755 zlog-recv $(PREFIX)/bin/zlog-recv
	/usr/bin/install -p -D -m 755 zlog-send $(PREFIX)/bin/zlog-send

clean:
	rm -f zlog-* $(SRC)/*.o
