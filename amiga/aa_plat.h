/* aa_plat.h: what the port's SDL subset (amiga/aa_sdl.c) and the game's
 * screen calls (amiga/aa_main.c) need from the machine. Two backends:
 * amiga/aa_acgame.c (GFOS: ACGame's 8-bit screen, keys, mouse and pad) and
 * tests/host/aa_host.c (a PC: a frame in memory, scripted keys and mouse,
 * virtual time).
 *
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#ifndef AA_PLAT_H
#define AA_PLAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AA_W 320
#define AA_H 200

/* mouse buttons in aa_input.buttons */
#define AA_MOUSE_LEFT 1
#define AA_MOUSE_RIGHT 2
#define AA_MOUSE_MIDDLE 4

struct aa_input {
    unsigned char raw[128];     /* every key by its Amiga raw key code: 1 held */
    int has_pointer;            /* x and y below are where the pointer is */
    int x, y;                   /* in frame pixels; may be outside the frame */
    int dx, dy;                 /* how far the mouse moved since the last read */
    int buttons;                /* AA_MOUSE_* */
};

/* Before the game starts: the game's drawer becomes the current directory. */
void aa_plat_start(int argc, char **argv);
/* The 320 x 200 256-colour screen. 0 when it is open. */
int aa_plat_open(void);
void aa_plat_close(void);
/* Show the frame: pixels (320 x 200) with pal6 (256 VGA DAC entries, 6 bits
 * each). frame_changed is 0 when only the palette changed. */
void aa_plat_show(const unsigned char *pixels, const unsigned char *pal6, int frame_changed);
/* The keys, the pointer and the mouse since the last call. The pad (if one)
 * comes in as keys: see amiga/aa_acgame.c. */
void aa_plat_input(struct aa_input *in);
/* Milliseconds from any fixed point. */
unsigned long aa_plat_ms(void);
/* Nothing to do for ms milliseconds: give the time back if it can. */
void aa_plat_idle(unsigned long ms);

/* The game's sound, mixed: frames of 16-bit stereo at rate. */
void aa_plat_audio(const short *samples, unsigned frames, unsigned rate);

/* amiga/aa_keys.c: Amiga raw key code to the SDL 1.2 key number the game's
 * KEYBOARD.C table is indexed by, 0 for a key it has none for. */
int aa_rawkey_to_sdlk(unsigned int rawkey);

/* amiga/aa_main.c: the frame and its palette as last shown (for tests) */
const unsigned char *aa_shown_frame(void);
#endif
