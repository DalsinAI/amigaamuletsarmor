/* io.h: DOS's low-level file calls Amulets & Armor uses, on POSIX open/read
 * (the Amiga's C library has them too).
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team. */
#ifndef AA_IO_H
#define AA_IO_H
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef O_TEXT
#define O_TEXT 0
#endif
static inline long filelength(int fd)
{
    long here = lseek(fd, 0, SEEK_CUR), end;
    if (here < 0) return -1;
    end = lseek(fd, 0, SEEK_END);
    lseek(fd, here, SEEK_SET);
    return end;
}
#endif
