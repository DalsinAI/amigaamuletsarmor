/* SDL_net.h: no network play on GFOS (the IPX link is not built), so
 * TESTME.C's SDLNet_Init is never reached; declared for the compiler only.
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team. */
#ifndef AA_SDL_NET_H
#define AA_SDL_NET_H
static inline int SDLNet_Init(void) { return -1; }
static inline const char *SDLNet_GetError(void) { return "no network play"; }
#endif
