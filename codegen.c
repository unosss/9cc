#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void gen(Node *node){
	if(node->kind == ND_NUM){
                printf("        push %d\n", node->val);
                return;
        }
        gen(node->lhs);
        gen(node->rhs);

        printf("        pop rdi\n");
        printf("        pop rax\n");

        switch(node->kind){
        case ND_ADD:
                printf("        add rax, rdi\n");
                break;
        case ND_SUB:
                printf("        sub rax, rdi\n");
                break;
        case ND_MUL:
                printf("        imul rax, rdi\n");
                break;
        case ND_DIV:
                printf("        cqo\n");
                printf("        idiv rdi\n");
                break;
        case ND_EQ:
                printf("        cmp rax, rdi\n");
                printf("        sete al\n");
                printf("        movzb rax, al\n");
        case ND_NOT_EQ:
                printf("        cmp rax, rdi\n");
                printf("        setne al\n");
                printf("        movzb rax, al\n");
        case ND_LARGE:
                printf("        cmp rax, rdi\n");
                printf("        setl al\n");
                printf("        movzb rax, al\n");
        case ND_LARGE_EQ:
                printf("        cmp rax, rdi\n");
                printf("        setle al\n");
                printf("        movzb rax, al\n");
        }
        printf("        push rax\n");
}
