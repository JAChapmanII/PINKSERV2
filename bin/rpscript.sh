#!/bin/bash

script="$1"
[[ -z $(echo $script | grep -E '.pbrane$') ]] && script="${script}.pbrane"

nick="${2-jac}"

cat "pscript/$script" | ./bin/rpbrane.sh

