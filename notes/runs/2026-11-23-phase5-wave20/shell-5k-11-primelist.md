# shell-5k-11 → network-5f-11: the /GL shell now supplies prime_list_template<unsigned>::value

**Result: it works in both targets. The stand-in is in the working tree.**

## The change

`Client/Win/placeholder.cpp` (no marker, compiled by both shells) now includes
`<boost/unordered/detail/util.hpp>`. `ogreShellStandInUse` gains one line:

```cpp
*static_cast<std::size_t*>(raw) = boost::unordered::detail::next_prime(*static_cast<std::size_t*>(raw));
```

This makes the shell's LTCG object instantiate `boost::unordered::detail::prime_list_template<unsigned>::value` and
`::length`. 2013 does the same: its copy sits among the shell's LTCG data, PLAYER 0x00bdafe8 (after Document's slot
vftables), STUDIO 0x00d10920 (after RobloxDiagnosticsView's).

## Measured (PLAYER, build-sh/w20, full protocol each: two runs, regenerate, two runs, regenerate, one run)

| Link | Supplier of `value` (map) | Entries at effective 100 |
|---|---|---|
| XH (the HEAD placeholder) | NetworkPacketCache.cpp.obj, as in sweep26 | 13,036 |
| X (with the stand-in) | **placeholder.cpp.obj**: `value` 0x00843f90, then `length` 0x00844028 | 13,052 |

The two links differ only in placeholder.obj.

- **The prime_list users do not change.** The 52 original functions that reference 0x00bdafe0, 0x00bdafe8 or
  0x00bdb080 have identical scores in sweep26, XH and X: 15 at 100, 4 below, 33 unpaired. The list is in
  `build-sh/w20/runs/prime_users.txt`; compare it with `build-sh/w20/sel.py`.
- **The other differences between XH and X are twin churn.** Five ServerReplicator functions show up as
  unpaired→100 and two serializeSFFlag(s) rows move.
- **The one-past-end address now always lands on the same symbol.** In our link it is the shell's own `length`,
  which comes from the same COMDAT group, so the two stay adjacent. The users stayed at 100 with that neighbour.
- **The original's one-past-end is different.** 0x00bdb080 is the COL pointer of `CComPolyObject<CAxHostWindow>`'s
  vftable; 2013 emits `length` somewhere else or folds it.

## What this means for StatsUpdateJob

The shell is linked first, so its copy wins the COMDAT. ConcurrentRakPeer.obj can declare the two `unordered_map`s,
and NetworkPacketCache can keep its own, without becoming the supplier, so the `"circular_buffer"` literal can no
longer land on value+end.

I did not build ConcurrentRakPeer with the maps declared. Please measure your StatsUpdateJob change on top of this
placeholder (A/B with the full protocol). If anything still moves, the next thing to check is what our link puts
after `length`.

**STUDIO:** the stand-in is in both shells. In SH → SP (identical objects except placeholder.obj, full protocol
each), the score went 12,240 → 12,246, +6 / −0:

- SH puts `value` in NetworkPacketCache (0x007cc5f8); SP puts it in placeholder.obj (0x00838a88).
- The six gains are Lua no-.reloc coincidences.
