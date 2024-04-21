#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *ADD = "+";
char *SUB = "-";
char *MUL = "*";
char *DIV = "/";
char *LB = "(";
char *RB = ")";
char *EQ = "==";
char *NOT_EQ = "!=";
char *LARGE = "<";
char *LARGE_EQ = "<=";
char *SMALL = ">";
char *SMALL_EQ = ">=";

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        exit(1);
}
bool consume(char *op) {
        if(token->kind != TK_RESERVED ||
                        strlen(op) != token->len ||
                        memcmp(token->str, op, token->len))
                return false;
        token = token->next;
        return true;
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);

        int pos = loc - user_input;
        fprintf(stderr, "%s\n", user_input);
        fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
        fprintf(stderr, "^ ");
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それいがいのばあいにはエラーを報告する。
void expect(char *op) {
        if (token->kind != TK_RESERVED ||
                        strlen(op) != token->len ||
                        memcmp(token->str, op, token->len))
                error("'%c'ではありません", op);
        token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
        if (token->kind != TK_NUM)
                error_at(token->str, "数ではありません");
        int val = token->val;
        token = token->next;
        return val;
}

bool at_eof() {
        return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
        Token *tok = calloc(1, sizeof(Token));
        tok->kind = kind;
        tok->str = str;
        cur->next = tok;
        return tok;
}

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


// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
        Token head;
        head.next = NULL;
        Token *cur = &head;

        while (*p) {
// 空白文字をスキップ
                if (isspace(*p)) {
                        p++;
                        continue;
                }

                if(*p == *EQ || *p == *NOT_EQ || *p == *LARGE_EQ || *p == *SMALL_EQ) {
                        cur = new_token(TK_RESERVED, cur, p++);
                        p++;
                        cur->len = 2;
                        continue;
                }

                if(*p == *LARGE || *p == *SMALL) {
                        cur = new_token(TK_RESERVED, cur, p++);
                        cur->len = 1;
                        continue;
                }

                if (*p == *ADD || *p == *SUB || *p == *MUL || *p == *DIV || *p == *LB || *p == *RB) {
                        cur = new_token(TK_RESERVED, cur, p++);
                        cur->len = 1;
                        continue;
                }

                if (isdigit(*p)) {
                        cur = new_token(TK_NUM, cur, p);
                        cur->val = strtol(p, &p, 10);
                        char buf[32];
                        snprintf(buf, 32, "%d", cur->val);
                        cur->len = strlen(buf);
                        continue;
                }

                error("トークナイズできません");
        }

        new_token(TK_EOF, cur, p);
        return head.next;
}
