
all: dcpu a16

dcpu: dcpu.c
	gcc -Wall -lcurses -o dcpu video.c dcpu.c

a16: a16.c
	gcc -Wall -o a16 a16.c

clean:
	rm -f dcpu a16
