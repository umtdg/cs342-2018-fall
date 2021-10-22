all: phistogram thistogram syn_phistogram
phistogram:
	gcc -Wall -Wextra -Werror -g -std=c11 -m64 helper.c phistogram.c -o phistogram
thistogram:
	gcc -Wall -Wextra -Werror -g -std=c11 -m64 -lpthread helper.c thistogram.c -o thistogram
syn_phistogram:
	gcc -Wall -Wextra -Werror -g -std=gnu11 -m64 -lpthread -lrt helper.c syn_phistogram.c -o syn_phistogram
clear:
	rm -rf phistogram
	rm -rf thistogram
	rm -rf syn_phistogram
	rm -rf hist*.txt
	rm -rf out*.txt
