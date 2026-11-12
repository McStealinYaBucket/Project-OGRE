#!/usr/bin/env bash
# Assemble the portable VC9 SP1 (15.00.30729.01) x86 toolset in toolchain/msvc9/.
#
# Run: bash tools/msvc9/assemble.sh            (from anywhere; idempotent)
#      FORCE=1 bash tools/msvc9/assemble.sh    (rebuild toolchain/msvc9 from the admin images)
#
# Inputs (gitignored, under toolchain/):
#   vs2008-dvd/   contents of en_visual_studio_2008_professional_x86_dvd_x14-26326.iso (7z x)
#   sp1-iso/      contents of VS2008SP1ENUX1512962.iso (only vs90sp1/*.msp is used)
#   winsdk-src/   WCU/WinSDK/WinSDK_{Build,W32Tools}.exe from the DVD, unpacked with 7z x
#
# Method: Windows Installer administrative installs under wine, so every file lands at the path
# the MSI File/Component/Directory tables (and the MSP transforms) give it. No file is picked by
# its FL_ cab name. Measured 2026-11-12 with wine 10.0:
#   * wine msiexec /a with BOTH SP1 MSPs in PATCH= silently drops every file KB947888 patches or
#     adds (VC/include 145 of 188 headers, VC/lib 25 of 58 files) while still returning success.
#     That image (toolchain/vsadmin-sp1) is wrong and must not be used.
#   * One patch per admin install works: each image then contains byte-identical copies of the
#     MSP payloads (KB947888: 1949 of 1959 payloads present, the other 10 are alternate-key .rc
#     locale duplicates; KB945140: every x86-host compiler/linker/mspdb payload present).
#   * The two patches touch disjoint files in the subset used here: KB945140 changes 13
#     (cl/c1/c1xx/c2/link/ml/nmake, *.config, mspdb80/mspdbcore/mspdbsrv), KB947888 changes
#     or adds 913 (CRT/STL/ATL/MFC headers and libs). So: base = RTM+KB947888 image, overlay
#     the files the RTM+KB945140 image changed relative to the plain RTM image.
#   * Windows SDK v6.0A: no SP1 MSP targets the headers/libs product (VistaClientHeadersLibs),
#     so Include/Lib are RTM. KB946729 patches the Win32 tools product (RC.Exe, RcDll.Dll, mt.exe).
#     KB946733 (.NET tools) and KB950424 (wsatui.dll) are irrelevant here.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
T="$ROOT/toolchain"
export WINEPREFIX="$T/wineprefix" WINEDEBUG=-all
# The repo path contains a space; wine reaches it through drive O: (dosdevices/o: -> repo root).
[ -e "$WINEPREFIX/dosdevices/o:" ] || { wineboot -i >/dev/null 2>&1; ln -sfn "$ROOT" "$WINEPREFIX/dosdevices/o:"; }

admin() { # admin <msi (O:\ path)> <target dir name> [patch (O:\ path)]
	local msi="$1" dst="$2" patch="${3:-}"
	[ -f "$T/$dst/$(basename "${msi//\\//}")" ] && return 0
	mkdir -p "$T/$dst"
	wine msiexec /a "$msi" /qn "TARGETDIR=O:\\toolchain\\$dst\\" ${patch:+"PATCH=$patch"} \
		/l*v "O:\\toolchain\\logs\\$dst.log" >/dev/null 2>&1
}
SP1='O:\toolchain\sp1-iso\vs90sp1'
admin 'O:\toolchain\vs2008-dvd\vs_setup.msi' vsadmin
admin 'O:\toolchain\vs2008-dvd\vs_setup.msi' vsadmin-vc "$SP1\\VC90sp1-KB947888-x86-enu.msp"
admin 'O:\toolchain\vs2008-dvd\vs_setup.msi' vsadmin-vs "$SP1\\VS90sp1-KB945140-X86-ENU.msp"
admin 'O:\toolchain\winsdk-src\WinSDK_Build\VistaClientHeadersLibs-x86.msi' sdkadmin-hl
admin 'O:\toolchain\winsdk-src\WinSDK_W32Tools\VistaClientWin32Tools-x86.msi' sdkadmin-w32 "$SP1\\WinSDK-KB946729.msp"

OUT="$T/msvc9"
if [ -d "$OUT" ] && [ -z "${FORCE:-}" ]; then echo "$OUT exists (FORCE=1 to rebuild)"; exit 0; fi
rm -rf "$OUT"
VS="Program Files/Microsoft Visual Studio 9.0"
RTM="$T/vsadmin/$VS" VC="$T/vsadmin-vc/$VS" VSP="$T/vsadmin-vs/$VS"
SDK="Program Files/Microsoft SDKs/Windows/v6.0A"

# pick <relpath under VS dir>: the KB945140 file if that patch changed it, else RTM+KB947888.
pick() {
	if [ -f "$VSP/$1" ] && ! cmp -s "$VSP/$1" "$RTM/$1"; then echo "$VSP/$1"; else echo "$VC/$1"; fi
}
copy_tree() { # copy_tree <relsrc dir> <dst> <find depth args...>
	local src="$1" dst="$2"; shift 2
	(cd "$VC/$src" && find . "$@" -type f) | while read -r f; do
		mkdir -p "$dst/$(dirname "$f")"; cp -p "$(pick "$src/$f")" "$dst/$f"
	done
}
copy_tree VC/bin "$OUT/VC/bin" -maxdepth 1   # x86-hosted, x86-targeting
copy_tree VC/bin/1033 "$OUT/VC/bin/1033"
for f in mspdb80.dll mspdbcore.dll mspdbsrv.exe msobj80.dll; do cp -p "$(pick "Common7/IDE/$f")" "$OUT/VC/bin/"; done
copy_tree VC/include "$OUT/VC/include"
copy_tree VC/lib "$OUT/VC/lib" -maxdepth 1                           # x86 only (no amd64/ia64)
copy_tree VC/atlmfc/include "$OUT/VC/atlmfc/include"
copy_tree VC/atlmfc/lib "$OUT/VC/atlmfc/lib" -maxdepth 1
mkdir -p "$OUT/SDK"
cp -rp "$T/sdkadmin-hl/$SDK/Include" "$OUT/SDK/Include"
cp -rp "$T/sdkadmin-hl/$SDK/Lib" "$OUT/SDK/Lib"
cp -rp "$T/sdkadmin-w32/$SDK/Bin" "$OUT/SDK/bin"

{
	echo "# toolchain/msvc9 manifest: <file version>\t<sha256>\t<path>. Built by tools/msvc9/assemble.sh"
	(cd "$OUT" && find . -type f ! -name MANIFEST.txt | sort | sed 's|^\./||' |
		tr '\n' '\0' | xargs -0 python3 -P "$ROOT/tools/msvc9/pever.py")
} > "$OUT/MANIFEST.txt"
echo "built $OUT: $(($(wc -l < "$OUT/MANIFEST.txt") - 1)) files"
