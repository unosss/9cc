#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
        Node *node = calloc(1, sizeof(Node));
        node->kind = kind;
        node->lhs = lhs;
        node->rhs = rhs;
        return node;
}

Node *new_node_num(int val){
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_NUM;
        node->val = val;
        return node;
}




Node *expr(){
        Node *node = equality();
        return node;
}

Node *equality(){
        Node *node = relational();

        for (;;){
                if(consume(EQ))
                        node = new_node(ND_EQ, node, relational());
                else if(consume(NOT_EQ))
                        node = new_node(ND_NOT_EQ, node, relational());
                else
                        return node;
        }
}

Node *relational(){
        Node *node = add();

        for (;;){
                if(consume(LARGE))
                        node = new_node(ND_LARGE, node, add());
                else if(consume(SMALL))
                        node = new_node(ND_LARGE, add(), node);
                else if(consume(LARGE_EQ))
                        node = new_node(ND_LARGE_EQ, node, add());
                else if(consume(SMALL_EQ))
                        node = new_node(ND_LARGE_EQ, add(), node);
                else
                        return node;
        }
}

Node *add(){
        Node *node = mul();

        for (;;){
                if(consume(ADD))
                        node = new_node(ND_ADD, node, mul());
                else if(consume(SUB))
                        node = new_node(ND_SUB, node, mul());
                else
                        return node;

        }
}

Node *mul(){
        Node *node = unary();

        for (;;){
                if(consume(MUL))
                        node = new_node(ND_MUL, node, unary());
                else if(consume(DIV))
                        node = new_node(ND_DIV, node, unary());
                else
                        return node;
        }
}

Node *primary(){
        // 次のトークンが"("なら、"(" expr ")"のはず
        if(consume(LB)){
                Node *node = expr();
                expect(RB);
                return node;
        }

        // そうでなければ数値のはず
        return new_node_num(expect_number());
}

Node *unary() {
        if (consume(ADD))
                return primary();
        if (consume(SUB))
                return new_node(ND_SUB, new_node_num(0), primary());
        return primary();
}

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
