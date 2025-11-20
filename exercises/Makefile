CC = gcc
CFLAGS = -O2 -march=native -std=gnu99 -Wall -Wextra
LDFLAGS = -Wl,-T,layout.ld
CORE = 4 # CPU core to bind the programs to

# Always rebuild programs
.PHONY: all clean 3_exploit_simple_spectre 2_exploit_pin exploit_spectre 1_exploit_vote 0_exploit_memcmp

all: 3_exploit_simple_spectre 2_exploit_pin exploit_spectre 1_exploit_vote 0_exploit_memcmp

#3_exploit_spectre: 3_exploit_spectre.c 3_victim_spectre.c
#	$(CC) $(CFLAGS) -o 3_exploit_spectre 3_exploit_spectre.c 3_victim_spectre.c $(LDFLAGS)

3_exploit_simple_spectre: 3_exploit_simple_spectre.c 3_victim_simple_spectre.c
	$(CC) $(CFLAGS) -o 3_exploit_simple_spectre 3_exploit_simple_spectre.c 3_victim_simple_spectre.c

2_exploit_pin: 2_exploit_pin.c 2_victim_pin.c
	$(CC) $(CFLAGS) -o 2_exploit_pin 2_exploit_pin.c 2_victim_pin.c $(LDFLAGS)

1_exploit_vote: 1_exploit_vote.c 1_victim_vote.c
	$(CC) $(CFLAGS) -o 1_exploit_vote 1_exploit_vote.c 1_victim_vote.c

0_exploit_memcmp: 0_exploit_memcmp.c 0_victim_memcmp.c
	$(CC) $(CFLAGS) -o 0_exploit_memcmp 0_exploit_memcmp.c 0_victim_memcmp.c

run_%: %
	taskset -c $(CORE) ./$<

clean:
	rm -f 0_exploit_memcmp 1_exploit_vote 2_exploit_pin 3_exploit_simple_spectre

.PHONY: all clean
