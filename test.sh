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
	expected="$1"
	input="$2"
	
	./9cc "$input" > tmp_function.s
	cc -o tmp_function tmp_function.s hello.o
	./tmp_function
	actual="$?"

        if [ "$actual" = "$expected" ]; then
                echo "($input) => $actual"
        else
                echo "($input) => $expected expected, but got $actual"
                exit 1
        fi
}

#assert 6 "int main(){ int x; x=3; int y; y=3; return x+y;}"
#assert 3 "int f(int x, int y){ return x+y; } int main(){ return f(1,2); }"
#assert 10 "int f(int x){ if(x==0)return 0; int y; y=x-1; return x + f(y); } int main(){ return f(4);}"
#assert 5 "int main(){int **z; int *y; *y = 5; z=&y; return **z;}"
#assert 5 "int main(){int **z; **z=5; return 5;}"
#func 7 "int main(){int *p; alloc(&p, 1, 3, 7, 16); int *q; q=p+3; return *q;}"
#assert 12 "int main(){int *p; int q; return sizeof(p)+sizeof(q);}"
#assert 5 "int main(){int a[3]; a[0]=1; a[1]=5; a[2]=3; return a[1];}"
#assert 8 "int x=8; int main(){ return x; }"
#assert 3 "int main(){ char x[3]; x[0]=-1; x[1]=2; int y; y=4; return x[0]+y;}"
func 0 "int main(){ char *x; x=\"Hello World!\"; hello(x); return 0;}"
echo "done"
