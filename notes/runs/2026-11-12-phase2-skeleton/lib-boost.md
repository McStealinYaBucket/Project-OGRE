# lib-boost (workflow phase2-skeleton, wf_e3afb0d0-6b4)

Boost 1.53.0 is now compiled with VC9 and matched against PLAYER, STUDIO and both shipped copies of boost.dll. The flags are proven for each part. The one exception is program_options, which PLAYER built with /GL (LTCG), so its internal functions cannot match an .obj compile.

**Sources**
- boost_1_53_0.tar.bz2 came from sourceforge. Its MD5 a00d22605d5dbcfb4c9936a9b35bc4c2 equals the hash sourceforge publishes for that file.
- It is extracted in 3rdparty/_downloads/boost_1_53_0/boost_1_53_0. Those headers are left in place as the boost include set for the whole engine build.
- zlib-1.2.3.tar.gz came from zlib.net/fossils. Its MD5 debc62758716a169df9f62e6ab2bc634 is the published 1.2.3 value. It is extracted in 3rdparty/_downloads/zlib-1.2.3 and supplies the headers only.
- The compiled sources are staged as pristine upstream copies in staging/boost_1_53_0/libs/{filesystem,program_options,iostreams,system,thread}/{src,build}, with the licence file. No source change was needed, so there is no patch file.
- 3rdparty/_downloads/ is not in .gitignore, although the task brief says it is. I did not edit .gitignore.

**Which zlib iostreams uses**
- PLAYER has only one zlib: G3D's bundled 1.2.3 copy. Its deflateInit2_ is at 0x009e1e80 and ends with `ret 0x20`, so that copy was built stdcall (ZLIB_WINAPI).
- boost::iostreams::detail::zlib_base::do_init (0x00613170) pushes ZLIB_VERSION "1.2.3" (0x00b29614) and calls straight into that copy. Compiled with ZLIB_WINAPI against the 1.2.3 headers it is byte-identical.
- D3DX's zlib 1.1.4 is a separate copy and is not used by boost.

**Results** (PLAYER and STUDIO rows in ida/exports/PLAYER-lib-boost.csv)
- **PLAYER:** 605 functions located, 540 identical. The out-of-line functions from the boost .cpp files break down as:

| Library | Located | Identical |
|---|---|---|
| filesystem | 34 | 34 |
| iostreams | 11 | 11 |
| program_options | 10 | 5 |

- **PLAYER header code:** 550 of the located functions are header inline or template bodies, and 490 of those are identical.
- **STUDIO:** 475 located, 437 identical. Out-of-line: filesystem 31/31, iostreams 11/11, program_options 1/1.
- **boost.dll:** rows are in ida/exports/BOOSTDLL-lib-boost.csv. The Oct 15 Player's copy (bin/player/boost.dll) has 337 of 337 located functions identical, 52 of them out-of-line thread and system functions. The different-hash copy in bin/studio/boost.dll gives the same 337/337. That second calibration passed completely.
- **boost.dll imports:** PLAYER imports 18 boost.dll names through the linker IAT at 0x00aa2d00–0x00aa2d44. All 18 are exports of the shipped boost.dll.

**How functions were located** (evidence column)
- **body:** the masked bytes are equal at one target function start.
- **body+verified:** the body matched, and its relocations were checked against known import, RTTI or vftable names and against .obj strings.
- **body+xref:** the body matched in several places, and a located caller's relocation picked the right one.
- **vftable:** the function is a slot of a vftable whose name RTTI gives.
- **xref:** a located, identical caller's relocation gives the address.

Other notes on the CSV:
- **Folded functions:** functions the linker folded together get one row per address, with the other names under `folded-with:`.
- **Shared with Roblox code:** 6 rows (5 PLAYER, 1 STUDIO) are trivial bodies the linker shared with Roblox functions, for example 0x00402730 and 0x0040b950. They are tagged `icf-shared-with:`.

**Why the 65 PLAYER and 38 STUDIO rows are not identical**
- **Header bodies:** 60 PLAYER rows and all 38 STUDIO rows are header bodies whose linked copy came from a Roblox source file, not a boost .cpp. An example is the gzip_error ctor at 0x005f0e60.
- **program_options:** 5 PLAYER rows are program_options out-of-line functions, explained by /GL below.

## Structured fields

```json
{
 "files": [
  "tools/lib/boost_match.py",
  "ida/exports/PLAYER-lib-boost.csv",
  "ida/exports/BOOSTDLL-lib-boost.csv",
  "staging/boost_1_53_0",
  "3rdparty/_downloads/boost_1_53_0/boost_1_53_0.tar.bz2",
  "3rdparty/_downloads/boost_1_53_0/boost_1_53_0",
  "3rdparty/_downloads/zlib-1.2.3/zlib-1.2.3.tar.gz",
  "3rdparty/_downloads/zlib-1.2.3/zlib-1.2.3"
 ],
 "csv": "ida/exports/PLAYER-lib-boost.csv",
 "matched_identical": 977,
 "located_not_identical": 103,
 "flags": "Common to everything: VC9 SP1 15.00.30729 through tools/msvc9/cl, with defines WIN32 NDEBUG _LIB _SECURE_SCL=0 ZLIB_WINAPI _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0501 NTDDI_VERSION=0x05010100 BOOST_ALL_NO_LIB. Includes are 3rdparty/_downloads/boost_1_53_0/boost_1_53_0 and the zlib 1.2.3 headers.\n\n1. Static boost libraries in the exe (filesystem, iostreams, program_options): /O2 /Ob2 /Oi /Oy /arch:SSE /fp:fast /GR /EHsc /MD, which is the App flags without /GS-. Also BOOST_SYSTEM_DYN_LINK and BOOST_THREAD_DYN_LINK, because system and thread live in boost.dll.\n   - /GS (stack cookie on) is proven by boost::iostreams::zlib_error::check at 0x006130e0, which has the __security_cookie prologue. Under /GS- it does not match.\n   - /EHsc is proven by boost::filesystem::codecvt_error_category at 0x009c0f20. boost.build's default /EHs wraps the function-local static's atexit registration in an EH frame, and PLAYER has none. Under /EHs this preset drops from 23/23 to 19/20 out-of-line.\n   - /Oy is consistent with every identical function having no ebp frame.\n   - /arch:SSE, /fp:fast and /Oi produce identical code but no function decides between them. They stay unproven for boost.\n   - zlib calls are stdcall (ZLIB_WINAPI): proven by zlib_base::do_init at 0x00613170 and deflateInit2_ at 0x009e1e80, which ends with ret 0x20.\n\n2. boost.dll (Client/boostlibs, thread and system): /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD, the same as the App flags from the Lua probe. Also BOOST_THREAD_BUILD_DLL, BOOST_SYSTEM_DYN_LINK, BOOST_SYSTEM_SOURCE and BOOST_CHRONO_HEADER_ONLY. The DLL was linked /INCREMENTAL (exports go through jmp thunks).\n   - /GS- is proven by 47/47 identical out-of-line thread and system functions. With /GS it is 35/44.\n   - BOOST_SYSTEM_SOURCE in every file is proven by boost::thread::start_thread at 0x10011730. It calls system_category with a direct call (0x10011751 call rel32), not through an import.\n   - BOOST_CHRONO_HEADER_ONLY is proven by boost::thread::try_join_until at 0x10014880. It inlines system_clock::now() (GetSystemTimeAsFileTime minus 0x019db1ded53e8000), and boost.dll exports no chrono symbol.\n\n3. program_options is built /GL (LTCG) in PLAYER. boost::program_options::untyped_value::xparse at 0x00478330 calls the multiple_occurrences constructor at 0x00478060 with `this` pushed on the stack rather than passed in ecx. It also keeps a `push ebp; mov ebp,esp; and esp,-8` aligned frame. Only link-time code generation rewrites a call like that. Its virtual functions match (vftable-slot functions such as error_with_option_name::what at 0x00478880 are identical), and so do its /GS cookies.\n\nThe 2016 vcxproj's /arch:SSE2 and boost.build's defaults (/EHs, no /arch) both score worse. The probe presets are kept in the script (roblox, roblox_nogs, roblox_gs, vcxproj2016, bjam).\n\nTo reproduce: python3 -P tools/lib/boost_match.py [--preset P] [--no-compile] [--check] [-v]. The objects go to /tmp/claude-1000/ogre-boost-obj/<preset>, not build/, because the CMake skeleton wiped build/ during this run.",
 "unresolved": [
  "program_options is /GL (LTCG) in PLAYER, so an .obj compile cannot match its internal functions. Five located out-of-line functions are not identical: 0x00478170 value_semantic_codecvt_helper<char>::parse, 0x00478330 untyped_value::xparse, 0x00478cd0 error_with_option_name::substitute_placeholders, 0x00478fb0 ambiguous_option::substitute_placeholders and 0x0047b880 variables_map::get. Its non-virtual internals stay unlocated: only 10 of 120 out-of-line program_options functions are placed in PLAYER. Matching them needs a whole-program /LTCG link of the client.",
  "/arch:SSE, /fp:fast and /Oi for the boost static libraries are consistent with the bytes, but no boost function decides between them.",
  "Header inline and template bodies that are not identical (60 in PLAYER, 38 in STUDIO, for example the gzip_error ctor at 0x005f0e60 and clone_impl copy constructors) are copies the linker took from Roblox source files compiled with other flags. They are not boost .cpp code and are left as identical=no.",
  "A few byte-identical boost helpers were deliberately not placed because several functions with identical bytes claim the same address and their relocations cannot tell them apart. Examples: windows_file_codecvt::do_length/do_unshift at 0x009bee30 and anonymous-namespace handle_wrapper::~handle_wrapper at 0x009bdb20. Template bodies with only std:: names are left out by design, for example std::locale::locale<windows_file_codecvt> at 0x009bee90.",
  "Only 34 of 114 filesystem and 11 of 94 iostreams out-of-line functions are located. The rest are most likely discarded by the linker's /OPT:REF because nothing uses them. This was not checked function by function.",
  "3rdparty/_downloads/ is not in .gitignore, although the task brief says it is gitignored. I did not edit .gitignore.",
  "6 CSV rows (5 PLAYER, 1 STUDIO) are trivial bodies the linker shared with Roblox functions, for example PLAYER 0x00402730 and 0x0040b950. They are tagged icf-shared-with:. reccmp CSV generation should not let them claim the Roblox function's address."
 ]
}
```

