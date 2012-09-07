# (C) Gorgi Kosev a.k.a. Spion, John Peterson. License GNU GPL 3.
if [ -n "$2" ]; then d="-d$2"; else d=; fi
cat $1|./feedtriplie "$d" -r"[a-zA-Z]+\s([0-9]+\s[0-9]+:[0-9]+:[0-9]+)\s+<([-_0-9a-zA-Z\|\^\[\]\\\{\}\`]*)>\s+(.+)"