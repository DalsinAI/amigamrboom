/*
 * Mr.Boom's music on AmigaOS: the ProTracker MODs through acsdl12's own
 * MOD player (Mix_LoadMUS), the FastTracker XMs through upstream's ibxm
 * (libretro/ibxm) in acsdl12's music hook.
 * The AmiCradle Team. MIT, like Mr.Boom.
 */
#ifndef MUSIC_AMIGA_H
#define MUSIC_AMIGA_H
#ifdef __cplusplus
extern "C" {
#endif

#define AMIGA_DATA_DIR "PROGDIR:data/"

/* Load tune `index` from AMIGA_DATA_DIR`name`. 0 on success. */
int amiga_music_load(int index, const char *name);
/* Play tune `index` looped at `volume` (0 - MIX_MAX_VOLUME), after
 * stopping whatever plays. A tune that did not load is silence. */
void amiga_music_play(int index, int volume);
void amiga_music_stop(void);
void amiga_music_free(void);

#ifdef __cplusplus
}
#endif
#endif
