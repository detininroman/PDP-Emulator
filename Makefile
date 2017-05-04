all:

pdp: pdp.c pdp.h
	  g++ -Wall -Wno-write-strings  pdp.c -o pdp
