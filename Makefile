
dcpu: dcpu.c
	gcc -g -Wall -lcurses -o dcpu video.c dcpu.c

clean:
	rm -f dcpu
