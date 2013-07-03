#!/bin/bash

nick=${1-jac}
chan=${2-3dpe}

cat /dev/stdin | sed -ur "s/(.+)/:$nick!$nick@host PRIVMSG #$chan :\\1/" | \
	./bin/pbrane

