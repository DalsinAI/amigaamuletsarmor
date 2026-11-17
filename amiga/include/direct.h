/* direct.h: DOS's one-argument mkdir (amiga/aa_files.c).
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team. */
#ifndef AA_DIRECT_H
#define AA_DIRECT_H
#include <sys/stat.h>
#include <sys/types.h>
int aa_mkdir1(const char *path);
#define _mkdir(p) aa_mkdir1(p)
#ifndef mkdir
#define mkdir(p) aa_mkdir1(p)
#endif
#endif
