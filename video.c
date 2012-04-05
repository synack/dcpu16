/* Copyright 2012, Jeremy Grosser.  BSD Licensed.  Share and Enjoy! */
/* A DCPU-16 Video Emulator */

/* DCPU-16 CPU Implementation is Copyright 2012, Brian Swetland */

/* DCPU-16 Spec is Copyright 2012 Mojang */
/* http://0x10c.com/doc/dcpu-16.txt */

#include <curses.h>
#include "video.h"

void dcpu_video_init(struct dcpu_video *v) {
	memset(v, 0, sizeof(v));

	v->window = initscr();
	cbreak();
	noecho();

	clear();
	mvprintw(0, 0, "DCPU VIDEO INIT");
	refresh();
}

void dcpu_video_step(struct dcpu *d, struct dcpu_video *v) {
	chtype c;
	int i;

	clear();
	for(i = 0; i < (SCREEN_ROWS * SCREEN_COLS); i++) {
		c = (chtype)(d->m[0x8000+i] & 0xFF);
		if(c == 0)
			c = '.';

		mvaddch(
			(i % SCREEN_COLS) + 1,
			(i / SCREEN_COLS) + 1,
			c | A_NORMAL);
	}

	mvprintw(14, 0, "PC=%04x SP=%04x OV=%04x SKIP=%04x",
		d->pc, d->sp, d->ov, d->skip);
	mvprintw(15, 0, "A=%04x  B=%04x  C=%04x",
		d->r[0], d->r[1], d->r[2]);
	mvprintw(16, 0, "X=%04x  Y=%04x  Z=%04x I=%04x J=%04x",
		d->r[3], d->r[4], d->r[5], d->r[6], d->r[7]);

	move(0, 0);
	refresh();
}

void dcpu_video_stop(struct dcpu_video *v) {
	endwin();
}
