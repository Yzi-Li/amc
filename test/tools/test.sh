#!/bin/bash

if [[ "$(basename $(pwd))" != "tools" ]]; then
	echo Please run in \'tools\' directory!
	exit 1
fi

items=$(find .. -maxdepth 1 -name '*.yz')

AS="as -gstabs"
LD="ld"
COMPILER_BIN="../../amc"
COMPILER_BIN_DEBUG="../../amc.debug"

BUILD_DIR="../build"
UNITS_DIR="test-units"

if [ -f "$COMPILER_BIN_DEBUG" ]; then
	COMPILER="$COMPILER_BIN_DEBUG"
elif [ -f "$COMPILER_BIN" ]; then
	COMPILER="$COMPILER_BIN"
else
	echo -e "\x1b[31mERROR\x1b[0m: Compiler not found!"
fi

assemble() {
	local input="$(get_output_asm $1)"
	local output="$(get_output_obj $input)"
	echo -e " -> \x1b[33mAssembling\x1b[0m: $(basename $input) -o $output"
	$AS "$input" -o "$output"
	local err=$?
	if [ $err -ne 0 ]; then
		echo -e "\x1b[31m -> ERROR\x1b[0m: Assembler stopped: $err!"
		echo -e "  > $AS $input -o $output"
		echo -en "  > \x1b[34mHINT\x1b[0m: Review this file? [y/n] "
		read ans
		if [ "$ans" = "y" ]; then
			$EDITOR $input
		else
			test_failed
		fi
	fi
}

clean_outputs() {
	local items=$(find $BUILD_DIR -name '*.yz.s')
	echo "Will remove these files:"
	for item in $items; do
		echo "$item"
	done
	echo -n "Remove?[y/n] "
	read ans
	[ "$ans" = "y" ] && rm -f $items
}

compile() {
	local input="$1"
	local output="$(get_output_asm $input)"
	echo -e "==> \x1b[34mCompiling\x1b[0m: $(basename $input) -o $output"
	$COMPILER "$input" -o "$output"
	err=$?
	if [ $err -ne 0 ]; then
		echo -e "\x1b[31m--> ERROR\x1b[0m: Compiler stopped: $err!"
		echo -e "  > $COMPILER $input -o $output"
		echo -en "  > \x1b[34mHINT\x1b[0m: debug compiler? [y/n] "
		read ans
		if [ "$ans" = "y" ]; then
			if [ ! -f $COMPILER_BIN_DEBUG ]; then
				echo -e "\x1b[31m  > ERROR\x1b[0m: Compiler(debug) not found!"
				test_failed
			fi
			gdb -args "$COMPILER_BIN_DEBUG" "$input" -o "$output"
			exit 0
		else
			test_failed
		fi
	fi
}

get_output_asm() {
	echo "$BUILD_DIR/$(basename $1).s"
}

get_output_bin() {
	echo "$BUILD_DIR/$(basename $1 .yz)"
}

get_output_obj() {
	echo "$BUILD_DIR/$(basename $1 .s).o"
}

get_outputs() {
	for i in $items; do
		outputs+="$(get_output_asm $i) "
	done
	echo $outputs
}

get_unit() {
	echo "$UNITS_DIR/test-$(basename $1).sh"
}

review() {
	$EDITOR $(get_outputs)
}

test_all() {
	for item in $items; do
		test_src "$item"
	done
}

test_failed() {
	echo -e "\x1b[31mTesting failed!\x1b[0m"
	exit 1
	#echo -en "Test other files? [y/n] "
	#read ans
	#[ "$ans" != "y" ] && return 1
}

test_src() {
	local input="$1"
	local unit="$(get_unit $input)"
	compile "$input"
	assemble "$input"
	link_obj "$input"
	if [ -f "$unit" ]; then
		test_unit "$unit" "$input" "$(get_output_bin $input)"
	else
		echo -e "\x1b[32mDONE\x1b[0m: $(basename $input)"
	fi
}

test_unit() {
	local script="$1"
	local src="$2"
	local bin="$3"
	echo -e " -> \x1b[33mUnit test begin\x1b[0m: $script"
	$script "$src" "$bin"
	local err=$?
	if [ $err -ne 0 ]; then
		echo -e "\x1b[31m -> ERROR\x1b[0m: Unit test stopped: $err!"
		echo -e "  > $script $src $bin"
		exit 1
	fi
	echo -e " <- \x1b[33mUnit test end\x1b[0m"
}

link_obj() {
	local input="$(get_output_obj $1)"
	local output="$(get_output_bin $1)"
	echo -e " -> \x1b[33mLinking\x1b[0m: $(basename $input) -o $output"
	$LD "$input" -o "$output"
	local err=$?
	if [ $err -ne 0 ]; then
		echo -e "\x1b[31m -> ERROR\x1b[0m: Linker stopped: $err!"
		echo -e "  > $LD $input -o $output"
		exit 1
	fi
}

case $1 in
	get_outputs) get_outputs ;;
	clean) clean_outputs ;;
	review) review ;;
	all|*) test_all ;;
esac
