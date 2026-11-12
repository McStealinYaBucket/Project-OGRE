/* HackDefines.h — October 2013 version.
   Evidence: RBX_BUILDSEED / HATE flag strings are absent from the Oct 2013 binaries;
   the obfuscation system (Security/RandomConstant LINE_RAND etc.) postdates Oct 2013.
   Kept as a near-empty compatibility shim until 2013 Verb.h is verified against the binary.
   NARROW EXCEPTION (2026-10-06): the Lua fork references HATE_LUA_HASH_CHANGED
   and the Oct 2013 binary PROVES DataModel::sendStats exists (BSS 0xFA6E5C),
   so the tamper-reporting path predates the rest of HATE. Two values are
   carried, BOTH binary-verified (batches 686/687): the luaL_loadprotected
   entry-mismatch arm compiles to "or byte [sendStats+1], 2" (= 0x200) and
   its streamed-hash EOF arm to "or byte [sendStats+1], 0x40" (= 0x4000). */

#pragma once

// Oct 2013 values proven from the binary disasm (see header note).
#define HATE_LUA_SCRIPT_HASH_CHANGED 0x200
#define HATE_LUA_HASH_CHANGED 0x4000

// 0x10: ScriptContext::executeInNewThread tamper telemetry (batch279/710:
// emplace of literal 16 into DataModel::hackFlagSet at the identity gates
// and the post-load write-lock gate). Same name/value in 2016 HackDefines.
#define HATE_ILLEGAL_SCRIPTS 0x10
