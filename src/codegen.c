#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int id = 0;

bool stack_x16 = true;

void gen_lval(Node *node){
	printf("	mov rax, rbp\n");
	printf("	sub rax, %d\n", node->offset);
	printf("	push rax\n");
}

void gen(Node *node){
	switch(node->kind) {
	case ND_NUM:
		printf("	push %d\n", node->val);
		return;
	case ND_LVAR:
		gen_lval(node);
		if(node->type->ty == CHAR){
			printf("        pop rax\n");
			printf("	movsx ecx, BYTE PTR [rax]\n");
			printf("	push rcx\n");
		} else {
			printf("        pop rax\n");
			printf("	mov rax, [rax]\n");
			printf("	push rax\n");
		}
		return;
	case ND_GVAR:
		if(node->type->ty == ARRAY){
			printf("	mov rax, [rip + %s+%d]\n", node->str, node->memory);
			printf("	push rax\n");
		} else {
			printf("        mov rax, [rip + %s]\n", node->str);
			printf("        push rax\n");
		}
		return;
	case ND_GLC:
		printf("	lea rdi, [rip + %s]\n", node->str);
		printf("	mov eax, 0\n");
		printf("	push rdi\n");
		return ;
	case ND_ASSIGN:
		if(node->lhs->kind == ND_DEREF){
			gen(node->lhs->lhs);
		}else if(node->lhs->kind == ND_GVAR){
			if(node->type->ty == ARRAY){
				printf("	mov rax, [rip + %s+%d]\n",node->str,node->memory);
				printf("	push rax\n");
			}else{
				printf("        mov rax, [rip + %s]\n",node->str);
				printf("        push rax\n");
			}
		}else gen_lval(node->lhs);
		gen(node->rhs);
		if(node->type->ty == CHAR){
			printf("	pop rcx\n");
			printf("	pop rax\n");
			printf("	mov [rax], cl\n");
			printf("	push rcx\n");
		} else {
			printf("	pop rdi\n");
			printf("	pop rax\n");
			printf("	mov [rax], rdi\n");
			printf("	push rdi\n");
		}
		return;
	case ND_LDECLARE:
		gen_lval(node);
		if(node->lhs->kind != ND_LVAR)gen(node->lhs);
		else gen_lval(node->lhs);

		printf("        pop rdi\n");
		printf("        pop rax\n");
		printf("        mov [rax], rdi\n");
		printf("        push rax\n");
		return;
	case ND_GDECLARE:
		printf("	.zero %d\n", node->memory);
		return;
	case ND_GINT:
		printf("        .int %d\n", node->val);
		return;
	case ND_GCHAR:
		printf("	.byte %d\n", node->val);
		return;
	case ND_LC:
		printf("	.string %s\n", node->lc);
		return;
	case ND_RETURN:
		gen(node->lhs);
		printf("        pop rax\n");
		printf("	mov rsp, rbp\n");
		printf("	pop rbp\n");
		printf("	ret\n");
		return;
	case ND_IF:
		gen(node->lhs);
		printf("	pop rax\n");
		printf("	cmp rax, 0\n");
		if (node->m1ex) {
			printf("	je .Lelse%d\n", id);
			gen(node->rhs);
			printf("        jmp .Lend%d\n", id);
			printf(".Lelse%d:\n", id);
			gen(node->m1hs);
		} else {
			printf("	je .Lend%d\n", id);
			gen(node->rhs);
		}
		printf(".Lend%d:\n", id);
		id++;
		return;
	case ND_WHILE:
		printf(".Lbegin%d:\n", id);
		gen(node->lhs);
		printf("	pop rax\n");
		printf("	cmp rax, 0\n");
		printf("	je .Lend%d\n", id);
		gen(node->rhs);
		printf("	jmp .Lbegin%d\n", id);
		printf(".Lend%d:\n", id);
		id++;
		return;
	case ND_FOR:
		if (node->lex) gen(node->lhs);
		printf(".Lbegin%d:\n", id);
		if (node->m1ex) {
			gen(node->m1hs);
			printf("	pop rax\n");
			printf("	cmp rax, 0\n");
			printf("	je .Lend%d\n", id);
		}
		gen(node->rhs);
		if (node->m2ex) {
			gen(node->m2hs);
		}
		printf("	jmp .Lbegin%d\n", id);
		printf(".Lend%d:\n", id);
		id++;
		return;
	case ND_BLOCK:
		for (int i = 0; i < node->v->used; i++) {
			gen(at_vector(node->v, i));
			printf("	pop rax\n");
		}
		return;
	case ND_FUNC:
		if(!stack_x16)printf("	push 0\n");
		for (int i = 0; i < node->v->used; i++) {
			gen(at_vector(node->v, i));
			printf("	pop rax\n");
			printf("	mov %s, rax\n", reg[i]);
		}
		printf("	call %s\n", node->str);
		if(!stack_x16)printf("  pop rcx\n");
		printf("	push rax\n");
		stack_x16 = !(stack_x16);
		return;
	case ND_ADDR:
                gen_lval(node->lhs);
                return;
        case ND_DEREF:
                gen(node->lhs);
                printf("        pop rax\n");
                printf("        mov rax, [rax]\n");
                printf("        push rax\n");
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
		break;
        case ND_NOT_EQ:
                printf("        cmp rax, rdi\n");
                printf("        setne al\n");
                printf("        movzb rax, al\n");
		break;
        case ND_LARGE:
                printf("        cmp rax, rdi\n");
                printf("        setl al\n");
                printf("        movzb rax, al\n");
		break;
        case ND_LARGE_EQ:
                printf("        cmp rax, rdi\n");
                printf("        setle al\n");
                printf("        movzb rax, al\n");
		break;
        }
        printf("        push rax\n");
}
