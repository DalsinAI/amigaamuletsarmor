/* conio.h: the DOS console calls Amulets & Armor names. The game reads its
 * keys through KEYBOARD.C, never the console, so these answer "no key".
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team. */
#ifndef AA_CONIO_H
#define AA_CONIO_H
#include <stdio.h>
static inline int kbhit(void) { return 0; }
static inline int getch(void) { return 0; }
#endif
