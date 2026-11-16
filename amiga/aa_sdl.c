/* aa_sdl.c: the SDL 1.2 calls in amiga/include/sdl.h, on amiga/aa_plat.h.
 *
 * Keys: SDL_PumpEvents reads the machine (aa_plat_input) and turns the held
 * Amiga raw keys into SDL_GetKeyState's table. Mouse: the pointer and the
 * movement since the last SDL_GetRelativeMouseState (for mouse look), in the
 * 640 x 400 window the game's Unix code expects (it halves both). Time:
 * SDL_GetTicks counts from the first call; SDL_Delay gives the time back and
 * keeps the screen and input moving (amiga/aa_main.c's aa_main_wait).
 * Audio: the game's mixer (SOUND.C's IMixer) is called from the game's own
 * thread as time passes (aa_sdl_audio_pump, from every screen update and
 * wait) instead of from an audio thread, at 11025 Hz rather than the
 * 44100 the game asks for (the game takes the rate it is given), and its
 * output goes to aa_plat_audio. So sounds start, run and end on time even
 * where nothing can be heard (ACGame has no sound output yet).
 *
 * GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
 */
#include "aa_plat.h"
#include "sdl.h"

void aa_main_wait(unsigned long ms);

static Uint8 keystate[SDLK_LAST];
static struct aa_input last;
static int have_input, rel_dx, rel_dy, grabbed;
static unsigned long t0;
static int t0_set;

/* the latest input, read at most once per call of a pump */
const struct aa_input *aa_sdl_pump(void)
{
    int i;
    aa_plat_input(&last);
    have_input = 1;
    rel_dx += last.dx;
    rel_dy += last.dy;
    memset(keystate, 0, sizeof(keystate));
    for (i = 0; i < 128; i++) {
        if (last.raw[i]) {
            int k = aa_rawkey_to_sdlk((unsigned int)i);
            if (k > 0 && k < SDLK_LAST) keystate[k] = 1;
        }
    }
    return &last;
}

const struct aa_input *aa_sdl_last(void)
{
    if (!have_input) aa_sdl_pump();
    return &last;
}

void SDL_PumpEvents(void)
{
    aa_sdl_pump();
}

Uint8 *SDL_GetKeyState(int *numkeys)
{
    if (numkeys) *numkeys = SDLK_LAST;
    return keystate;
}

static Uint8 sdl_buttons(int b)
{
    Uint8 r = 0;
    if (b & AA_MOUSE_LEFT) r |= SDL_BUTTON_LMASK;
    if (b & AA_MOUSE_RIGHT) r |= SDL_BUTTON_RMASK;
    if (b & AA_MOUSE_MIDDLE) r |= SDL_BUTTON_MMASK;
    return r;
}

Uint8 SDL_GetMouseState(int *x, int *y)
{
    const struct aa_input *in = aa_sdl_last();
    if (x) *x = in->x * 2;
    if (y) *y = in->y * 2;
    return sdl_buttons(in->buttons);
}

Uint8 SDL_GetRelativeMouseState(int *x, int *y)
{
    const struct aa_input *in = aa_sdl_last();
    if (x) *x = rel_dx * 2;
    if (y) *y = rel_dy * 2;
    rel_dx = rel_dy = 0;
    return sdl_buttons(in->buttons);
}

void SDL_WarpMouse(Uint16 x, Uint16 y)
{
    (void)x; (void)y;   /* aa_main.c keeps the game's own pointer */
}

SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode)
{
    if (mode != SDL_GRAB_QUERY) grabbed = mode == SDL_GRAB_ON;
    return grabbed ? SDL_GRAB_ON : SDL_GRAB_OFF;
}

Uint32 SDL_GetTicks(void)
{
    unsigned long now = aa_plat_ms();
    if (!t0_set) { t0 = now; t0_set = 1; }
    return (Uint32)(now - t0);
}

void SDL_Delay(Uint32 ms)
{
    aa_main_wait(ms);
}

#define AA_AUDIO_RATE 11025
#define AA_AUDIO_FRAMES 256
#define AA_AUDIO_MAX_LAG_MS 250

static SDL_AudioSpec audio;
static int audio_open, audio_running, audio_busy;
static unsigned long audio_start, audio_done;
static Sint16 audio_buf[AA_AUDIO_FRAMES * 2];

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    if (!desired || !desired->callback || desired->channels != 2 || desired->format != AUDIO_S16SYS)
        return -1;
    audio = *desired;
    audio.freq = AA_AUDIO_RATE;
    audio.samples = AA_AUDIO_FRAMES;
    audio.silence = 0;
    audio.size = AA_AUDIO_FRAMES * 4;
    if (obtained) *obtained = audio;
    audio_open = 1;
    audio_running = 0;
    return 0;
}

void SDL_PauseAudio(int pause_on)
{
    if (!audio_open) return;
    if (!pause_on && !audio_running) {
        audio_start = aa_plat_ms();
        audio_done = 0;
    }
    audio_running = !pause_on;
}

void SDL_CloseAudio(void)
{
    audio_open = audio_running = 0;
}

/* Run the mixer for the time that has passed since it last ran. */
void aa_sdl_audio_pump(void)
{
    unsigned long due;
    if (!audio_running || audio_busy) return;
    audio_busy = 1;     /* a sound's done callback may lead back here */
    due = (aa_plat_ms() - audio_start) * (unsigned long)AA_AUDIO_RATE / 1000;
    if (due > audio_done + (unsigned long)AA_AUDIO_RATE * AA_AUDIO_MAX_LAG_MS / 1000) {
        /* too far behind (a long load): drop the time, not the sounds */
        audio_done = due - AA_AUDIO_FRAMES;
    }
    while (due >= audio_done + AA_AUDIO_FRAMES && audio_running) {
        memset(audio_buf, 0, sizeof(audio_buf));
        audio.callback(audio.userdata, (Uint8 *)audio_buf, (int)sizeof(audio_buf));
        aa_plat_audio(audio_buf, AA_AUDIO_FRAMES, AA_AUDIO_RATE);
        audio_done += AA_AUDIO_FRAMES;
    }
    audio_busy = 0;
}

const char *SDL_GetError(void)
{
    return "not on GFOS";
}
