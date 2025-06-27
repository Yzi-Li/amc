#!/bin/bash

items=$(find . -maxdepth 1 -name '*.yz')

COMPILER_BIN="../amc"
COMPILER_BIN_DEBUG="../amc.debug"

if [ -f "$COMPILER_BIN_DEBUG" ]; then
	COMPILER="$COMPILER_BIN_DEBUG"
elif [ -f "$COMPILER_BIN" ]; then
	COMPILER="$COMPILER_BIN"
else
	echo -e "\x1b[31mERROR\x1b[0m: Compiler not found!"
fi

get_outputs() {
	for i in $items; do
		outputs+="$i.s "
	done
	echo $outputs
}

clean_outputs() {
	items=$(find . -name '*.yz.s')
	echo "Will remove these files:"
	for item in $items; do
		echo "$item"
	done
	echo -n "Remove?[y/n] "
	read ans
	[ "$ans" = "y" ] && rm -f $items
}

compile() {
	input="$1"
	output="$1.s"
	$COMPILER $input -o $output
	err=$?
	if [ $err -ne 0 ]; then
		echo -e "\x1b[31mERROR\x1b[0m: Compile failed: $err!"
		echo -e "\t$COMPILER $input -o $output"
		exit 1
	fi
	echo -e "\x1b[32mDONE\x1b[0m: $input -o $output"
}

compile_all() {
	for item in $items; do
		compile "$item"
	done
}

review() {
	$EDITOR $(get_outputs)
}

case $1 in
	get_outputs) get_outputs ;;
	clean) clean_outputs ;;
	review) review ;;
	all|*) compile_all ;;
esac
