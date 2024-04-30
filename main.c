#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *user_input;

char *user_input_orig;

// 現在着目しているトークン
Token *token;

Node *code[10][100];

char *func_name[10];

Vector *vec[10];

int main(int argc, char **argv) {
        if (argc != 2) {
                error("引数の個数が正しくありません");
                return 1;
        }

	// トークナイズしてパースする
        user_input_orig = argv[1];

	user_input = user_input_orig;

        tokenize();
	//Token *check = calloc(1,sizeof(Token));
	//check = token;
	//while(check->kind != TK_EOF){
	//	printf("%s\n",check->str);
	//	check = check->next;
	//}
        function();
	// アセンブリの前半部分を出力
        printf(".intel_syntax noprefix\n");
        printf(".globl");
	for(int i = 0; func_name[i]; i++){
		if(i)printf(",");
		printf(" %s", func_name[i]);
	}
	printf("\n");
        for(int i = 0; func_name[i]; i++){
		printf("%s:\n", func_name[i]);
		// プロローグ
                // 変数２６個分の領域を確保する
                printf("        push rbp\n");
                printf("        mov rbp, rsp\n");
                printf("        sub rsp, 208\n");
		for(int j = 0;j < vec[i]->used; j++){
                        gen_lval(at_vector(vec[i],j));
                        printf("        pop rax\n");
                        printf("        mov [rax], %s\n", reg[j]);
                }
		for(int j = 0; code[i][j]; j++){
			gen(code[i][j]);
			// 式の評価結果としてスタックに一つの値が残っている
			// はずなので、スタックが溢れないようにポップしておく
        		if(code[i][j]->kind != ND_RETURN)printf("        pop rax\n");
        	}
	}
	return 0;
}
