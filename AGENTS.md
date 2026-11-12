# AGENTS.md: Project OGRE

`CLAUDE.md` is the authoritative instruction file for this repository, and `DECOMP_PLAN.md` holds the phases and current status. Read both before touching anything. This file repeats the rules that cause the most damage when skipped, for agents that only load `AGENTS.md`. If this file and `CLAUDE.md` ever disagree, `CLAUDE.md` wins. To keep one copy, as OpenRBX does, replace this file with a symlink: `ln -sf CLAUDE.md AGENTS.md`.

## What this is

Project OGRE: a matching decompilation of the October 2013 Roblox Windows clients (targets **PLAYER** and **STUDIO**), modeled on [OpenRBX](https://github.com/Asphaltian/OpenRBX)'s December 2007 decomp. Source is rewritten until the original MSVC version compiles it to the same bytes. reccmp scores it.

## Non-negotiable rules

1. **IDA Pro 9.5, `.i64` databases, one canonical database per binary** in `ida/`. Open the existing `.i64` and never re-analyze from scratch. Snapshot to `ida/snapshots/` before any bulk script. **Every session that changes anything ends with a saved, packed `.i64`**, an entry in `ida/LOG.md`, and refreshed text exports in `ida/exports/`.
2. **`App/` is already about 55% decompiled for 2013.** Check the existing tree before writing any App function, build on it, and score it with reccmp. A claimed match only counts once reccmp measures it at 100%.
3. **Authority order:** the 2013 Windows bytes, then Mac Studio symbols, then iOS builds, then the 2016 source, then the 2007 OpenRBX tree. The 2016 source is a hypothesis about shape, never a body to paste.
4. **Never commit** the original binaries, the `.i64` files, the Mac or iOS builds, or any part of the 2016 source. All of them stay under the gitignored `bin/`, `ref/` and `ida/` paths.
5. **`// FUNCTION:` means 100%.** Anything less is `// STUB:`. `library` means third-party code only.
6. **Never guess** a name, offset, path or enum value that a higher source records. Placeholders (`FUN_xxxxxxxx`, `m_unk0xXX`, …) beat plausible guesses.
7. **Never patch a target binary** to test something. These clients self-check `.text`.
8. **Run every check** in `CLAUDE.md` → *Checks* before committing, not just the score.
9. **Stop and ask the user** if Phase 0 finds whole-program optimization (`/GL`) across most of the engine, or packing/protection on a target.
