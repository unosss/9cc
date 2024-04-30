#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./9cc "$input" > tmp_assert.s
	cc -o tmp_assert tmp_assert.s
	./tmp_assert
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "($input) => $actual"
	else
		echo "($input) => $expected expected, but got $actual"
		exit 1
	fi
}

func() {
	input="$1"
	echo "$input"
	./9cc "$input" > tmp_function.s
	cc -o tmp_function tmp_function.s foo.o
	./tmp_function
}

#assert 0 "a=5-5;"
#assert 3 "foo=2*7-9-2;"
#assert 4 "return 1+3;"
#assert 1 "a=7-2*3; if(a) return 1; else return 0;"
#assert 2 "b=1; while(b)b=0; return 2;"
#assert 6 "d = 0; for( d = 0;d <= 3;) { d = d + 1; d = d + 2; } return d;"
func "foo(3, 4, 5);"
echo "done"
