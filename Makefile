LIBEVENT ?= /usr/local
LIBSIMPLEHTTP ?= /usr/local
LIBZMQ ?= /usr/local

CFLAGS = -I. -I$(LIBZMQ)/include -Wall -O2
LIBS = -L. -L$(LIBZMQ)/lib -lzmq
SRC = src

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

.PHONY: clean

clean:
	rm -f zlog-* $(SRC)/*.o
