#!/bin/sh
# The upstream source and data, ready to build: package/aa.lock's commit of
# ExiguusEntertainment/AmuletsArmor is fetched into DIR/upstream (once), then
#   DIR/src   Source/*.C and Include/*.H (and Source/Win32, Generic, DOS,
#             Unix), named in lower case, every #include "..." too, DOS's
#             CRs taken out, and amiga/patches applied;
#   DIR/data  the game's files from Exe/ as upstream names them (the .RES
#             files, maps, forms, MAPDESC/ and the S0000000/ save slot),
#             without the PC programs and drivers and without AAMUSIC/
#             (71 MB of CD audio the port does not play).
#   sh package/prepare-source.sh DIR
# GPL-3.0-or-later, like Amulets & Armor. Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/.." && pwd)
. "$HERE/aa.lock"
D=${1:?DIR}
mkdir -p "$D"
U="$D/upstream"
if [ "$(git -C "$U" rev-parse HEAD 2>/dev/null)" != "$AA_COMMIT" ]; then
    rm -rf "$U"; mkdir -p "$U"
    git -C "$U" init -q
    git -C "$U" fetch -q --depth 1 "$AA_GIT" "$AA_COMMIT"
    git -C "$U" checkout -q FETCH_HEAD
fi
[ "$(git -C "$U" rev-parse HEAD)" = "$AA_COMMIT" ] || { echo "amuletsarmor: commit mismatch" >&2; exit 1; }

lower() { echo "$1" | tr 'A-Z' 'a-z'; }
# a C file as the port builds it: lower-case name and #include names, no CRs
c_file() {
    tr -d '\r' < "$1" | sed -E 's/^([ \t]*#[ \t]*include[ \t]*["<])([^">]*)/\1\L\2/' > "$2"
}

rm -rf "$D/src" "$D/data"
mkdir -p "$D/src" "$D/data"
for f in "$U"/Source/* "$U"/Include/*; do
    [ -f "$f" ] || continue
    b=$(basename "$f")
    case "$b" in
    *.C|*.H|*.c|*.h) c_file "$f" "$D/src/$(lower "$b")" ;;
    esac
done
for sub in Win32 Generic DOS Unix; do
    s=$(lower "$sub"); mkdir -p "$D/src/$s"
    for f in "$U/Source/$sub"/*; do
        [ -f "$f" ] && c_file "$f" "$D/src/$s/$(lower "$(basename "$f")")"
    done
done
for p in "$ROOT"/amiga/patches/*.patch; do
    patch -s -d "$D/src" -p1 < "$p"
done

for f in "$U"/Exe/*; do
    b=$(basename "$f")
    if [ -d "$f" ]; then
        case "$b" in MAPDESC|S0000000) cp -R "$f" "$D/data/$b" ;; esac
        continue
    fi
    case "$(lower "$b")" in
    *.exe|*.dll|*.bat|*.386|*.conf|*.ico|*.bmp) ;;
    *) cp "$f" "$D/data/$b" ;;
    esac
done
