CC=gcc
CFLAGS=-I. -lzmq
SRC=src

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: oplog-hub oplog-recv oplog-send

oplog-hub: $(SRC)/hub.o
	$(CC) -o $@ $^ $(CFLAGS)

oplog-recv: $(SRC)/recv.o
	$(CC) -o $@ $^ $(CFLAGS)

oplog-send: $(SRC)/send.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f oplog-* $(SRC)/*.o
