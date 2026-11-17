/* aa_endian.h: Amulets & Armor's data files are little-endian (DOS and
 * Windows wrote them by casting structures to bytes); the 68040 is
 * big-endian. Every file and resource the game reads as numbers is put
 * into the machine's order once, where it is read (amiga/patches/0001
 * calls these), so the game's own code is unchanged:
 *   AA_RD16LE/AA_RD32LE   read a value in the file's order, wherever it is
 *                         (the script interpreter reads its code this way)
 *   aa_fix_resource       a block from a .RES file or a loose file the
 *                         resource system loads, by its path (pictures,
 *                         object types, weapon animations, spells, items,
 *                         MAP.ANI)
 *   aa_fix_*              the map lumps, the maths tables, script headers
 * On a little-endian machine (the PC host tests) they all do nothing.
 *
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#ifndef AA_ENDIAN_H
#define AA_ENDIAN_H

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define AA_BIG_ENDIAN 1
#elif defined(__m68k__) || defined(__mc68000__) || defined(mc68000)
#define AA_BIG_ENDIAN 1
#else
#define AA_BIG_ENDIAN 0
#endif

#define AA_RD16LE(p) ((unsigned short)(((const unsigned char *)(p))[0] | \
                       (((const unsigned char *)(p))[1] << 8)))
#define AA_RD32LE(p) ((unsigned long)(((const unsigned char *)(p))[0] | \
                       (((const unsigned char *)(p))[1] << 8) | \
                       ((unsigned long)((const unsigned char *)(p))[2] << 16) | \
                       ((unsigned long)((const unsigned char *)(p))[3] << 24)))

/* in place, n values each */
void aa_swap16(void *p, unsigned long n);
void aa_swap32(void *p, unsigned long n);

/* A resource block just read: file is the .RES file's name (NULL for a
 * loose file), path the block's name inside it ("OBJS/00530/Info"), or
 * the loose file's name. */
void aa_fix_resource(const char *file, const char *path, unsigned char *data, unsigned long size);

/* The map's WAD lumps by name ("SEGS", "THINGS", ...), and its header and
 * directory entries. */
void aa_fix_wad_header(void *header);
void aa_fix_wad_entry(void *entry);
void aa_fix_map_lump(const char *name, void *data, unsigned long size);
/* A script file (S*.SRP) as loaded: the header, the event and place
 * tables (the code is read with AA_RD16LE/AA_RD32LE). */
void aa_fix_script(unsigned char *data, unsigned long size);
/* A resource file's header and index entries (39 bytes each). */
void aa_fix_res_header(void *header);
void aa_fix_res_index(void *index, unsigned long entries);

#endif
