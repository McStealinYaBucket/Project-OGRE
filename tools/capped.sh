#!/bin/sh
# Run a command in its own systemd user scope with a memory cap.
#   tools/capped.sh cmake --build build
#   OGRE_MEMMAX=12G tools/capped.sh toolchain/venv/bin/python -I tools/ida/import_types.py dump ...
# Why: Claude Code runs inside VS Code's systemd scope, and systemd stops the whole scope (VS Code
# included) when the kernel OOM-kills any process in it (2026-11-12: a 13-22 GB python took VS Code
# down twice). In its own scope a runaway command hits its own MemoryMax and only it is killed.
# Exit status 137 (or a "oom-kill" message from systemd) means the command needed more than the cap.
exec systemd-run --user --scope --quiet --collect \
    -p MemoryMax="${OGRE_MEMMAX:-8G}" -p MemorySwapMax=0 -- "$@"
