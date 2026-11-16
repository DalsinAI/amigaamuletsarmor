/* aa_host.c: amiga/aa_plat.h on a PC, for the host tests: the screen is a
 * frame in memory, time is virtual (a wait takes no time at all, a busy loop
 * moves the clock on by 1 ms every 64 reads), and the keys and the mouse
 * come from scripts.
 *
 *   AA_DATA   the game's drawer (it becomes the current directory)
 *   AA_KEYS   "MS:RAW:1,MS:RAW:0,...": Amiga raw key RAW goes down (1) or
 *             up (0) at virtual time MS
 *   AA_MOUSE  "MS:X:Y:B,...": from virtual time MS the pointer is at X,Y
 *             with buttons B (1 left, 2 right)
 *   AA_SHOTS  "MS,MS,...": the frame shown at that time is written to
 *             AA_OUT/shot-MS.ppm
 *   AA_END    the virtual time the run ends (exit 0), after writing
 *             AA_OUT/summary.txt: how many frames were shown, the colours
 *             in the last one, and whether the screen was ever opened
 *
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#include "aa_plat.h"
#include <unistd.h>

static unsigned long vt, reads;
static unsigned char frame[AA_W * AA_H], pal[768];
static int opened, shows, closed;
static unsigned long audio_frames, audio_loud;
static const char *out_dir = ".";

static unsigned long env_ul(const char *name, unsigned long dflt)
{
    const char *v = getenv(name);
    return v && *v ? strtoul(v, 0, 10) : dflt;
}

static void write_ppm(const char *path)
{
    FILE *f = fopen(path, "wb");
    int i;
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", AA_W, AA_H);
    for (i = 0; i < AA_W * AA_H; i++) {
        const unsigned char *c = pal + frame[i] * 3;
        fputc(((c[0] & 63) << 2) | ((c[0] & 63) >> 4), f);
        fputc(((c[1] & 63) << 2) | ((c[1] & 63) >> 4), f);
        fputc(((c[2] & 63) << 2) | ((c[2] & 63) >> 4), f);
    }
    fclose(f);
}

static int colours_used(void)
{
    static unsigned char seen[256];
    int i, n = 0;
    memset(seen, 0, sizeof(seen));
    for (i = 0; i < AA_W * AA_H; i++) {
        if (!seen[frame[i]]) { seen[frame[i]] = 1; n++; }
    }
    return n;
}

static void finish(void)
{
    char path[512];
    FILE *f;
    snprintf(path, sizeof(path), "%s/summary.txt", out_dir);
    f = fopen(path, "w");
    if (f) {
        fprintf(f, "opened %d\nshows %d\ncolours %d\nvt %lu\naudio %lu\nloud %lu\n",
                opened, shows, colours_used(), vt, audio_frames, audio_loud);
        fclose(f);
    }
    snprintf(path, sizeof(path), "%s/last.ppm", out_dir);
    write_ppm(path);
}

static void check_time(void)
{
    static unsigned long end;
    const char *shots = getenv("AA_SHOTS");
    if (!end) end = env_ul("AA_END", 60000);
    if (shots) {
        static unsigned long done_upto;
        const char *p = shots;
        while (*p) {
            unsigned long t = strtoul(p, (char **)&p, 10);
            if (t > done_upto && t <= vt) {
                char path[512];
                snprintf(path, sizeof(path), "%s/shot-%lu.ppm", out_dir, t);
                write_ppm(path);
                done_upto = t;
            }
            if (*p == ',') p++; else break;
        }
    }
    if (vt >= end) {
        finish();
        fflush(stdout);
        _exit(0);
    }
}

void aa_plat_start(int argc, char **argv)
{
    const char *d = getenv("AA_DATA");
    (void)argc; (void)argv;
    if (getenv("AA_OUT")) out_dir = getenv("AA_OUT");
    if (d && chdir(d) != 0) { perror(d); exit(2); }
}

int aa_plat_open(void) { opened++; return 0; }
void aa_plat_close(void) { closed++; }

void aa_plat_show(const unsigned char *pixels, const unsigned char *pal6, int frame_changed)
{
    if (frame_changed) memcpy(frame, pixels, sizeof(frame));
    memcpy(pal, pal6, sizeof(pal));
    shows++;
}

void aa_plat_audio(const short *samples, unsigned frames, unsigned rate)
{
    unsigned i;
    (void)rate;
    audio_frames += frames;
    for (i = 0; i < frames * 2; i++)
        if (samples[i] > 256 || samples[i] < -256) { audio_loud++; break; }
}

void aa_plat_input(struct aa_input *in)
{
    const char *p;
    static int last_x, last_y;
    memset(in, 0, sizeof(*in));
    for (p = getenv("AA_KEYS"); p && *p; ) {
        unsigned long t = strtoul(p, (char **)&p, 10);
        long k = -1, v = 0;
        if (*p == ':') k = strtol(p + 1, (char **)&p, 10);
        if (*p == ':') v = strtol(p + 1, (char **)&p, 10);
        if (t <= vt && k >= 0 && k < 128) in->raw[k] = (unsigned char)v;
        while (*p && *p != ',') p++;
        if (*p == ',') p++;
    }
    in->has_pointer = 1;
    in->x = 160;
    in->y = 100;
    for (p = getenv("AA_MOUSE"); p && *p; ) {
        unsigned long t = strtoul(p, (char **)&p, 10);
        long x = 0, y = 0, b = 0;
        if (*p == ':') x = strtol(p + 1, (char **)&p, 10);
        if (*p == ':') y = strtol(p + 1, (char **)&p, 10);
        if (*p == ':') b = strtol(p + 1, (char **)&p, 10);
        if (t <= vt) { in->x = (int)x; in->y = (int)y; in->buttons = (int)b; }
        while (*p && *p != ',') p++;
        if (*p == ',') p++;
    }
    in->dx = in->x - last_x;
    in->dy = in->y - last_y;
    last_x = in->x;
    last_y = in->y;
}

unsigned long aa_plat_ms(void)
{
    if ((++reads & 63) == 0) vt++;
    check_time();
    return vt;
}

void aa_plat_idle(unsigned long ms)
{
    vt += ms ? ms : 1;
    check_time();
}
