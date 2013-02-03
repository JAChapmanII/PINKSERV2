#!/bin/bash

script="$1"
[[ -z $(echo $script | grep -E '.pbrane$') ]] && script="${script}.pbrane"

nick="${2-jac}"

if [[ ! -f ./bin/teval ]]; then
	echo "./bin/teval does not exist, make sure you make'd!"
	exit 1
fi

cat "pscript/$script" | grep -vE '^\s*$' | while read -r line; do
	echo "$nick"
	echo "$line"
done | ./bin/teval

