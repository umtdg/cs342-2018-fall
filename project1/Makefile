CC = gcc
CVERSION = gnu11
CCFLAGS = -Wall -Wextra -Werror -g -m64 -std=$(CVERSION)
LDFLAGS = -lpthread -lrt
FILES = helper.c

all: phistogram thistogram syn_phistogram
phistogram:
	$(CC) $(CCFLAGS) $(LDFLAGS) $(FILES) phistogram.c -o phistogram
thistogram:
	$(CC) $(CCFLAGS) $(LDFLAGS) $(FILES) thistogram.c -o thistogram
syn_phistogram:
	$(CC) $(CCFLAGS) $(LDFLAGS) $(FILES) syn_phistogram.c -o syn_phistogram
clear:
	rm -rf hist*.txt
	rm -rf out*.txt
	rm -rf phistogram
	rm -rf thistogram
	rm -rf syn_phistogram
