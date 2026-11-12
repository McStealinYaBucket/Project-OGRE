#!/bin/sh
# Kill our own runaway analysis processes before the kernel OOM-killer stops VS Code's whole scope.
#   tools/mem_watchdog.sh [limit_kb]   (default 10 GB RSS; logs kills to notes/mem-watchdog.log)
# Only python, idat/idalib, reccmp and cvdump processes of this user are candidates.
LIMIT=${1:-10485760}
LOG="$(dirname "$0")/../notes/mem-watchdog.log"
while :; do
    ps -u "$(id -u)" -o pid=,rss=,comm=,args= | awk -v lim="$LIMIT" \
        '$2 > lim && ($3 ~ /python|idat|reccmp|cvdump/) {print}' |
    while read -r pid rss comm args; do
        kill -9 "$pid" 2>/dev/null && echo "$(date -Is) killed pid $pid ($comm, ${rss} kB): $args" | cut -c1-400 >> "$LOG"
    done
    sleep 3
done
