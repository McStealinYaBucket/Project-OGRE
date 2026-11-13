#!/bin/sh
# Build share/ogre-private.zip: what a fresh clone lacks (the git-ignored build inputs under Client/ and
# staging/: files byte-identical to the 2016 source, RakNet, the G3D fork, FMOD headers, shaders) plus the
# IDA databases (ida/*.i64). For private sharing between contributors only: it holds leaked and
# licence-restricted code, so never publish it. Public upstream downloads (3rdparty/_downloads) are left
# out; tools/lib/install_libs.py fetches them again.
#   sh tools/make_share_zip.sh
set -e
cd "$(dirname "$0")/.."
for f in ida/*.id0 ida/*.id1 ida/*.nam ida/*.til; do
    [ -e "$f" ] && { echo "a database is open or unpacked ($f); close it first" >&2; exit 1; }
done
mkdir -p share
out=share/ogre-private.zip
rm -f "$out.tmp"
{ git ls-files --others --ignored --exclude-standard -- Client staging; ls ida/*.i64; } |
    zip -q -n .i64 -@ "$out.tmp"   # .i64s are already deflated by IDA: store them
mv "$out.tmp" "$out"
ls -l "$out"
