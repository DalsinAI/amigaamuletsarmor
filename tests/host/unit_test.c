/* unit_test.c: the port's own pieces on a PC, one check at a time. Each
 * file block is built as the game's files have it (little-endian), put
 * through amiga/aa_endian.c, then read in the machine's own order: on a PC
 * nothing changes, on a 68k (tests/host/test_aa.sh under qemu-m68k) every
 * number is swapped and every byte that is not a number stays:
 *  - aa_swap16/aa_swap32, AA_RD16LE/AA_RD32LE;
 *  - resource file header and index entries;
 *  - pictures (plain and with a column table), weapon .MXY runs, spells,
 *    scripts, map lumps (SIDEDEFS keep their texture names), and the
 *    blocks that are bytes (fonts, messages, sounds.res) left alone;
 *  - aa_keys.c: Amiga raw keys to the SDL 1.2 keys the game reads;
 *  - aa_files.c: the game's DOS paths (\) as / paths.
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#include "aa_plat.h"
#include "aa_endian.h"
#include "sdl.h"

const char *aa_path(const char *path, char *buf, unsigned long size);

static int fails, checks;
#define CHECK(c) do { checks++; if (!(c)) { fails++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

static void le16(unsigned char *p, unsigned v) { p[0] = v & 255; p[1] = v >> 8; }
static void le32(unsigned char *p, unsigned long v) { le16(p, v & 0xffff); le16(p + 2, v >> 16); }
static unsigned short n16(const unsigned char *p) { unsigned short v; memcpy(&v, p, 2); return v; }
static unsigned int n32(const unsigned char *p) { unsigned int v; memcpy(&v, p, 4); return v; }

static void test_swap(void)
{
    unsigned char b[8] = { 0x12, 0x34, 0x56, 0x78, 1, 2, 3, 4 };
    CHECK(AA_RD16LE(b) == 0x3412);
    CHECK(AA_RD32LE(b) == 0x78563412ul);
    aa_swap16(b, 2);
    CHECK(b[0] == 0x34 && b[1] == 0x12 && b[2] == 0x78 && b[3] == 0x56 && b[4] == 1);
    aa_swap32(b + 4, 1);
    CHECK(b[4] == 4 && b[5] == 3 && b[6] == 2 && b[7] == 1);
}

static void test_res(void)
{
    unsigned char h[14], e[2 * 39];
    int i;
    memcpy(h, "Res!", 4); le32(h + 4, 1000); le32(h + 8, 70000); le16(h + 12, 7);
    aa_fix_res_header(h);
    CHECK(!memcmp(h, "Res!", 4));
    CHECK(n32(h + 4) == 1000 && n32(h + 8) == 70000 && n16(h + 12) == 7);
    memset(e, 0, sizeof(e));
    for (i = 0; i < 2; i++) {
        unsigned char *x = e + 39 * i;
        memcpy(x, "PICS", 5);
        le32(x + 18, 123456 + i); le32(x + 22, 999 + i); le16(x + 26, 2);
        le32(x + 29, 0); le16(x + 33, 3); le32(x + 35, 0xffffffff);
    }
    aa_fix_res_index(e, 2);
    for (i = 0; i < 2; i++) {
        unsigned char *x = e + 39 * i;
        CHECK(!memcmp(x, "PICS", 5));
        CHECK(n32(x + 18) == 123456u + i && n32(x + 22) == 999u + i);
        CHECK(n16(x + 26) == 2 && n16(x + 33) == 3 && n32(x + 35) == 0xffffffffu);
    }
}

static void test_pictures(void)
{
    /* plain: 3 x 2 */
    unsigned char p[4 + 6] = { 0, 0, 0, 0, 1, 2, 3, 4, 5, 6 };
    /* with a column table: 2 x 2, column 0 rows 0..1 at 12, column 1 empty */
    unsigned char c[4 + 8 + 2];
    le16(p, 3); le16(p + 2, 2);
    aa_fix_resource("ui.res", "UI/PANEL", p, sizeof(p));
    CHECK(n16(p) == 3 && n16(p + 2) == 2);
    CHECK(p[4] == 1 && p[9] == 6);

    le16(c, 2); le16(c + 2, 2);
    le16(c + 4, 12); c[6] = 0; c[7] = 1;
    le16(c + 8, 0); c[10] = 255; c[11] = 0;
    c[12] = 7; c[13] = 8;
    aa_fix_resource("ui.res", "UI/ROUND", c, sizeof(c));
    CHECK(n16(c) == 2 && n16(c + 2) == 2);
    CHECK(n16(c + 4) == 12 && c[6] == 0 && c[7] == 1);
    CHECK(c[10] == 255 && c[12] == 7 && c[13] == 8);

    /* bytes: a font, a message, the sounds, a palette */
    {
        unsigned char f[8] = { 'F', 'o', 'n', 0, 1, 2, 3, 4 };
        unsigned char m[6] = { 2, 0, 1, 0, 'h', 'i' };
        unsigned char s[6] = { 2, 0, 1, 0, 9, 9 };
        aa_fix_resource("ui.res", "FONTS/SMALL", f, sizeof(f));
        CHECK(f[4] == 1 && f[5] == 2);
        aa_fix_resource("ui.res", "MESSAGES/M1", m, sizeof(m));
        CHECK(m[0] == 2 && m[1] == 0);
        aa_fix_resource("sounds.res", "SND/1", s, sizeof(s));
        CHECK(s[0] == 2 && s[1] == 0);
    }
}

static void test_weapons_spells(void)
{
    /* .MXY: line 5, then x 3 with 2 pixels, then the end */
    unsigned char x[12];
    unsigned char sp[29];
    int i;
    le16(x, 0x8005); le16(x + 2, 3); le16(x + 4, 2); x[6] = 0x41; x[7] = 0x42; le16(x + 8, 0xffff);
    x[10] = 0x55; x[11] = 0x66;
    aa_fix_resource("ui.res", "WEAPONS/SWORD/F1.MXY", x, sizeof(x));
    CHECK(n16(x) == 0x8005 && n16(x + 2) == 3 && n16(x + 4) == 2);
    CHECK(x[6] == 0x41 && x[7] == 0x42 && n16(x + 8) == 0xffff);
    CHECK(x[10] == 0x55 && x[11] == 0x66);

    memset(sp, 0, sizeof(sp));
    memcpy(sp, "1234", 4);
    for (i = 0; i < 8; i++) le16(sp + 4 + 2 * i, 300 + i);
    le16(sp + 27, 4321);
    aa_fix_resource("ui.res", "SPELDESC/SPL0001", sp, sizeof(sp));
    CHECK(!memcmp(sp, "1234", 4));
    for (i = 0; i < 8; i++) CHECK(n16(sp + 4 + 2 * i) == 300 + i);
    CHECK(n16(sp + 27) == 4321);
}

static void test_scripts(void)
{
    /* 1 event, 1 place, 4 bytes of code */
    unsigned char s[64 + 4 + 4];
    int i;
    memset(s, 0, sizeof(s));
    le16(s, 1); le16(s + 2, 1); le32(s + 4, 4);
    for (i = 1; i < 14; i++) le32(s + 4 + 4 * i, 1000 + i);
    s[64] = 0x11; s[65] = 0x22; s[66] = 0x33; s[67] = 0x44;
    le16(s + 68, 7); le16(s + 70, 9);
    aa_fix_script(s, sizeof(s));
    CHECK(n16(s) == 1 && n16(s + 2) == 1 && n32(s + 4) == 4);
    CHECK(n32(s + 8) == 1001 && n32(s + 56) == 1013);
    CHECK(AA_RD16LE(s + 64) == 0x2211);           /* the code is read as it is */
    CHECK(n16(s + 68) == 7 && n16(s + 70) == 9);
}

static void test_map(void)
{
    unsigned char hd[12], de[16], sd[30], vx[8];
    memcpy(hd, "PWAD", 4); le32(hd + 4, 11); le32(hd + 8, 54321);
    aa_fix_wad_header(hd);
    CHECK(!memcmp(hd, "PWAD", 4) && n32(hd + 4) == 11 && n32(hd + 8) == 54321);
    le32(de, 12); le32(de + 4, 300); memcpy(de + 8, "SIDEDEFS", 8);
    aa_fix_wad_entry(de);
    CHECK(n32(de) == 12 && n32(de + 4) == 300 && !memcmp(de + 8, "SIDEDEFS", 8));

    memset(sd, 0, sizeof(sd));
    le16(sd, 0xfff0); le16(sd + 2, 16);
    memcpy(sd + 4, "WALL1\0\0\0", 8); memcpy(sd + 12, "-\0\0\0\0\0\0\0", 8); memcpy(sd + 20, "DOOR2\0\0\0", 8);
    le16(sd + 28, 42);
    aa_fix_map_lump("SIDEDEFS", sd, sizeof(sd));
    CHECK(n16(sd) == 0xfff0 && n16(sd + 2) == 16 && n16(sd + 28) == 42);
    CHECK(!memcmp(sd + 4, "WALL1", 5) && !memcmp(sd + 20, "DOOR2", 5));

    le16(vx, 100); le16(vx + 2, 0xff9c); le16(vx + 4, 7); le16(vx + 6, 8);
    aa_fix_map_lump("VERTEXES", vx, sizeof(vx));
    CHECK(n16(vx) == 100 && n16(vx + 2) == 0xff9c && n16(vx + 6) == 8);
}

static void test_keys(void)
{
    CHECK(aa_rawkey_to_sdlk(0x11) == SDLK_w);
    CHECK(aa_rawkey_to_sdlk(0x20) == SDLK_a);
    CHECK(aa_rawkey_to_sdlk(0x40) == SDLK_SPACE);
    CHECK(aa_rawkey_to_sdlk(0x45) == SDLK_ESCAPE);
    CHECK(aa_rawkey_to_sdlk(0x46) == SDLK_DELETE);
    CHECK(aa_rawkey_to_sdlk(0x5f) == SDLK_INSERT);      /* Help: look down */
    CHECK(aa_rawkey_to_sdlk(0x63) == SDLK_LCTRL);
    CHECK(aa_rawkey_to_sdlk(0x60) == SDLK_LSHIFT);
    CHECK(aa_rawkey_to_sdlk(0x66) == SDLK_LALT);        /* left Amiga */
    CHECK(aa_rawkey_to_sdlk(0x43) == SDLK_KP_ENTER);    /* cast */
    CHECK(aa_rawkey_to_sdlk(0x1d) == SDLK_KP1);         /* rune 1 */
    CHECK(aa_rawkey_to_sdlk(0x4c) == SDLK_UP);
    CHECK(aa_rawkey_to_sdlk(0x80) == 0 && aa_rawkey_to_sdlk(0x1000) == 0);
}

static void test_paths(void)
{
    char b[64];
    CHECK(!strcmp(aa_path("S0000000\\CHDATA03", b, sizeof(b)), "S0000000/CHDATA03"));
    CHECK(!strcmp(aa_path("MAPDESC\\L10.TXT", b, sizeof(b)), "MAPDESC/L10.TXT"));
    CHECK(!strcmp(aa_path("config.ini", b, sizeof(b)), "config.ini"));
    CHECK(strlen(aa_path("a\\b\\c\\d\\e\\f\\g\\h\\i\\j", b, 8)) == 7);
}

int main(void)
{
    test_swap();
    test_res();
    test_pictures();
    test_weapons_spells();
    test_scripts();
    test_map();
    test_keys();
    test_paths();
    printf("unit: %d of %d checks passed (%s-endian)\n", checks - fails, checks, AA_BIG_ENDIAN ? "big" : "little");
    return fails != 0;
}
