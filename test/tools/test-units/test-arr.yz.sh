#!/bin/bash

src="$1"
bin="$2"

$bin

if [ $? -ne 0 ]; then
	exit 1
fi
