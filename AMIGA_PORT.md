Mr.Boom for AmigaOS 3.x
=======================

Mr.Boom (Remdy Software, 1997; asm2c conversion and libretro/SDL ports by
Frranck/Javanaise), an 8-player Bomberman clone, for AmigaOS 3.x on a
68040 with FPU, on acsdl12, our SDL 1.2 for game ports (ACGame's screen,
AHI sound). Upstream: https://github.com/Javanaise/mrboom-libretro at
40ac320 (master, 5.5), MIT.

Running
-------

Double-click MrBoom, or run it from a Shell; it finds its data drawer
through PROGDIR:, so it runs from anywhere. It opens a 320x200 screen in
256 colours (AGA or RTG).

Needs: 68040 + FPU, AmigaOS 3.x, AHI for sound (without AHI it plays
silent), and about 8 MB of free memory (the program is 6 MB: the DOS
game's 4 MB heap with all its pictures is compiled in).

Controls
--------

Players join in the menu by pressing a button; Space adds a bot.

* Player 1 (keyboard): W A S D move, Ctrl drop bomb, Left Alt remote
  trigger, Left Shift jump (kangaroo).
* Player 2 (keyboard): cursor keys move, keypad 0 drop bomb, keypad . or
  Right Alt remote trigger, Right Shift jump; keypad Enter is Start and
  jump.
* Pad (the first pad acsdl12 sees: a CD32 pad, a joystick, or a USB pad
  through gfopeninput): d-pad or left stick move, A/red drop bomb,
  B/blue remote trigger, X/green jump, Start start/pause, Back add a bot.
  Start and Back together leave the match.
* Return: start. P: pause. Esc: leave the match, or quit in the menu.
  Right Amiga + Q quits.

The Amiga keys are not used for players: Ctrl with both Amiga keys
resets the machine.

What is different on the Amiga
------------------------------

* The front end is upstream's SDL 1.2 one (sdl/sdl.cpp, written for the
  Atari Falcon), not the SDL 2 one: the game draws a 320x200 frame in 256
  colours with a VGA palette, which is what acsdl12's screen is, so each
  frame is copied straight onto the screen and the palette set as it is;
  the SDL 2 front end's 32-bit texture and xBRZ scaling are not needed.
* The game runs at 60 frames a second, paced with SDL_GetTicks; when the
  machine falls behind, up to two frames in a row are run without being
  shown. m.slowcpu is set, as on the Falcon (one sprite drawn in C).
* Big-endian: the core is built with -DMSB_FIRST, upstream's own
  big-endian mode (its libretro core uses it on PowerPC): register halves
  and STOS/LODS bytes are laid out for it, and the game swaps the
  little-endian longs of its demo records itself (m.isbigendian). The
  game's data is the heap in mrboom_data.c (as in the libretro core), so
  the DOS file reads are never used. Sounds are PCM WAVs read by acsdl12
  (which reads the little-endian header and samples), and ibxm reads
  modules a byte at a time. Nothing is saved.
* Sound and music are files in data/ (upstream's SDL 2 build unzips them
  from sdl2/sdl2_data.h into $HOME at each start; here
  amiga/tools/extract_data.py unpacks them once, at build time). The 21
  sound effects go through acsdl12's mixer (16 channels, mono, 22050 Hz).
  The seven MODs play on acsdl12's MOD player; the four XMs (the menu's
  DEADFEEL among them) on ibxm, the replayer upstream's libretro core
  carries, in acsdl12's music hook (amiga/music_amiga.c). Panning is
  ignored (acsdl12 is mono).
* Only one pad: acsdl12 gives the first pad ACGame finds. With the two
  keyboard players that is three humans; the other places can be bots.
* No netplay, no savestates (libretro only), no SDL 2 options (turbo,
  team-mode switches from the command line).

Building
--------

    make -f amiga/Makefile.amigaos STOVE=/path/to/openamigagcc \
         NDK=/path/to/gfopensdk/OpenNDK/Include_H \
         ACSDL=/path/to/acsdl12

needs python3 for the data. Output: build/amigaos/MrBoom/ and its icon
build/amigaos/MrBoom.info. The upstream changes are under __amigaos__ in
sdl/sdl.cpp, common.cpp and mrboom_data.c (m aligned to a long word); the
rest is in amiga/. The icons are made with port_icons.py from
amiga/icons/icons.json (the art is the game's own icon).
