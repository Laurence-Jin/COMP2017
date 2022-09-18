CC=gcc
CFLAGS=-fsanitize=address -Wvla -Wall -Werror -g -std=gnu11 -lm

# fill in all your make rules
#echo "objdump_x2017"

vm_x2017: vm_x2017.c
	$(CC) $(CFLAGS) $^ -o $@ 

objdump_x2017: objdump_x2017.c
	$(CC) $(CFLAGS) $^ -o $@ 
	
tests:
	echo"tests"

run_tests:
	bash test.sh

clean:
	rm vm_x2017
	rm objdump_x2017

