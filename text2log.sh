# (C) Gorgi Kosev a.k.a. Spion, John Peterson. License GNU GPL 3.
i=1
while read -r line; do
	if [ "$line" ]; then
		echo :: $i blah blah : "$line"
	fi
	((i++))
done
