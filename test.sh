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

assert 3 "int f(int x, int y){ return x + y; } int main(){ return f(1,2); }"
assert 10 "int f(int x){ if(x==0)return 0; int y; y=x-1; return x + f(y); } int main(){ return f(4);}"
assert 5 "int main(){int **z; int *y; *y = 5; z=&y; return **z;}"
echo "done"
