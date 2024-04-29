/* 9cc.h */

#ifndef CC_H

#define CC_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// 抽象構文木のノードの種類
typedef enum {
        ND_ADD,		// +
        ND_SUB,		// -
        ND_MUL,		// *
        ND_DIV,		// /
        ND_NUM,		// 整数
        ND_EQ,		// ==
        ND_NOT_EQ,      // !=
        ND_LARGE,       // <
        ND_LARGE_EQ,    // <=
	ND_ASSIGN,	// =
	ND_LVAR,	// ローカル変数
	ND_RETURN,	// return
	ND_IF,		// if
	ND_WHILE,	// while
	ND_FOR,		// for
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
        NodeKind kind;  // ノードの型
        Node *lhs;      // 左辺
	Node *m1hs;	// 中間ノード１
	Node *m2hs;	// 中間ノード２
        Node *rhs;      // 右辺
        int val;        // kindがND_NUMの場合のみ使う
	int offset;	// kindがND_LVARの場合のみ使う
	bool m1ex;	// 中間ノード１が使われたかどうか
	bool m2ex;	// 中間ノード２が使われたかどうか
};

// トークンの種類
typedef enum {
        TK_RESERVED, // 記号
	TK_IDENT,    // 識別子
        TK_NUM,      // 整数トークン
        TK_EOF,      // 入力の終わりを表すトークン
	TK_RETURN,   // return
	TK_IF,	     // if
	TK_WHILE,    // while
	TK_FOR,	     // for
	TK_ELSE,     // else
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
        TokenKind kind; // トークンの型
        Token *next;    // 次の入力トークン
        int val;        // kindがTK_NUMの場合、その数値
        char *str;      // トークン文字列
        int len;        // トークンの長さ
};

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
	LVar *next; // 次の変数かNULL
	char *name; // 変数の名前
	int len;    // 名前の長さ
	int offset; // RBPからのオフセット
};

extern LVar *locals;

extern char *ADD;
extern char *SUB;
extern char *MUL;
extern char *DIV;
extern char *LB;
extern char *RB;
extern char *EQ;
extern char *NOT_EQ;
extern char *LARGE;
extern char *LARGE_EQ;
extern char *SMALL;
extern char *SMALL_EQ;
extern char *EOS;
extern char *ASS;

extern char *user_input;

extern char *user_input_orig;

extern int id;

// 現在着目しているトークン
extern Token *token;

extern Node *code[100];

void program();
Node *assign();
Node *stmt();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();

void gen(Node *node);

bool consume(char *op);

Token *consume_ident();

void expect(char *op);

int expect_number();

void tokenize();

void error(char *fmt, ...);

LVar *find_lvar(Token *tok);

int is_alnum(char c);

void init_node(Node **node);

#endif
