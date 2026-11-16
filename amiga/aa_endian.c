/* aa_endian.c: the game's little-endian data in the 68040's order; see
 * amiga/aa_endian.h. The layouts are the game's own structures as DOS and
 * Windows wrote them (packed, 16-bit words and 32-bit longs):
 *   pictures       2 words (width, height), then the pixels; a compressed
 *                  (object, pointer) picture has one 4-byte entry per
 *                  column after them: a word offset, a start and an end
 *                  byte. Wall textures carry smaller copies after the
 *                  picture, whose headers the game never reads.
 *   OBJS/n/Info    T_objectType: a 22-byte header, its stances (5 words),
 *                  their frames (a byte, 4 words), the frames' pictures
 *                  (a word and 2 longs); stances may share frames.
 *   WEAPONS/x/Info T_overlayAnimation: a word, 32 names, 3 x 140 frames of
 *                  2 words and 2 bytes; *.MXY images are a stream of words
 *                  (0xFFFF ends, 0x8000|y a line, else x, a length and
 *                  that many pixels).
 *   SPELDESC/n     T_spellStruct (29 bytes); INVDESC/n the item
 *                  descriptions (71 bytes, or 79 for the 9-effect ones).
 *   MAP.ANI        4 word counts and 4 longs, then the wall (8 words),
 *                  side (30 bytes, 6 words), sector (20 bytes, 23 words)
 *                  and start (a byte, a pad byte, 2 words) states.
 *   L*.MAP         a WAD: its lumps are words (SIDEDEFS and SECTORS have
 *                  texture names among them; REJECT is bits).
 *   S*.SRP         a 64-byte header (2 words, 14 longs), the code, then
 *                  the event and place tables (words).
 * Text, palettes, fonts, colour tables and sounds are bytes and stay as
 * they are.
 *
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "aa_endian.h"

void aa_swap16(void *p, unsigned long n)
{
    unsigned char *b = (unsigned char *)p, t;
    while (n--) {
        t = b[0]; b[0] = b[1]; b[1] = t;
        b += 2;
    }
}

void aa_swap32(void *p, unsigned long n)
{
    unsigned char *b = (unsigned char *)p, t;
    while (n--) {
        t = b[0]; b[0] = b[3]; b[3] = t;
        t = b[1]; b[1] = b[2]; b[2] = t;
        b += 4;
    }
}

/* ---- names ---- */

static const char *leaf(const char *path)
{
    const char *s = path, *p;
    for (p = path; *p; p++)
        if (*p == '/' || *p == '\\' || *p == ':') s = p + 1;
    return s;
}

static int ieq(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == *b;
}

static int ends_with(const char *s, const char *tail)
{
    size_t n = strlen(s), m = strlen(tail);
    return n >= m && ieq(s + n - m, tail);
}

/* path begins with the directory dir ("OBJS" matches "OBJS/00530/Info") */
static int in_dir(const char *path, const char *dir)
{
    size_t m = strlen(dir);
    while (*path == '/' || *path == '\\') path++;
    if (strlen(path) <= m) return 0;
    {
        size_t i;
        for (i = 0; i < m; i++)
            if (tolower((unsigned char)path[i]) != tolower((unsigned char)dir[i])) return 0;
    }
    return path[m] == '/' || path[m] == '\\';
}

/* ---- swap once: shared parts of a block are swapped one time only ---- */

struct once {
    unsigned char *data;
    unsigned long size;
    unsigned char *done;    /* one flag per byte where a value starts */
};

static void once16(struct once *o, unsigned long off)
{
    if (off + 2 > o->size || o->done[off]) return;
    o->done[off] = 1;
    aa_swap16(o->data + off, 1);
}

static void once32(struct once *o, unsigned long off)
{
    if (off + 4 > o->size || o->done[off]) return;
    o->done[off] = 1;
    aa_swap32(o->data + off, 1);
}

/* ---- pictures ---- */

static int compressed_ok(const unsigned char *d, unsigned long size, unsigned x, unsigned y)
{
    unsigned long table = 4 + 4ul * y, i;
    if (x == 0 || y == 0 || table > size) return 0;
    for (i = 0; i < y; i++) {
        const unsigned char *e = d + 4 + 4 * i;
        unsigned long off = AA_RD16LE(e);
        unsigned s = e[2], t = e[3];
        if (s == 255) continue;
        if (t < s || off < table || off + (t - s + 1) > size) return 0;
    }
    return 1;
}

static void fix_picture(unsigned char *d, unsigned long size)
{
    unsigned x, y;
    if (size < 4) return;
    x = AA_RD16LE(d);
    y = AA_RD16LE(d + 2);
    if (4ul + (unsigned long)x * y != size && compressed_ok(d, size, x, y)) {
        unsigned long i;
        for (i = 0; i < y; i++)
            aa_swap16(d + 4 + 4 * i, 1);    /* the offset; start and end are bytes */
    } else if (4ul + (unsigned long)x * y > size) {
        return;                             /* not a picture: leave it */
    }
    aa_swap16(d, 2);
}

static void fix_mxy(unsigned char *d, unsigned long size)
{
    unsigned long p = 0;
    while (p + 2 <= size) {
        unsigned w = AA_RD16LE(d + p);
        aa_swap16(d + p, 1);
        p += 2;
        if (w == 0xFFFF) break;
        if (w & 0x8000) continue;
        if (p + 2 > size) break;
        w = AA_RD16LE(d + p);           /* the run's length */
        aa_swap16(d + p, 1);
        p += 2 + w;
    }
}

/* ---- object types ---- */

/* a word that may already be in the machine's order (shared and swapped) */
static unsigned long word_at(struct once *o, unsigned long off)
{
    const unsigned char *b = o->data + off;
    if (off + 2 > o->size) return 0;
    if (o->done[off]) return AA_BIG_ENDIAN ? ((unsigned)b[0] << 8 | b[1]) : AA_RD16LE(b);
    return AA_RD16LE(b);
}

static void fix_objtype(unsigned char *d, unsigned long size)
{
    struct once o;
    unsigned long stances, s;
    if (size < 22) return;
    o.data = d;
    o.size = size;
    o.done = (unsigned char *)calloc(1, size);
    if (!o.done) return;
    stances = AA_RD16LE(d);
    once16(&o, 0);      /* numStances */
    once32(&o, 2);      /* lockCount */
    once16(&o, 6);      /* radius */
    once16(&o, 8);      /* attributes */
    once16(&o, 10);     /* weight */
    once16(&o, 12);     /* value */
    once32(&o, 14);     /* script */
    once16(&o, 18);     /* health */
    once16(&o, 20);     /* objMoveAttr */
    for (s = 0; s < stances; s++) {
        unsigned long st = 22 + 10 * s, frames, fl, f;
        if (st + 10 > size) break;
        frames = word_at(&o, st);
        fl = word_at(&o, st + 8);
        once16(&o, st);         /* numFrames */
        once16(&o, st + 2);     /* speed */
        once16(&o, st + 4);     /* type */
        once16(&o, st + 6);     /* nextStance */
        once16(&o, st + 8);     /* offsetFrameList */
        for (f = 0; f < frames; f++) {
            unsigned long fr = fl + 9 * f, angles, pl, a;
            if (fr + 9 > size) break;
            angles = d[fr];
            if (angles != 1 && angles != 4 && angles != 8) angles = 1;
            pl = word_at(&o, fr + 1);
            once16(&o, fr + 1);     /* offsetPicList */
            once16(&o, fr + 3);     /* soundNum */
            once16(&o, fr + 5);     /* soundRadius */
            once16(&o, fr + 7);     /* objectAttributes */
            for (a = 0; a < angles; a++) {
                unsigned long pic = pl + 10 * a;
                once16(&o, pic);        /* number */
                once32(&o, pic + 2);    /* resource (0 on disk) */
                once32(&o, pic + 6);    /* p_pic (0 on disk) */
            }
        }
    }
    free(o.done);
}

/* ---- weapons, spells, items ---- */

static void fix_overlay_anim(unsigned char *d, unsigned long size)
{
    unsigned long f;
    if (size != 2 + 32 * 16 + 3 * 140 * 6) return;
    aa_swap16(d, 1);
    for (f = 0; f < 3 * 140; f++)
        aa_swap16(d + 2 + 32 * 16 + 6 * f, 2);
}

static void fix_spell(unsigned char *d, unsigned long size)
{
    if (size != 29) return;
    aa_swap16(d + 4, 8);    /* type .. costmod */
    aa_swap16(d + 27, 1);   /* sound */
}

static void fix_item(unsigned char *d, unsigned long size)
{
    if (size == 71) {           /* 8 effects */
        aa_swap16(d + 19, 8 * 3);
        aa_swap16(d + 68, 1);   /* useable */
    } else if (size == 79) {    /* 9 effects */
        aa_swap16(d + 21, 9 * 3);
        aa_swap16(d + 76, 1);
    }
}

/* ---- MAP.ANI ---- */

static void fix_map_anim(unsigned char *d, unsigned long size)
{
    unsigned long walls, sides, sectors, inits, p, i;
    if (size < 24) return;
    walls = AA_RD16LE(d);
    sides = AA_RD16LE(d + 2);
    sectors = AA_RD16LE(d + 4);
    inits = AA_RD16LE(d + 6);
    aa_swap16(d, 4);
    aa_swap32(d + 8, 4);
    p = 24;
    for (i = 0; i < walls && p + 16 <= size; i++, p += 16)
        aa_swap16(d + p, 8);
    for (i = 0; i < sides && p + 42 <= size; i++, p += 42)
        aa_swap16(d + p + 30, 6);
    for (i = 0; i < sectors && p + 66 <= size; i++, p += 66)
        aa_swap16(d + p + 20, 23);
    for (i = 0; i < inits && p + 6 <= size; i++, p += 6)
        aa_swap16(d + p + 2, 2);
}

/* ---- resources ---- */

static int is_bytes(const char *path)
{
    const char *l = leaf(path);
    static const char *const dirs[] = {
        "COLORIZE", "CREDESC", "MESSAGES", "OBJDESC", "OBJDESC2", "FONTS", "MAPDESC", 0
    };
    int i;
    for (i = 0; dirs[i]; i++)
        if (in_dir(path, dirs[i])) return 1;
    if (ends_with(l, ".TXT") || ends_with(l, ".PAL") || ends_with(l, ".DAT") ||
        ends_with(l, ".INI") || ends_with(l, ".I"))
        return 1;
    if (ieq(l, "BPalette") || !strncmp(l, "Font", 4) || !strncmp(l, "DP_", 3))
        return 1;
    return 0;
}

void aa_fix_resource(const char *file, const char *path, unsigned char *data, unsigned long size)
{
#if AA_BIG_ENDIAN
    const char *l = path ? leaf(path) : "";
    if (!data || !path) return;
    if (file) {
        const char *f = leaf(file);
        if (ieq(f, "sounds.res") || ieq(f, "sounds16.res")) return;
    } else {
        /* a loose file the resource system loaded */
        if (ieq(l, "MAP.ANI")) fix_map_anim(data, size);
        return;
    }
    if (size >= 4 && !memcmp(data, "Fon", 4)) return;
    if (is_bytes(path)) return;
    if (in_dir(path, "OBJS") && ieq(l, "Info")) { fix_objtype(data, size); return; }
    if (in_dir(path, "WEAPONS")) {
        if (ieq(l, "Info")) fix_overlay_anim(data, size);
        else if (ends_with(l, ".MXY")) fix_mxy(data, size);
        return;
    }
    if (in_dir(path, "SPELDESC")) { fix_spell(data, size); return; }
    if (in_dir(path, "INVDESC")) { fix_item(data, size); return; }
    fix_picture(data, size);
#else
    (void)file; (void)path; (void)data; (void)size;
    (void)fix_picture; (void)fix_mxy; (void)fix_objtype;
    (void)fix_overlay_anim; (void)fix_spell; (void)fix_item; (void)fix_map_anim; (void)is_bytes;
#endif
}

void aa_fix_res_header(void *header)
{
#if AA_BIG_ENDIAN
    /* uniqueID stays as bytes ("Res!"): the game compares it as bytes */
    aa_swap32((unsigned char *)header + 4, 2);
    aa_swap16((unsigned char *)header + 12, 1);
#else
    (void)header;
#endif
}

void aa_fix_res_index(void *index, unsigned long entries)
{
#if AA_BIG_ENDIAN
    unsigned char *e = (unsigned char *)index;
    unsigned long i;
    for (i = 0; i < entries; i++, e += 39) {
        aa_swap32(e + 18, 2);   /* fileOffset, size */
        aa_swap16(e + 26, 1);   /* lockCount */
        aa_swap32(e + 29, 1);   /* p_data */
        aa_swap16(e + 33, 1);   /* resourceFile */
        aa_swap32(e + 35, 1);   /* ownerDir */
    }
#else
    (void)index; (void)entries;
#endif
}

/* ---- the map ---- */

void aa_fix_wad_header(void *header)
{
#if AA_BIG_ENDIAN
    aa_swap32((unsigned char *)header + 4, 2);
#else
    (void)header;
#endif
}

void aa_fix_wad_entry(void *entry)
{
#if AA_BIG_ENDIAN
    aa_swap32(entry, 2);
#else
    (void)entry;
#endif
}

void aa_fix_map_lump(const char *name, void *data, unsigned long size)
{
#if AA_BIG_ENDIAN
    unsigned char *d = (unsigned char *)data;
    unsigned long i;
    if (!strncmp(name, "SIDEDEFS", 8)) {
        for (i = 0; i + 30 <= size; i += 30) {
            aa_swap16(d + i, 2);        /* tmXoffset, tmYoffset */
            aa_swap16(d + i + 28, 1);   /* sector */
        }
    } else if (!strncmp(name, "SECTORS", 7)) {
        for (i = 0; i + 26 <= size; i += 26) {
            aa_swap16(d + i, 2);        /* floorHt, ceilingHt */
            aa_swap16(d + i + 20, 3);   /* light, type, trigger */
        }
    } else if (!strncmp(name, "REJECT", 6)) {
        /* bits */
    } else {
        /* THINGS, LINEDEFS, VERTEXES, SEGS, SSECTORS, NODES, BLOCKMAP */
        aa_swap16(d, size / 2);
    }
#else
    (void)name; (void)data; (void)size;
#endif
}

/* ---- scripts ---- */

void aa_fix_script(unsigned char *d, unsigned long size)
{
#if AA_BIG_ENDIAN
    unsigned long events, places, code, tables;
    if (size < 64) return;
    events = AA_RD16LE(d);
    places = AA_RD16LE(d + 2);
    code = AA_RD32LE(d + 4);
    aa_swap16(d, 2);
    aa_swap32(d + 4, 14);
    tables = 64 + code;
    if (tables + 2 * (events + places) <= size)
        aa_swap16(d + tables, events + places);
#else
    (void)d; (void)size;
#endif
}
