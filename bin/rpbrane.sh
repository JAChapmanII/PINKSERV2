#!/bin/bash

nick=${1-jac}
chan=${2-3dpe}
rdir=${3-run}

if [[ ! -f ./bin/pbrane ]]; then
	echo "./bin/pbrane does not exist, make sure you make'd!"
	exit 1
fi

pbrane="$(realpath "./bin/pbrane")"

mkdir -p "${rdir}"
cd "${rdir}"

cat /dev/stdin \
	| sed -ur "s/(.+)/console :$nick!$nick@host PRIVMSG #$chan :\\1/" \
	| ${pbrane}

