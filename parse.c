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
char *LMB = "{";
char *RMB = "}";
char *EQ = "==";
char *NOT_EQ = "!=";
char *LARGE = "<";
char *LARGE_EQ = "<=";
char *SMALL = ">";
char *SMALL_EQ = ">=";
char *EOS = ";";
char *ASS = "=";
char *COM = ",";

char *reg[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

LVar *locals;

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

bool consume_tk(TokenKind tk) {
	if(token->kind != tk) return false;
	token = token->next;
	return true;
}

Token *consume_ident() {
	Token *tok = calloc(1,sizeof(Token));
	if(token->kind == TK_IDENT) {
		tok->kind = token->kind;
 		tok->str = token->str;
		tok->len = token->len;
		tok->next = token->next;
		token=token->next;
	}
	return 	tok;
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);

        int pos = loc - user_input_orig;
        fprintf(stderr, "%s\n", user_input_orig);
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
                error("'%c'ではありません", token->str[0]);
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
        Token *tok = calloc(1, sizeof(Token));
        tok->kind = kind;
        tok->str = str;
        cur->next = tok;
	if (len) {
		tok->len = len;
	} else if (tok->kind == TK_NUM) {
		char *endup = user_input;
		tok->val = strtol(endup, &endup, 10);
		len = endup - user_input;
		tok->len = len;
	} else if (tok->kind == TK_IDENT) {
		char *endup = user_input;
		while(*endup && 'a' <= endup[0] && endup[0] <= 'z') {
			endup++;
		}
		len = endup - user_input;
		tok->len = len;
	} else tok->len = len;
	user_input += tok->len;
        return tok;
}


Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
        Node *node = calloc(1, sizeof(Node));
	init_node(&node);
        node->kind = kind;
        node->lhs = lhs;
        node->rhs = rhs;
	node->lex = true;
	node->rex = true;
        return node;
}

Node *new_node_num(int val){
        Node *node = calloc(1, sizeof(Node));
	init_node(&node);
        node->kind = ND_NUM;
        node->val = val;
        return node;
}

void function(){
	int index = 0;
	while(!at_eof) {
		strncpy(func_name[index], token->str, token->len);
		token = token->next;
		if(!consume(LB))
			error_at(token->str, "'('ではないトークンです");
		locals = calloc(1, sizeof(LVar));
		init_vector(vec[index],6);
		while(!consume(RB)){
			Node *buf = calloc(1,sizeof(Node));
			buf = primary();
			insert_vector(vec[index],buf);
			if(consume(RB))break;
			consume(COM);
		}
		if(!consume(LMB))
			error_at(token->str, "'{'ではないトークンです");
		program(index++);
		free(locals);
	}
	func_name[index] = NULL;
}

void program(int index){
	int i = 0;
	while (!consume(RMB))
		code[index][i++] = stmt();
	code[index][i] = NULL;
}

void init_node(Node **node){
	(*node)->m1ex = false;
	(*node)->m2ex = false;
	(*node)->lex = false;
	(*node)->rex = false;
}

void init_vector(Vector *v, size_t initialSize) {
	v->array = (Node **)malloc(initialSize * sizeof(Node));
	v->used = 0;
	v->size = initialSize;
}

void insert_vector(Vector *v, Node *node) {
	if (v->used == v->size) {
		v->size *= 2;
		v->array = (Node **)realloc(v->array, v->size * sizeof(Node));
	}
	v->array[v->used++] = node;
}

Node *at_vector(Vector *v, size_t index) {
	if (index < v->used) {
		return v->array[index];
	} else {
		printf("Index out of bounds\n");
		exit(1);
	}
}

Node *stmt(){
	Node *node = calloc(1,sizeof(Node));
	init_node(&node);
	if (consume_tk(TK_RETURN)) {
		node->kind = ND_RETURN;
		node->lhs = expr();
		node->lex = true;
		if (!consume(";"))
			error_at(token->str, "';'ではないトークンです");
	} else if (consume_tk(TK_IF)) {
		node->kind = ND_IF;
		consume(LB);
		node->lhs = expr();
		node->lex = true;
		consume(RB);
		node->rhs = stmt();
		node->rex = true;
		if (consume_tk(TK_ELSE)) {
			node->m1hs = stmt();
			node->m1ex = true;
		}	
	} else if(consume_tk(TK_WHILE)) {
		node->kind = ND_WHILE;
		consume(LB);
		node->lhs = expr();
		node->lex = true;
		consume(RB);
		node->rhs = stmt();
		node->rex = true;
	} else if(consume_tk(TK_FOR)) {
		node->kind = ND_FOR;
		consume(LB);
		if(!consume(";")){
			node->lhs = expr();
			node->lex = true;
			if(!consume(";"))
				error_at(token->str, "';'ではないトークンです");
		}
		if(!consume(";")){
			node->m1hs = expr();
			node->m1ex = true;
			if(!consume(";"))
				error_at(token->str, "';'ではないトークンです");
		}
		if(!consume(RB)){
			node->m2hs = expr();
			node->m2ex = true;
			if(!consume(RB))
				error_at(token->str, "';'ではないトークンです");
		}
		node->rhs = stmt();
		node->rex = true;

	} else if (consume(LMB)) {
		node->kind = ND_BLOCK;
		node->v = calloc(1,sizeof(Vector));
		init_vector(node->v, 10);
		while(!consume(RMB)){
			Node *buf = calloc(1,sizeof(Node));
			init_node(&buf);
			buf = stmt();
			insert_vector(node->v, buf);
		}
	} else {
		node = expr();
		if (!consume(";"))
			error_at(token->str, "';'ではないトークンです");
	}

	return node;
}

Node *assign(){
	Node *node = calloc(1,sizeof(Node));
	init_node(&node);
        node = equality();
	if (consume("="))
		node = new_node(ND_ASSIGN, node, assign());
	return node;
}

Node *expr(){
        return assign();
}

Node *equality(){
        Node *node = calloc(1,sizeof(Node));
	init_node(&node);
        node = relational();

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
        Node *node = calloc(1,sizeof(Node));
	init_node(&node);
	node = add();

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
        Node *node = calloc(1,sizeof(Node));
	init_node(&node);
	node = mul();
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
        Node *node = calloc(1,sizeof(Node));
	init_node(&node);
	node = unary();
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
                Node *node = calloc(1,sizeof(Node));
		init_node(&node);
		node = expr();
                expect(RB);
                return node;
        }

	Token *tok = consume_ident();
	if(tok->kind == TK_IDENT){
		Node *node = calloc(1, sizeof(Node));
		init_node(&node);
		if (consume(LB)){
			node->v = calloc(1,sizeof(Vector));
			init_vector(node->v, 6);
			node->kind = ND_FUNC;
			node->str = calloc(1,sizeof(char));
			strncpy(node->str, tok->str, tok->len);
			if (token->kind == TK_NUM) {
				for (;;) {
					insert_vector(node->v, new_node_num(expect_number()));
					if(!consume(COM))break;
				}
			}
			if (!consume(RB)){
				error_at(token->str, "')'ではないトークンです");
			}
		} else {
			node->kind = ND_LVAR;

			LVar *lvar = find_lvar(tok);
			if (lvar) {
				node->offset = lvar->offset;
			} else {
				lvar = calloc(1, sizeof(LVar));
				lvar->next = locals;
				lvar->name = tok->str;
				lvar->len = tok->len;
				lvar->offset = locals->offset + 8;
				node->offset = lvar->offset;
				locals = lvar;
			}
		}
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
void tokenize() {
        Token head;
        head.next = NULL;
        Token *cur = &head;

        while (*user_input) {
// 空白文字をスキップ
                if (isspace(*user_input)) {
                        user_input++;
                        continue;
                }

		if (strlen(user_input) >= 7 && strncmp(user_input, "return", 6) == 0 && !is_alnum(user_input[6])) {
			cur = new_token(TK_RETURN, cur, user_input, 6);
			continue;
		}

		if (strlen(user_input) >= 3 && strncmp(user_input, "if", 2) == 0 && !is_alnum(user_input[2])) {
			cur = new_token(TK_IF, cur, user_input, 2);
			continue;
		}

		if (strlen(user_input) >= 5 && strncmp(user_input, "else", 4) == 0 && !is_alnum(user_input[4])) {
			cur = new_token(TK_ELSE, cur, user_input, 4);
			continue;
		}

		if (strlen(user_input) >= 6 && strncmp(user_input, "while", 5) == 0 && !is_alnum(user_input[5])) {
			cur = new_token(TK_WHILE, cur, user_input, 5);
			continue;
		}

		if (strlen(user_input) >= 4 && strncmp(user_input, "for", 3) == 0 && !is_alnum(user_input[3])) {
			cur = new_token(TK_FOR, cur, user_input, 3);
			continue;
		}

		if ('a' <= user_input[0] && user_input[0] <= 'z') {
			cur = new_token(TK_IDENT, cur, user_input, 0);
			continue;
		}

                if (!memcmp(user_input,EQ,2) || !memcmp(user_input,NOT_EQ,2) || !memcmp(user_input,LARGE_EQ,2) || !memcmp(user_input,SMALL_EQ,2)) {
                        cur = new_token(TK_RESERVED, cur, user_input, 2);
                        continue;
                }

                if (*user_input == *LARGE || *user_input == *SMALL) {
                        cur = new_token(TK_RESERVED, cur, user_input, 1);
                        continue;
                }

		if (*user_input == *ASS) {
			cur = new_token(TK_RESERVED, cur, user_input, 1);
			continue;
		}

                if (*user_input == *ADD || *user_input == *SUB || *user_input == *MUL || *user_input == *DIV
			       	|| *user_input == *LB || *user_input == *RB || *user_input == *LMB || *user_input == *RMB || *user_input == *COM) {
                        cur = new_token(TK_RESERVED, cur, user_input, 1);
                        continue;
                }

                if (isdigit(*user_input)) {
                        cur = new_token(TK_NUM, cur, user_input, 0);
                        continue;
                }

		if(*user_input == *EOS) {
			cur = new_token(TK_RESERVED, cur, user_input, 1);
			continue;
		}

                error("トークナイズできません");
        }
        new_token(TK_EOF, cur, user_input, 0);
        token = head.next;
	return;
}

LVar *find_lvar(Token *tok) {
	for (LVar *var = locals; var; var = var->next)
		if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
			return var;
	return NULL;
}

int is_alnum(char c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
}
