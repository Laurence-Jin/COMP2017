CC=gcc
CFLAGS=-fsanitize=address -Wvla -Wall -Werror -g -std=gnu11

timetable: timetable.c
	$(CC) $(CFLAGS) $^ -o $@

test: timetable
	bash test.sh

clean:
	rm timetable
