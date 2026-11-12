#!/bin/sh
# Regenerate the progress images and HTML reports in docs/ (shown in README.md, served by GitHub Pages).
#   sh tools/progress.sh          (needs build/ and build-studio/ built, and reccmp-user.yml)
# The SVG title is the exe name; "RobloxPlayerBeta.exe" overflows reccmp's template at its 12.7px
# font, so the title is shrunk afterwards.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
gen() { # target build-dir
    (cd "$ROOT/$2" && PYTHONIOENCODING=utf-8 "$ROOT/tools/capped.sh" "$ROOT/toolchain/venv/bin/reccmp-reccmp" \
        --target "$1" --silent -S "$ROOT/docs/$1PROGRESS.SVG" -H "$ROOT/docs/$1PROGRESS.HTML")
    sed -i 's/font-size:12.7px/font-size:9px/' "$ROOT/docs/$1PROGRESS.SVG"
}
mkdir -p "$ROOT/docs"
gen PLAYER build
gen STUDIO build-studio
