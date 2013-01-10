#!/bin/bash

cat pscript/$1 | grep -vE '^\s*$' | while read -r line; do
	echo "$2"
	echo "$line"
done | segfind ./bin/teval

