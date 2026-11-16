Amulets & Armor 1.05.03a for GFOS
=================================

Amulets & Armor (1997, United Software Artists): a first-person fantasy
role-playing game. Make a character, take quests from the Town Hall, go
down into the dungeons, fight, cast spells from runes, and buy and sell in
the town between them. Its makers released the source and data under the
GPL (version 3).

Keys
----
  W S A D          forward, back, turn left and right
  Alt (or Amiga)   held: sidestep with A and D
  Shift            run
  Space            jump
  E                open, use what is in front
  Ctrl             attack with what is in the hand
  Del, Help        look up, look down; Backspace centres the view
  Keypad 1 to 9    spell runes; keypad Enter casts, keypad . aborts
  Alt + keypad     the INV, EQP, STA, OPT, JNL ... boxes
  1 to 7           the map: on, turn with you, see-through, clear,
                   smaller, bigger, where it sits
  P                pause
  Esc              the menu (save, options, quit)
The mouse moves the pointer over the game's own buttons and boxes; in the
3D view it turns and looks.

A pad (GFOpenInput, or a joystick in port 2): the stick or d-pad walks
and turns, A attacks, B opens, X jumps, Start is Esc.

What it needs
-------------
GFOS (AmigaOS 3.2 with GFOpenUp) on a 68040 or 68060; built soft float,
no FPU needed. The game draws on ACGame's 320 x 200 256-colour screen: on
RTG (and GFOpenGfx where it is installed) or AGA. It needs no SDL and no
GFOpenGPU. It ships acgame.library in the drawer's Libs/. About 40 MB of
disk.

Sound: not yet. The game mixes its effects as it plays, but ACGame has no
sound output yet, so nothing is heard. The music (AAMUSIC, 71 MB of
recorded CD audio) is not shipped.

One player: the network game (IPX and modem) is not built.

The game keeps its settings (config.ini) and saved characters (S0000000)
in its drawer; an update keeps them.

Installing
----------
From the archive: double-click Install-AmuletsArmor; it asks where the
drawer goes (Work:Games by default). With GFOpenUp:

  GFOpenUpTool INSTALL Parts/AmuletsArmor

puts it in SYS:Games/AmuletsArmor.

How it was ported
-----------------
The game's own code is built as its Unix (SDL 1.2) version, and the port
gives it what SDL and the Windows front end gave it:

  - its 320 x 200 frame and 256-colour palette are shown through ACGame
    (acgame.library) when they change, at most 50 times a second;
  - the keyboard reaches the game's key table by SDL key number, from the
    Amiga's raw keys (amiga/aa_keys.c); the mouse is the screen's pointer
    and its moves;
  - the sound mixer the game hands to SDL is run from the game's own loop
    for the time that has passed (amiga/aa_sdl.c);
  - DOS paths (S0000000\CHDATA00) are AmigaDOS paths (amiga/aa_files.c).

The game's data files were written on a PC by saving its structures as
bytes: every number in them is little-endian, and the 68040 is
big-endian. amiga/aa_endian.c turns each block round once, as it is read
(the resource files' headers and index, the pictures and their column
tables, the object types, the weapon animations, the spells, the items,
the map's lumps and its animation file, the scripts and the maths
tables). amiga/patches/0001 calls it where the game reads its files, and
reads the script code by bytes; the game's own code is otherwise as it
was. On an emulated 68040 the game gives the PC's frames byte for byte.

Designed, tested and delivered by The AmiCradle Team.
