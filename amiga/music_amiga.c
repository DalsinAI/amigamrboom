/*
 * Mr.Boom's music on AmigaOS. Upstream's SDL builds play its seven MODs
 * and four XMs with SDL_mixer (libmodplug); acsdl12's Mix_LoadMUS plays
 * MODs only, so the MODs go there and the XMs are played by ibxm, the
 * replayer upstream's libretro core already carries (libretro/ibxm), in
 * acsdl12's music hook. ibxm reads module data a byte at a time, so it
 * needs nothing for the big-endian machine.
 * The AmiCradle Team. MIT, like Mr.Boom.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "ibxm.h"
#include "music_amiga.h"

#define MAX_TUNES 16

typedef struct {
    Mix_Music *mod;          /* a MOD, for acsdl12's player */
    struct module *xm;       /* an XM, for ibxm */
    char *xm_file;
} Tune;

static Tune tunes[MAX_TUNES];
static struct replay *replay;
static int *mix_buf;
static int mix_offset, mix_frames;
static int rate;
static volatile int hook_volume;

/* In acsdl12's mixer process. The audio is open mono (sdl.cpp), so the
 * stream is 16-bit mono in the machine's byte order; ibxm makes stereo
 * pairs, which are folded down here. */
static void xm_hook(void *udata, Uint8 *stream, int len)
{
    Sint16 *out = (Sint16 *)stream;
    int frames = len / 2;
    int vol = hook_volume;

    (void)udata;
    if (!replay || !vol)
        return;
    while (frames > 0) {
        int n, i;
        const int *in;

        if (mix_frames == 0) {
            mix_offset = 0;
            mix_frames = replay_get_audio(replay, mix_buf, 0);
            if (mix_frames <= 0)
                return;
        }
        n = mix_frames < frames ? mix_frames : frames;
        in = &mix_buf[mix_offset * 2];
        for (i = 0; i < n; i++) {
            int v = ((in[i * 2] + in[i * 2 + 1]) / 2 * vol) >> 7;
            out[i] = (Sint16)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
        }
        out += n;
        mix_offset += n;
        mix_frames -= n;
        frames -= n;
    }
}

static char *load_file(const char *path, int *size)
{
    FILE *f = fopen(path, "rb");
    char *bytes = NULL;
    long length;

    *size = 0;
    if (!f)
        return NULL;
    if (fseek(f, 0, SEEK_END) == 0 && (length = ftell(f)) > 0 &&
        length < 4L * 1024 * 1024 && fseek(f, 0, SEEK_SET) == 0) {
        bytes = (char *)malloc((size_t)length);
        if (bytes && fread(bytes, 1, (size_t)length, f) != (size_t)length) {
            free(bytes);
            bytes = NULL;
        }
        if (bytes)
            *size = (int)length;
    }
    fclose(f);
    return bytes;
}

static int is_xm(const char *name)
{
    size_t n = strlen(name);
    return n > 3 && (name[n - 3] == '.') &&
           (name[n - 2] == 'X' || name[n - 2] == 'x') &&
           (name[n - 1] == 'M' || name[n - 1] == 'm');
}

int amiga_music_load(int index, const char *name)
{
    char path[256];
    Tune *t;

    if (index < 0 || index >= MAX_TUNES)
        return -1;
    t = &tunes[index];
    snprintf(path, sizeof(path), "%s%s", AMIGA_DATA_DIR, name);
    if (!is_xm(name)) {
        t->mod = Mix_LoadMUS(path);
        return t->mod ? 0 : -1;
    }
    if (!rate) {
        int frequency, channels;
        Uint16 format;

        if (!Mix_QuerySpec(&frequency, &format, &channels))
            return -1;
        rate = frequency;
        mix_buf = (int *)calloc((size_t)calculate_mix_buf_len(rate), sizeof(int));
        if (!mix_buf)
            return -1;
    }
    {
        struct data d;
        char message[64] = "";

        t->xm_file = load_file(path, &d.length);
        if (!t->xm_file)
            return -1;
        d.buffer = t->xm_file;
        t->xm = module_load(&d, message);
        if (!t->xm || message[0]) {
            if (t->xm)
                dispose_module(t->xm);
            t->xm = NULL;
            free(t->xm_file);
            t->xm_file = NULL;
            return -1;
        }
    }
    return 0;
}

void amiga_music_stop(void)
{
    Mix_HaltMusic();
    Mix_HookMusic(NULL, NULL);          /* the mixer is out of the hook now */
    if (replay)
        dispose_replay(replay);
    replay = NULL;
    mix_offset = mix_frames = 0;
}

void amiga_music_play(int index, int volume)
{
    Tune *t;

    amiga_music_stop();
    if (index < 0 || index >= MAX_TUNES)
        return;
    t = &tunes[index];
    if (t->mod) {
        Mix_VolumeMusic(volume);
        Mix_PlayMusic(t->mod, -1);
    } else if (t->xm && mix_buf) {
        replay = new_replay(t->xm, rate, 0);
        if (replay) {
            hook_volume = volume;
            Mix_HookMusic(xm_hook, NULL);
        }
    }
}

void amiga_music_free(void)
{
    int i;

    amiga_music_stop();
    for (i = 0; i < MAX_TUNES; i++) {
        if (tunes[i].mod)
            Mix_FreeMusic(tunes[i].mod);
        if (tunes[i].xm)
            dispose_module(tunes[i].xm);
        free(tunes[i].xm_file);
        memset(&tunes[i], 0, sizeof(tunes[i]));
    }
    free(mix_buf);
    mix_buf = NULL;
    rate = 0;
}
