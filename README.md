# AmigaAmuletsArmor

Amulets & Armor (1997, United Software Artists; GPL-3.0) for GFOS
(AmigaOS 3.2 with GFOpenUp) on a 68040 or 68060: the first-person RPG on
ACGame's 320 x 200 256-colour screen (acgame.library, opened at run time
through its stubs and shipped in the drawer), with no SDL and no
GFOpenGPU. Installed as a GFOpenUp part: `SYS:Games/AmuletsArmor`.

Status: builds and plays on a PC and on an emulated 68040 (QEMU) from the
same patched sources, frame for frame the same (see Tests); the Amiga
build is made by the stove, not yet run in a lab. The sound effects
play through ACGame 1.3 (AHI, else Paula); the CD music is not shipped;
one player only.

AMIGA_PORT.md is the drawer's ReadMe: the keys, what it needs, and how the
port works.

## Building

    STOVE=~/AmiCradle/stoves/os32-gcc16-open6 OUT=out sh package/build-amiga.sh

or in Cradle, stove_build with the os32-gcc16-open6 stove, target
`package`. Upstream (ExiguusEntertainment/AmuletsArmor) and DalsinAI/acgame
are fetched at the commits `package/aa.lock` pins; nothing of upstream is
kept here. `OUT` gets the archive (`AmuletsArmor-1.05.03a-gfos1-68040.lha`,
the drawer and Install-AmuletsArmor) and `Parts/AmuletsArmor`, installed
with

    GFOpenUpTool INSTALL Parts/AmuletsArmor

## Tests

    sh package/prepare-source.sh /tmp/aa && AA=/tmp/aa sh tests/host/test_aa.sh

on a PC:

- `tests/host/unit_test.c`: the byte swaps, the resource file header and
  index, pictures (plain and with column tables), weapon .MXY runs,
  spells, scripts and map lumps turned round with their names and bytes
  left alone, the blocks that are bytes left alone, the Amiga raw key to
  SDL key table, and DOS paths (59 checks);
- the whole game on `tests/host/aa_host.c` (a frame in memory, virtual
  time, scripted keys and mouse): the logos and title appear; a character
  is made and saved, the Town Hall's first quest is taken, and in the 3D
  world walking, turning, jumping and attacking change the view; the
  game's sound is mixed.

Big-endian, as on the Amiga, with a 68k Linux cross compiler and QEMU:

    HOSTCC="m68k-linux-gnu-gcc -static" AA_RUN="qemu-m68k -cpu m68040" AA=/tmp/aa \
        AA_SAME_AS=/tmp/aa-pc-frames sh tests/host/test_aa.sh

runs the same tests on an emulated 68040 and checks every frame is byte for
byte the PC's (AA_SAME_AS is a little-endian run's AA_TEST_KEEP).

## Layout

- `amiga/aa_main.c`: the game's frame, mouse and wait calls (what its
  Windows front end gave it). `amiga/aa_sdl.c`, `include/sdl.h`: the SDL
  1.2 calls it makes. `amiga/aa_keys.c`: raw keys to SDL keys.
- `amiga/aa_acgame.c`: all that on ACGame. `tests/host/aa_host.c`: on a PC.
- `amiga/aa_endian.c`: the data files' little-endian numbers in the
  68040's order. `amiga/aa_files.c`, `aa_compat.h`: DOS paths.
- `amiga/patches/0001`: the game's own code, where it had to change.
- `package/`: the lock, the source preparation, the stove build, the part.

Designed, tested and delivered by The AmiCradle Team.
