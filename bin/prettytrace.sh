#!/bin/bash

otypes=('std::basic_string<char, std::char_traits<char>, std::allocator<char> >')
rtypes=('std::string')

cat /dev/stdin | while read -r line; do
	if [[ -n $(echo "$line" | grep '^err::ex'$'\t') ]]; then
		addr="$(echo "$line" | sed -r 's/.*\[0x([a-f0-9]+)\]$/\1/')"
		bin="$(echo "$line" | sed -r 's/^err::ex\t([^()]+)(\([^\)]*\))? \[0x[a-f0-9]+\]/\1/')"
		file="$(addr2line -e "$bin" "$addr" | tr ':' '\n' | head -1)"
		file="$(echo "$file" | sed -r 's:(.*@[^/]+)?(.*):\2:')"
		file="$(echo "$file" | sed -r "s:^$(pwd)/?::")"
		file="$(echo "$file" | sed 's:^[^.]*\./::')"
		line="$(addr2line -e "$bin" "$addr" | tr ':' '\n' | tail -1 | cut -d' ' -f1)"
		func="$(addr2line -e "$bin" "$addr" -f | head -1 | c++filt)"
		for i in $(seq 1 ${#otypes[@]}); do
			func="$(echo "$func" | sed "s/${otypes[$i - 1]}/${rtypes[$i - 1]}/g")"
		done
		[[ $file != "??" ]] && echo "    $file: $line: $func"
	else
		echo $line
	fi
done

