#!/bin/bash

nick=${1-jac}

while read -r line; do
	echo $nick
	echo $line
done | ./bin/teval

