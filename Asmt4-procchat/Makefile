CC=gcc
CFLAGS=-Wall -Werror -Wvla -std=gnu11 -fsanitize=address
PFLAGS=-fprofile-arcs -ftest-coverage
DFLAGS=-g
HEADERS=server.h
SRC=server.c

procchat: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC) -o $@

test: procchat
	$(CC) $(CFLAGS) $(PFLAGS) test_client.c -o test_client.out && sh ./test.sh

clean:
	rm -f procchat
