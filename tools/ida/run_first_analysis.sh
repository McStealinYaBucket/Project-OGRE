#!/bin/sh
# Usage: run_first_analysis.sh TARGET path/to/binary   (from the repo root; refuses to overwrite a database)
set -e
[ -e "ida/$1.i64" ] && { echo "ida/$1.i64 exists, open it instead"; exit 1; }
TVHEADLESS=1 "${IDADIR:-$HOME/ida-pro-9.4}/idat" -A -Stools/ida/first_analysis.py -L"ida/logs/$1-00.log" -o"ida/$1.i64" "$2" >/dev/null
cp --update=none "ida/$1.i64" "ida/snapshots/$1-00-autoanalysis.i64"
tail -1 "ida/exports/$1-functions.csv" >/dev/null && grep -a "first_analysis:" "ida/logs/$1-00.log"
