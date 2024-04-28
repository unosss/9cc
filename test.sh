#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./9cc "$input" > tmp.s
	cc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "($input) => $actual"
	else
		echo "($input) => $expected expected, but got $actual"
		exit 1
	fi
}

assert 0 "a=5-5;"
assert 3 "foo=2*7-9-2;"
echo "ok"
