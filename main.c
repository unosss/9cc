#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
        if (argc != 2) {
                error("引数の個数が正しくありません");
                return 1;
        }

// トークナイズしてパースする
        user_input = argv[1];
        token = tokenize(user_input);
        Node *node = expr();
// アセンブリの前半部分を出力
        printf(".intel_syntax noprefix\n");
        printf(".globl main\n");
        printf("main:\n");

// 抽象構文木を下りながらコード生成
        gen(node);

// スタックトップに式全体の値が残っているはずなので
// それをRAXにロードして関数からの返り値とする
        printf("        pop rax\n");
        printf("        ret\n");
        return 0;
}
