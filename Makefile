all:

pdp: pdp.c pdp.h
	  g++ -Wall -Wnoarrowing -Wno-write-strings pdp.c -o pdp
