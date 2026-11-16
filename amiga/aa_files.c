/* aa_files.c: the file calls amiga/aa_compat.h sends the game's names
 * through: a DOS name's backslashes become slashes, then the C library's
 * own call (on GFOS AmigaDOS, which ignores case; on a PC the host tests
 * add a case-blind lookup).
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define AA_PATH_MAX 256

const char *aa_path(const char *path, char *buf, unsigned long size)
{
    unsigned long i;
    if (!path) return path;
    for (i = 0; path[i] && i + 1 < size; i++)
        buf[i] = path[i] == '\\' ? '/' : path[i];
    buf[i] = 0;
    return buf;
}

FILE *aa_fopen(const char *path, const char *mode)
{
    char buf[AA_PATH_MAX];
    return fopen(aa_path(path, buf, sizeof(buf)), mode);
}

int aa_open(const char *path, int flags, ...)
{
    char buf[AA_PATH_MAX];
    int mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, int);
        va_end(ap);
    }
    return open(aa_path(path, buf, sizeof(buf)), flags, mode);
}

int aa_remove(const char *path)
{
    char buf[AA_PATH_MAX];
    return remove(aa_path(path, buf, sizeof(buf)));
}

int aa_access(const char *path, int mode)
{
    char buf[AA_PATH_MAX];
    return access(aa_path(path, buf, sizeof(buf)), mode);
}

int aa_mkdir1(const char *path)
{
    char buf[AA_PATH_MAX];
    return mkdir(aa_path(path, buf, sizeof(buf)), 0777);
}
