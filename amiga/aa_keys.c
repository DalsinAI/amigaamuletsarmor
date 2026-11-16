/* aa_keys.c: the Amiga keyboard, by raw key code, to the SDL 1.2 key numbers
 * the game's KEYBOARD.C turns into its PC scan codes. Keys the Amiga
 * keyboard lacks: Help is Insert (look down; Del is look up), and a PC-style
 * keyboard's Insert, Home, End, Page Up/Down, F11 and F12 keep their own
 * raw codes (0x47, 0x70, 0x71, 0x48, 0x49, 0x4b, 0x6f). Both Amiga keys
 * are Alt as well (the game's sidestep and menu chords).
 *
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#include "aa_plat.h"
#include "sdl.h"

static const unsigned short raw_to_sdlk[128] = {
    /* 0x00 */ SDLK_BACKQUOTE, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7,
    /* 0x08 */ SDLK_8, SDLK_9, SDLK_0, SDLK_MINUS, SDLK_EQUALS, SDLK_BACKSLASH, 0, SDLK_KP0,
    /* 0x10 */ SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_y, SDLK_u, SDLK_i,
    /* 0x18 */ SDLK_o, SDLK_p, SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET, 0, SDLK_KP1, SDLK_KP2, SDLK_KP3,
    /* 0x20 */ SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_j, SDLK_k,
    /* 0x28 */ SDLK_l, SDLK_SEMICOLON, SDLK_QUOTE, 0, 0, SDLK_KP4, SDLK_KP5, SDLK_KP6,
    /* 0x30 */ 0, SDLK_z, SDLK_x, SDLK_c, SDLK_v, SDLK_b, SDLK_n, SDLK_m,
    /* 0x38 */ SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, 0, SDLK_KP_PERIOD, SDLK_KP7, SDLK_KP8, SDLK_KP9,
    /* 0x40 */ SDLK_SPACE, SDLK_BACKSPACE, SDLK_TAB, SDLK_KP_ENTER, SDLK_RETURN, SDLK_ESCAPE, SDLK_DELETE, SDLK_INSERT,
    /* 0x48 */ SDLK_PAGEUP, SDLK_PAGEDOWN, SDLK_KP_MINUS, SDLK_F11, SDLK_UP, SDLK_DOWN, SDLK_RIGHT, SDLK_LEFT,
    /* 0x50 */ SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5, SDLK_F6, SDLK_F7, SDLK_F8,
    /* 0x58 */ SDLK_F9, SDLK_F10, 0, 0, SDLK_KP_DIVIDE, SDLK_KP_MULTIPLY, SDLK_KP_PLUS, SDLK_INSERT,
    /* 0x60 */ SDLK_LSHIFT, SDLK_RSHIFT, SDLK_CAPSLOCK, SDLK_LCTRL, SDLK_LALT, SDLK_RALT, SDLK_LALT, SDLK_RALT,
    /* 0x68 */ 0, 0, 0, 0, 0, 0, SDLK_PAUSE, SDLK_F12,
    /* 0x70 */ SDLK_HOME, SDLK_END, 0, 0, 0, 0, 0, 0,
    /* 0x78 */ 0, 0, 0, 0, 0, 0, 0, 0,
};

int aa_rawkey_to_sdlk(unsigned int rawkey)
{
    return rawkey < 128 ? raw_to_sdlk[rawkey] : 0;
}
