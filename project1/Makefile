CC = gcc
CVERSION = gnu11
CCFLAGS = -Wall -Wextra -Werror -g -m64 -std=$(CVERSION)
LDFLAGS = -lpthread -lrt
FILES = helper.c

all: phistogram thistogram syn_phistogram
phistogram:
	mkdir -p bin
	$(CC) $(CCFLAGS) $(LDFLAGS) $(FILES) phistogram.c -o bin/phistogram
thistogram:
	mkdir -p bin
	$(CC) $(CCFLAGS) $(LDFLAGS) $(FILES) thistogram.c -o bin/thistogram
syn_phistogram:
	mkdir -p bin
	$(CC) $(CCFLAGS) $(LDFLAGS) $(FILES) syn_phistogram.c -o bin/syn_phistogram
clear:
	rm -rf hist*.txt
	rm -rf out*.txt
	rm -rf bin
