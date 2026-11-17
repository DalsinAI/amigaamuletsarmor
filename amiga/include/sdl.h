/* SDL.h: the few SDL 1.2 calls Amulets & Armor's game code makes
 * (KEYBOARD.C, MOUSEMOD.C, TICKER.C, SOUND.C and OPTIONS.H's delay), done
 * by amiga/aa_sdl.c on top of amiga/aa_plat.h: on GFOS that is ACGame
 * (acgame.library), on a PC the host tests' backend. It is not SDL: no
 * windows, surfaces or events, and audio does not open (ACGame has no
 * sound yet), so the game runs silent.
 * The key numbers are SDL 1.2's, which KEYBOARD.C's table is indexed by.
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#ifndef AA_SDL_H
#define AA_SDL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t Uint8;
typedef int8_t Sint8;
typedef uint16_t Uint16;
typedef int16_t Sint16;
typedef uint32_t Uint32;
typedef int32_t Sint32;

typedef enum {
    SDLK_UNKNOWN = 0,
    SDLK_FIRST = 0,
    SDLK_BACKSPACE = 8,
    SDLK_TAB = 9,
    SDLK_CLEAR = 12,
    SDLK_RETURN = 13,
    SDLK_PAUSE = 19,
    SDLK_ESCAPE = 27,
    SDLK_SPACE = 32,
    SDLK_EXCLAIM = 33,
    SDLK_QUOTEDBL = 34,
    SDLK_HASH = 35,
    SDLK_DOLLAR = 36,
    SDLK_AMPERSAND = 38,
    SDLK_QUOTE = 39,
    SDLK_LEFTPAREN = 40,
    SDLK_RIGHTPAREN = 41,
    SDLK_ASTERISK = 42,
    SDLK_PLUS = 43,
    SDLK_COMMA = 44,
    SDLK_MINUS = 45,
    SDLK_PERIOD = 46,
    SDLK_SLASH = 47,
    SDLK_0 = 48, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9,
    SDLK_COLON = 58,
    SDLK_SEMICOLON = 59,
    SDLK_LESS = 60,
    SDLK_EQUALS = 61,
    SDLK_GREATER = 62,
    SDLK_QUESTION = 63,
    SDLK_AT = 64,
    SDLK_LEFTBRACKET = 91,
    SDLK_BACKSLASH = 92,
    SDLK_RIGHTBRACKET = 93,
    SDLK_CARET = 94,
    SDLK_UNDERSCORE = 95,
    SDLK_BACKQUOTE = 96,
    SDLK_a = 97, SDLK_b, SDLK_c, SDLK_d, SDLK_e, SDLK_f, SDLK_g, SDLK_h, SDLK_i,
    SDLK_j, SDLK_k, SDLK_l, SDLK_m, SDLK_n, SDLK_o, SDLK_p, SDLK_q, SDLK_r,
    SDLK_s, SDLK_t, SDLK_u, SDLK_v, SDLK_w, SDLK_x, SDLK_y, SDLK_z,
    SDLK_DELETE = 127,
    SDLK_WORLD_0 = 160,
    SDLK_WORLD_95 = 255,
    SDLK_KP0 = 256, SDLK_KP1, SDLK_KP2, SDLK_KP3, SDLK_KP4,
    SDLK_KP5, SDLK_KP6, SDLK_KP7, SDLK_KP8, SDLK_KP9,
    SDLK_KP_PERIOD = 266,
    SDLK_KP_DIVIDE = 267,
    SDLK_KP_MULTIPLY = 268,
    SDLK_KP_MINUS = 269,
    SDLK_KP_PLUS = 270,
    SDLK_KP_ENTER = 271,
    SDLK_KP_EQUALS = 272,
    SDLK_UP = 273,
    SDLK_DOWN = 274,
    SDLK_RIGHT = 275,
    SDLK_LEFT = 276,
    SDLK_INSERT = 277,
    SDLK_HOME = 278,
    SDLK_END = 279,
    SDLK_PAGEUP = 280,
    SDLK_PAGEDOWN = 281,
    SDLK_F1 = 282, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5, SDLK_F6, SDLK_F7, SDLK_F8,
    SDLK_F9, SDLK_F10, SDLK_F11, SDLK_F12, SDLK_F13, SDLK_F14, SDLK_F15,
    SDLK_NUMLOCK = 300,
    SDLK_CAPSLOCK = 301,
    SDLK_SCROLLOCK = 302,
    SDLK_RSHIFT = 303,
    SDLK_LSHIFT = 304,
    SDLK_RCTRL = 305,
    SDLK_LCTRL = 306,
    SDLK_RALT = 307,
    SDLK_LALT = 308,
    SDLK_RMETA = 309,
    SDLK_LMETA = 310,
    SDLK_LSUPER = 311,
    SDLK_RSUPER = 312,
    SDLK_MODE = 313,
    SDLK_COMPOSE = 314,
    SDLK_HELP = 315,
    SDLK_PRINT = 316,
    SDLK_SYSREQ = 317,
    SDLK_BREAK = 318,
    SDLK_MENU = 319,
    SDLK_POWER = 320,
    SDLK_EURO = 321,
    SDLK_UNDO = 322,
    SDLK_LAST
} SDLKey;

#define SDL_BUTTON(x) (1 << ((x) - 1))
#define SDL_BUTTON_LEFT 1
#define SDL_BUTTON_MIDDLE 2
#define SDL_BUTTON_RIGHT 3
#define SDL_BUTTON_LMASK SDL_BUTTON(SDL_BUTTON_LEFT)
#define SDL_BUTTON_MMASK SDL_BUTTON(SDL_BUTTON_MIDDLE)
#define SDL_BUTTON_RMASK SDL_BUTTON(SDL_BUTTON_RIGHT)

typedef enum { SDL_GRAB_QUERY = -1, SDL_GRAB_OFF = 0, SDL_GRAB_ON = 1 } SDL_GrabMode;

#define AUDIO_S16LSB 0x8010
#define AUDIO_S16MSB 0x9010
#if defined(__BIG_ENDIAN__) || defined(__m68k__) || defined(__mc68000__) || \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define AUDIO_S16SYS AUDIO_S16MSB
#else
#define AUDIO_S16SYS AUDIO_S16LSB
#endif

typedef struct SDL_AudioSpec {
    int freq;
    Uint16 format;
    Uint8 channels;
    Uint8 silence;
    Uint16 samples;
    Uint16 padding;
    Uint32 size;
    void (*callback)(void *userdata, Uint8 *stream, int len);
    void *userdata;
} SDL_AudioSpec;

Uint8 *SDL_GetKeyState(int *numkeys);
void SDL_PumpEvents(void);
Uint8 SDL_GetMouseState(int *x, int *y);
Uint8 SDL_GetRelativeMouseState(int *x, int *y);
void SDL_WarpMouse(Uint16 x, Uint16 y);
SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode);
Uint32 SDL_GetTicks(void);
void SDL_Delay(Uint32 ms);
int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
void SDL_PauseAudio(int pause_on);
void SDL_CloseAudio(void);
const char *SDL_GetError(void);

#ifdef __cplusplus
}
#endif

#endif
