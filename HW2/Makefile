CC=gcc
CFLAGS=-Wall -Wextra -O2
LDFLAGS=-pthread -lrt

all: wait signal

wait: wait.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

signal: signal.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f wait signal
