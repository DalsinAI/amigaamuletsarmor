/* aa_acgame.c: amiga/aa_plat.h on GFOS, through ACGame (acgame.library):
 * the game's 320 x 200 screen is ACGame's 8-bit screen (an RTG mode, drawn
 * by GFOpenGfx where it is installed, or AGA; no SDL and no GFOpenGPU),
 * its palette ACGame's, the keyboard ACGame's raw key table, the mouse the
 * screen's pointer (the game draws its own, so the system's is hidden),
 * and the first pad (GFOpenInput, or lowlevel.library's port 2) comes in
 * as keys: stick and d-pad walk and turn (W S A D), A attacks (Ctrl),
 * B opens and uses (E), X jumps (space), Start is Esc.
 *
 * The game's files are read from its own drawer (PROGDIR:), wherever it is
 * started from.
 *
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#include <exec/types.h>
#include <dos/dos.h>
#include <proto/dos.h>

#include <acgame/platform.h>

#include "aa_plat.h"

/* the game's 3D view and forms keep big tables on the stack */
unsigned long __stack = 256 * 1024;

static BPTR progdir, olddir;
static int have_olddir, screen;

void aa_plat_start(int argc, char **argv)
{
    (void)argc; (void)argv;
    progdir = Lock((CONST_STRPTR)"PROGDIR:", ACCESS_READ);
    if (progdir) {
        olddir = CurrentDir(progdir);
        have_olddir = 1;
    }
}

static void restore_dir(void)
{
    if (have_olddir) {
        CurrentDir(olddir);
        UnLock(progdir);
        have_olddir = 0;
    }
}

int aa_plat_open(void)
{
    acgame_video_config cfg;
    cfg.width = AA_W;
    cfg.height = AA_H;
    cfg.refresh_hz = 70;
    cfg.mode = ACGAME_VIDEO_INDEXED8;
    cfg.display = ACGAME_DISPLAY_AUTO;
    if (acgame_platform_init(&cfg) != 0) {
        printf("Amulets & Armor: ACGame could not open a 320 x 200 256-colour screen\n");
        restore_dir();
        return -1;
    }
    acgame_platform_show_pointer(0);
    screen = 1;
    return 0;
}

void aa_plat_close(void)
{
    if (screen) acgame_platform_shutdown();
    screen = 0;
    restore_dir();
}

static void dac_to_rgb(const unsigned char *pal6, acgame_rgb8 *out)
{
    int i;
    for (i = 0; i < 256; i++) {
        const unsigned char *c = pal6 + i * 3;
        unsigned r = c[0] & 63, g = c[1] & 63, b = c[2] & 63;
        out[i].r = (uint8_t)((r << 2) | (r >> 4));
        out[i].g = (uint8_t)((g << 2) | (g >> 4));
        out[i].b = (uint8_t)((b << 2) | (b >> 4));
    }
}

void aa_plat_show(const unsigned char *pixels, const unsigned char *pal6, int frame_changed)
{
    acgame_indexed_surface *s;
    int y;
    if (!screen) return;
    if (!frame_changed) {
        acgame_rgb8 pal[256];
        dac_to_rgb(pal6, pal);
        acgame_platform_set_palette(pal);
        return;
    }
    s = acgame_platform_begin_indexed_frame();
    if (!s) return;
    for (y = 0; y < AA_H && y < (int)s->height; y++)
        memcpy(s->pixels + (size_t)y * s->pitch, pixels + y * AA_W, AA_W);
    dac_to_rgb(pal6, s->palette);
    acgame_platform_present_indexed(s);
}

/* the pad as keys (Amiga raw codes) */
#define RAW_W 0x11
#define RAW_S 0x21
#define RAW_A 0x20
#define RAW_D 0x22
#define RAW_E 0x12
#define RAW_SPACE 0x40
#define RAW_ESC 0x45
#define RAW_CTRL 0x63
#define PAD_DEAD 12000

static void pad_keys(const acgame_pad_state *p, unsigned char *raw)
{
    uint32_t b = p->buttons;
    if (!p->connected) return;
    if ((b & ACGAME_PAD_BIT(ACGAME_PAD_DPAD_UP)) || p->axes[ACGAME_PAD_LEFTY] < -PAD_DEAD) raw[RAW_W] = 1;
    if ((b & ACGAME_PAD_BIT(ACGAME_PAD_DPAD_DOWN)) || p->axes[ACGAME_PAD_LEFTY] > PAD_DEAD) raw[RAW_S] = 1;
    if ((b & ACGAME_PAD_BIT(ACGAME_PAD_DPAD_LEFT)) || p->axes[ACGAME_PAD_LEFTX] < -PAD_DEAD) raw[RAW_A] = 1;
    if ((b & ACGAME_PAD_BIT(ACGAME_PAD_DPAD_RIGHT)) || p->axes[ACGAME_PAD_LEFTX] > PAD_DEAD) raw[RAW_D] = 1;
    if (b & ACGAME_PAD_BIT(ACGAME_PAD_A)) raw[RAW_CTRL] = 1;
    if (b & ACGAME_PAD_BIT(ACGAME_PAD_B)) raw[RAW_E] = 1;
    if (b & ACGAME_PAD_BIT(ACGAME_PAD_X)) raw[RAW_SPACE] = 1;
    if (b & ACGAME_PAD_BIT(ACGAME_PAD_START)) raw[RAW_ESC] = 1;
}

void aa_plat_input(struct aa_input *in)
{
    acgame_input_state st;
    const uint8_t *k;
    int x, y;
    memset(in, 0, sizeof(*in));
    if (!screen) return;
    acgame_platform_poll(&st);
    k = acgame_platform_rawkeys();
    if (k) memcpy(in->raw, k, sizeof(in->raw));
    pad_keys(&st.pad, in->raw);
    if (acgame_platform_pointer(&x, &y) == 0) {
        in->has_pointer = 1;
        in->x = x;
        in->y = y;
    }
    in->dx = st.mouse_dx;
    in->dy = st.mouse_dy;
    if (st.mouse_buttons & 1) in->buttons |= AA_MOUSE_LEFT;
    if (st.mouse_buttons & 2) in->buttons |= AA_MOUSE_RIGHT;
    if (st.mouse_buttons & 4) in->buttons |= AA_MOUSE_MIDDLE;
}

void aa_plat_audio(const short *samples, unsigned frames, unsigned rate)
{
    /* ACGame has no sound output yet (it answers -1): nothing is heard */
    acgame_platform_audio_s16stereo((const int16_t *)samples, frames, rate);
}

unsigned long aa_plat_ms(void)
{
    return acgame_platform_ticks_ms();
}

void aa_plat_idle(unsigned long ms)
{
    /* a whole 50 Hz tick or more to wait: give it back to the system */
    if (ms >= 20) Delay(ms / 20);
}
