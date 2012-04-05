/* Copyright 2012, Jeremy Grosser.  BSD Licensed.  Share and Enjoy! */
/* A DCPU-16 Video Emulator */

/* DCPU-16 CPU Implementation is Copyright 2012, Brian Swetland */

/* DCPU-16 Spec is Copyright 2012 Mojang */
/* http://0x10c.com/doc/dcpu-16.txt */

#ifndef DCPU_VIDEO_H
#define DCPU_VIDEO_H
#include <curses.h>

#include "dcpu.h"

#define SCREEN_COLS 32
#define SCREEN_ROWS 12

struct dcpu_video {
	WINDOW *window;
};

void dcpu_video_init(struct dcpu_video *v);
void dcpu_video_step(struct dcpu *d, struct dcpu_video *v);
void dcpu_video_stop(struct dcpu_video *v);
void dcpu_video_debug(const char *msg);

#endif
