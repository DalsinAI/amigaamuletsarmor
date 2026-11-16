/* aa_main.c: what Build/Windows/VC2013/AA/main.c and direct.cpp give the
 * game on Windows and Unix, done on amiga/aa_plat.h instead of SDL video:
 *   main         opens the 320 x 200 screen and runs game_main (TESTME.C);
 *   WindowsUpdate  the game's "show this frame" (GRAPHICS.C, with the mouse
 *                pointer drawn in): shown at most every AA_SHOW_MS, the
 *                frame kept if it comes sooner and shown by the next update
 *                or wait; then the mouse and the keys are read;
 *   SleepMS      waits (aa_main_wait);
 *   DirectMouse*, OutsideMouseDriver*  the game's pointer: it follows the
 *                system pointer whenever that moves, and stays where the
 *                game puts it (after mouse look) until it does.
 *
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#include "aa_plat.h"
/* the Amiga compiler has its own __interrupt, __far and __near; the
 * game's OPTIONS.H makes them nothing, as for Unix */
#undef __interrupt
#undef __far
#undef __near
#include "general.h"
#include "graphics.h"
#include "keyboard.h"
#include "mousemod.h"

#define AA_SHOW_MS 20

extern T_void game_main(T_word16 argc, char *argv[]);
const struct aa_input *aa_sdl_pump(void);
void aa_sdl_audio_pump(void);
extern T_void KeyboardUpdate(E_Boolean updateBuffers);

static unsigned char shown[AA_W * AA_H], shown_pal[768];
static unsigned char pending[AA_W * AA_H], pending_pal[768];
static int have_shown, have_pending;
static unsigned long last_show;

static T_buttonClick mouse_button;
static T_word16 mouse_x, mouse_y;
static int abs_x = -1, abs_y = -1;

const unsigned char *aa_shown_frame(void) { return shown; }

static void show_now(const unsigned char *pixels, const unsigned char *pal)
{
    int frame_changed = !have_shown || memcmp(shown, pixels, sizeof(shown)) != 0;
    int pal_changed = !have_shown || memcmp(shown_pal, pal, sizeof(shown_pal)) != 0;
    if (frame_changed) memcpy(shown, pixels, sizeof(shown));
    if (pal_changed) memcpy(shown_pal, pal, sizeof(shown_pal));
    if (frame_changed || pal_changed)
        aa_plat_show(shown, shown_pal, frame_changed);
    have_shown = 1;
    have_pending = 0;
    last_show = aa_plat_ms();
}

static void flush_pending(void)
{
    if (have_pending && aa_plat_ms() - last_show >= AA_SHOW_MS)
        show_now(pending, pending_pal);
}

void aa_main_wait(unsigned long ms)
{
    unsigned long start = aa_plat_ms(), now;
    for (;;) {
        flush_pending();
        aa_sdl_audio_pump();
        now = aa_plat_ms();
        if (now - start >= ms) break;
        aa_plat_idle(ms - (now - start));
    }
}

void SleepMS(T_word32 ms)
{
    aa_main_wait(ms);
}

T_void DirectMouseSet(T_word16 x, T_word16 y)
{
    if (MouseIsRelativeMode()) return;  /* mouse look: the pointer stays */
    if (x > 319) x = 319;
    if (y > 199) y = 199;
    mouse_x = x;
    mouse_y = y;
}

void OutsideMouseDriverGet(T_word16 *x, T_word16 *y)
{
    *x = mouse_x;
    *y = mouse_y;
}

void OutsideMouseDriverSet(T_word16 x, T_word16 y)
{
    DirectMouseSet(x, y);
}

T_void DirectMouseSetButton(T_buttonClick click)
{
    mouse_button = click;
}

T_buttonClick DirectMouseGetButton(T_void)
{
    return mouse_button;
}

static void read_mouse(void)
{
    const struct aa_input *in = aa_sdl_pump();
    T_buttonClick b = 0;
    if (in->has_pointer && (in->x != abs_x || in->y != abs_y)) {
        abs_x = in->x;
        abs_y = in->y;
        DirectMouseSet((T_word16)(in->x < 0 ? 0 : in->x), (T_word16)(in->y < 0 ? 0 : in->y));
    }
    if (in->buttons & AA_MOUSE_LEFT) b |= MOUSE_BUTTON_LEFT;
    if (in->buttons & AA_MOUSE_RIGHT) b |= MOUSE_BUTTON_RIGHT;
    if (in->buttons & AA_MOUSE_MIDDLE) b |= MOUSE_BUTTON_MIDDLE;
    DirectMouseSetButton(b);
}

void WindowsUpdate(char *p_screen, unsigned char *palette)
{
    if (aa_plat_ms() - last_show >= AA_SHOW_MS || !have_shown) {
        show_now((const unsigned char *)p_screen, palette);
    } else {
        memcpy(pending, p_screen, sizeof(pending));
        memcpy(pending_pal, palette, sizeof(pending_pal));
        have_pending = 1;
    }
    aa_sdl_audio_pump();
    read_mouse();
    KeyboardUpdate(TRUE);
}

static void close_screen(void)
{
    aa_plat_close();
}

int main(int argc, char **argv)
{
    static unsigned char *frame;
    aa_plat_start(argc, argv);
    /* the game draws into 320 x 240 (it writes a little past line 199) */
    frame = (unsigned char *)calloc(1, AA_W * 240);
    if (!frame) {
        printf("Amulets & Armor: not enough memory for the screen\n");
        return 20;
    }
    GRAPHICS_ACTUAL_SCREEN = (T_screen)frame;
    if (aa_plat_open() != 0) return 20;
    atexit(close_screen);
    game_main((T_word16)argc, argv);
    return 0;
}
