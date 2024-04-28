#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *user_input;

// 現在着目しているトークン
Token *token;

Node *code[100];

int main(int argc, char **argv) {
        if (argc != 2) {
                error("引数の個数が正しくありません");
                return 1;
        }

	// トークナイズしてパースする
        user_input = argv[1];
        tokenize();
        program();
	// アセンブリの前半部分を出力
        printf(".intel_syntax noprefix\n");
        printf(".globl main\n");
        printf("main:\n");

	// プロローグ
	// 変数２６個分の領域を確保する
	printf("	push rbp\n");
	printf("	mov rbp, rsp\n");
	printf("	sub rsp, 208\n");

	// 先頭の式から順にコード生成
        for(int i = 0; code[i]; i++){
		gen(code[i]);
		
		// 式の評価結果としてスタックに一つの値が残っている
		// はずなので、スタックが溢れないようにポップしておく
        	printf("        pop rax\n");
	}

	// エピローグ	
	// 最後の式の結果がRAXに残っているのでそれが返り値になる
	printf("	mov rsp, rbp\n");
	printf("	pop rbp\n");
        printf("        ret\n");
        return 0;
}
