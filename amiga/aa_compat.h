/* aa_compat.h: included at the end of the game's GENERAL.H (by
 * amiga/patches/0001), so every game file sees it. The game names files
 * the DOS way ("S0000000\CHDATA00", "AAMUSIC\TITLE.MUS"); these take the
 * backslashes out before the C library sees the name. And the endian calls
 * (amiga/aa_endian.h) the patched files use.
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#ifndef AA_COMPAT_H
#define AA_COMPAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "aa_endian.h"

FILE *aa_fopen(const char *path, const char *mode);
int aa_open(const char *path, int flags, ...);
int aa_remove(const char *path);
int aa_access(const char *path, int mode);
int aa_mkdir1(const char *path);
/* path with '\' as '/', in buf (at most size bytes) */
const char *aa_path(const char *path, char *buf, unsigned long size);

#define fopen(p, m) aa_fopen((p), (m))
#define open(...) aa_open(__VA_ARGS__)
#define remove(p) aa_remove(p)
#define access(p, m) aa_access((p), (m))

#endif
