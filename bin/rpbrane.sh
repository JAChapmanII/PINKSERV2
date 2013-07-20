#!/bin/bash

nick=${1-jac}
chan=${2-3dpe}

if [[ ! -f ./bin/pbrane ]]; then
	echo "./bin/pbrane does not exist, make sure you make'd!"
	exit 1
fi

cat /dev/stdin | sed -ur "s/(.+)/:$nick!$nick@host PRIVMSG #$chan :\\1/" | \
	./bin/pbrane

