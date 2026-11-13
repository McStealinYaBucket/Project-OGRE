#!/bin/sh
# Regenerate the progress images and HTML reports in docs/ (shown in README.md, served by GitHub Pages).
#   sh tools/progress.sh          (needs build/ and build-studio/ built, and reccmp-user.yml)
# Each report carries its exe's own icon (tools/exe_icon.py, from bin/), drawn smoothly rather than
# with the template's nearest-neighbour scaling. The SVG title is the exe name; "RobloxPlayerBeta.exe"
# overflows reccmp's template at its 12.7px font, so the title is shrunk afterwards.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ICONS="$(mktemp -d)"
trap 'rm -rf "$ICONS"' EXIT
gen() { # target build-dir exe
    python3 -P "$ROOT/tools/exe_icon.py" "$ROOT/$3" "$ICONS/$1.png" >/dev/null
    (cd "$ROOT/$2" && PYTHONIOENCODING=utf-8 "$ROOT/tools/capped.sh" "$ROOT/toolchain/venv/bin/reccmp-reccmp" \
        --target "$1" --silent --svg-icon "$ICONS/$1.png" \
        -S "$ROOT/docs/$1PROGRESS.SVG" -H "$ROOT/docs/$1PROGRESS.HTML")
    sed -i 's/font-size:12.7px/font-size:9px/; s/image-rendering:optimizeSpeed/image-rendering:auto/' \
        "$ROOT/docs/$1PROGRESS.SVG"
}
mkdir -p "$ROOT/docs"
gen PLAYER build bin/player_1023/RobloxPlayerBeta.exe
gen STUDIO build-studio bin/studio/RobloxStudioBeta.exe
