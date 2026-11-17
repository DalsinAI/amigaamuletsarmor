/* aa_pre.h: read before every game file (-include): the C library's
 * headers first, so the game's GENERAL.H macros for DOS calls it lacked
 * on Unix (itoa, stricmp, ...) come after the library's own declarations
 * of them (libnix has itoa and stricmp) and do not rewrite them.
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#ifndef AA_PRE_H
#define AA_PRE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#endif
