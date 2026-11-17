#!/bin/sh
# Host tests: the port's own pieces (tests/host/unit_test.c), then the whole
# game built for this PC from the same patched sources and amiga/aa_*.c, on
# tests/host/aa_host.c (a frame in memory, virtual time, scripted keys and
# mouse):
#  - intro: the logos and the title, each a different picture;
#  - play: a new character (named "aba"), the Town Hall's first quest
#    taken, the journal closed, then into the 3D world: W walks, D turns,
#    space jumps, Ctrl swings; the frames differ as it moves and turns, and
#    the game's sound is mixed as it plays.
#   AA=/path/to/prepared (package/prepare-source.sh) sh tests/host/test_aa.sh
#   AA_TEST_KEEP=DIR keeps the frames (PPM) there.
# Big-endian, as on the Amiga: with a 68k Linux cross compiler and QEMU,
#   HOSTCC="m68k-linux-gnu-gcc -static" AA_RUN="qemu-m68k -cpu m68040" sh tests/host/test_aa.sh
# runs the same tests on an emulated 68040 (amiga/aa_endian.c turning the
# data files' little-endian numbers round); AA_SAME_AS=DIR (a little-endian
# run's AA_TEST_KEEP) then checks that every frame is byte for byte the PC's.
# GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/../.." && pwd)
AA=${AA:?AA=prepared source (package/prepare-source.sh DIR)}
CC=${HOSTCC:-cc}
RUN=${AA_RUN:-}
T=$(mktemp -d "${TMPDIR:-/tmp}/aa-host.XXXXXX")
trap 'rm -rf "$T"' EXIT INT TERM

# 68k glibc under qemu: 32-bit file offsets and time (qemu-m68k's 64-bit
# calls are not all there), but readdir with 64-bit offsets
X=""; XD="-D_FILE_OFFSET_BITS=64"
case "$($CC -dumpmachine 2>/dev/null)" in
m68k*) X="-U_FILE_OFFSET_BITS -D_FILE_OFFSET_BITS=32 -U_TIME_BITS" ;;
esac
DEFS="-DWIN32=1 -DTARGET_UNIX=1 -DWIN_IPX=0 -DNO_ASSEMBLY=1 -Dcdecl= -D_cdecl= -D__cdecl= -D__fastcall="
INC="-I$ROOT/amiga/include -I$ROOT/amiga -iquote $AA/src"
UP="-x c -std=gnu99 -w -fpermissive -include $ROOT/amiga/aa_pre.h -O1 -funsigned-char -fno-strict-aliasing $DEFS $INC $X"
MINE="-std=gnu99 -Wall -Werror -Wno-unknown-pragmas -O1 -funsigned-char -fno-strict-aliasing $DEFS $INC $X"
for f in aa_main aa_sdl aa_keys aa_endian aa_files; do $CC $MINE -c -o "$T/$f.o" "$ROOT/amiga/$f.c"; done
$CC $MINE -c -o "$T/aa_host.o" "$HERE/aa_host.c"
$CC $MINE -c -o "$T/unit_test.o" "$HERE/unit_test.c"

# shellcheck disable=SC2086
$CC $X -o "$T/unit" "$T/unit_test.o" "$T/aa_keys.o" "$T/aa_endian.o" "$T/aa_files.o"
$RUN "$T/unit"

mkdir -p "$T/up"
for f in "$AA"/src/*.c; do
    b=$(basename "$f" .c)
    case "$b" in debugbor|tosdata) continue ;; esac
    $CC $UP -c -o "$T/up/$b.o" "$f"
done
# upstream's Unix file opener: the game asks for its files in lower case,
# they are upper case on the disk (AmigaDOS doesn't mind, a PC does)
$CC -O1 $XD -c -o "$T/casefile.o" "$AA/src/unix/casefile.c"
# shellcheck disable=SC2086
$CC $X -o "$T/aa" "$T"/up/*.o "$T/aa_main.o" "$T/aa_sdl.o" "$T/aa_keys.o" "$T/aa_endian.o" \
    "$T/aa_files.o" "$T/aa_host.o" "$T/casefile.o" -lm -Wl,--wrap=open -Wl,--wrap=fopen

fail=0
summary() { sed -n "s/^$2 //p" "$1/summary.txt"; }
differ() { ! cmp -s "$1" "$2"; }
ppm_colours() {   # distinct colours in a PPM frame
    python3 -c 'import sys
d=open(sys.argv[1],"rb").read(); i=d.index(b"255\n")+4; px=d[i:]
print(len({px[j:j+3] for j in range(0,len(px),3)}))' "$1"
}

run() {   # run NAME VAR=VALUE...: the game, in a fresh copy of its drawer, until AA_END
    n=$1; shift
    mkdir -p "$T/$n"; rm -rf "$T/data-$n"; cp -R "$AA/data" "$T/data-$n"
    if ! env AA_DATA="$T/data-$n" AA_OUT="$T/$n" "$@" $RUN "$T/aa" > "$T/$n/stdout.txt" 2> "$T/$n/stderr.txt"; then
        echo "FAIL $n: the game stopped"; tail -30 "$T/$n/stdout.txt" "$T/$n/stderr.txt"; fail=1; return 1
    fi
    [ -f "$T/$n/summary.txt" ] || { echo "FAIL $n: no summary"; fail=1; return 1; }
}

# intro: the logos, then the title
if run intro AA_END=20000 AA_SHOTS=2000,8000,19000; then
    [ "$(summary "$T/intro" opened)" = 1 ] || { echo "FAIL intro: the screen was not opened once"; fail=1; }
    [ "$(summary "$T/intro" shows)" -gt 20 ] || { echo "FAIL intro: few frames shown"; fail=1; }
    differ "$T/intro/shot-2000.ppm" "$T/intro/shot-19000.ppm" || { echo "FAIL intro: the picture never changed"; fail=1; }
    c=$(ppm_colours "$T/intro/shot-19000.ppm"); [ "$c" -ge 20 ] || { echo "FAIL intro: the title has $c colours"; fail=1; }
    echo "intro: logos and title shown ($c colours on the title)"
fi

# play: Create character, the name "aba" (a 0x20, b 0x35), Accept, the
# Town Hall, its quest, Accept, the journal closed; then W (0x11) walks,
# D (0x22) turns, space (0x40) jumps, Ctrl (0x63) swings, W again
MOUSE="27000:215:55:0,27500:215:55:1,27800:215:55:0,34000:180:187:0,34500:180:187:1,34800:180:187:0"
MOUSE="$MOUSE,40000:30:23:0,40500:30:23:1,40800:30:23:0,44000:117:167:0,44500:117:167:1,44800:117:167:0"
MOUSE="$MOUSE,48000:309:9:0,48300:309:9:1,48500:309:9:0,49000:160:80:0"
KEYS="31000:32:1,31100:32:0,31300:53:1,31400:53:0,31600:32:1,31700:32:0"
KEYS="$KEYS,50000:17:1,53000:17:0,53000:34:1,54200:34:0,54200:17:1,57000:17:0"
KEYS="$KEYS,58000:32:1,59500:32:0,60000:99:1,60300:99:0,61000:17:1,64000:17:0"
SHOTS="32000,37000,45000,49500,52000,55000,58000,60200,63000,65500"
if run play AA_END=66000 AA_KEYS="$KEYS" AA_MOUSE="$MOUSE" AA_SHOTS="$SHOTS"; then
    differ "$T/play/shot-32000.ppm" "$T/play/shot-37000.ppm" || { echo "FAIL play: character creation went nowhere"; fail=1; }
    differ "$T/play/shot-49500.ppm" "$T/play/shot-52000.ppm" || { echo "FAIL play: walking changed nothing"; fail=1; }
    differ "$T/play/shot-52000.ppm" "$T/play/shot-55000.ppm" || { echo "FAIL play: turning changed nothing"; fail=1; }
    differ "$T/play/shot-60200.ppm" "$T/play/shot-63000.ppm" || { echo "FAIL play: walking again changed nothing"; fail=1; }
    c=$(ppm_colours "$T/play/shot-52000.ppm"); [ "$c" -ge 40 ] || { echo "FAIL play: the 3D view has $c colours"; fail=1; }
    [ -f "$T/data-play/S0000000/CHDATA00" ] || { echo "FAIL play: the character was not saved"; fail=1; }
    [ "$(summary "$T/play" loud)" -gt 0 ] || { echo "FAIL play: no sound was mixed"; fail=1; }
    echo "play: character made and saved, quest taken, the 3D world walks and turns ($c colours in the view)"
fi

if [ -n "${AA_SAME_AS:-}" ]; then
    n=0
    for f in intro/shot-2000 intro/shot-8000 intro/shot-19000 $(echo "$SHOTS" | tr ',' '\n' | sed 's|^|play/shot-|'); do
        n=$((n + 1))
        cmp -s "$T/$f.ppm" "$AA_SAME_AS/$f.ppm" || { echo "FAIL same: $f differs from $AA_SAME_AS"; fail=1; }
    done
    [ $fail = 0 ] && echo "same: all $n frames are byte for byte those of $AA_SAME_AS"
fi
if [ -n "${AA_TEST_KEEP:-}" ]; then mkdir -p "$AA_TEST_KEEP"; cp -R "$T/intro" "$T/play" "$AA_TEST_KEEP/"; fi
[ $fail = 0 ] && echo "host tests: all passed"
exit $fail
