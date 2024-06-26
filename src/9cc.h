/* 9cc.h */

#ifndef CC_H

#define CC_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type Type;

struct Type {
	enum { INT, PTR, ARRAY, CHAR } ty;
	struct Type *ptr_to;
	size_t array_size;
};

typedef struct Node Node;

typedef struct {
	Node **array;
	size_t used;
	size_t size;
} Vector;

void init_vector(Vector *v, size_t initialSize);

void insert_vector(Vector *v, Node *node);

Node *at_vector(Vector *v, size_t index);

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
	ND_BLOCK,	// block
	ND_FUNC,	// 関数の呼び出し
	ND_DEREF,	// ポインタ
	ND_ADDR,	// 参照
	ND_LDECLARE,	// ローカル変数の宣言
	ND_GDECLARE,	// グローバル変数の宣言
	ND_GVAR,	// グローバル変数
	ND_GINT,	// グローバル変数の宣言 + intで初期化
	ND_GCHAR,	// グローバル変数の宣言 + charで初期化
	ND_LC,		// 文字列リテラルの宣言
	ND_GLC,		// グローバル領域にある文字列リテラル
} NodeKind;


// 抽象構文木のノードの型
struct Node {
        NodeKind kind;  // ノードの型
        Node *lhs;      // 左辺
	Node *m1hs;	// 中間ノード１
	Node *m2hs;	// 中間ノード２
        Node *rhs;      // 右辺
        int val;        // 値
	int offset;	// RBPからのオフセット
	int memory;	// グローバル変数が確保するメモリ
	bool m1ex;	// 中間ノード１が使われたかどうか
	bool m2ex;	// 中間ノード２が使われたかどうか
	bool lex;	// 左辺が使われたかどうか
	bool rex;	// 右辺が使われたかどうか
	Vector *v;	// block
	char *str;	// 関数名、変数名
	Type *type;	// type
	char *lc;	// 文字列リテラル 
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
	TK_INT,	     // int
	TK_SIZEOF,   // sizeof
	TK_CHAR,     // char
	TK_LC,	     // 文字列リテラル
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


// ローカル変数
typedef struct LVar LVar;

struct LVar {
	LVar *next; // 次の変数かNULL
	char *name; // 変数の名前
	int len;    // 名前の長さ
	int offset; // RBPからのオフセット
	Type *type;  // type
};

// 関数名と型を紐づけるマップ
typedef struct FVar FVar;

struct FVar {
	FVar *next; // 次の関数かNULL
	char *name; // 関数名
	int len;    // 名前の長さ
	Type *type; // type
};

// グローバル変数
typedef struct GVar GVar;
struct GVar {
        GVar *next;  // 次の変数かNULL
        char *name;  // 変数の名前
        int len;     // 名前の長さ
        Type *type;  // type
};

// 文字列リテラル
typedef struct LC LC;
struct LC {
	LC *next;    // 次の文字列リテラル
	char *name;  // 変数の名前
	int len;     // 名前の長さ 
}; 

// 関数と変数の識別子
typedef enum {
	FUNC,
	VAR,
}ID;

extern LVar *locals;

extern FVar *functions;

extern GVar *glocals;

extern LC *lcs;

extern char *ADD;
extern char *SUB;
extern char *MUL;
extern char *DIV;
extern char *LB;
extern char *RB;
extern char *LMB;
extern char *RMB;
extern char *LLB;
extern char *RLB;
extern char *EQ;
extern char *NOT_EQ;
extern char *LARGE;
extern char *LARGE_EQ;
extern char *SMALL;
extern char *SMALL_EQ;
extern char *EOS;
extern char *ASS;
extern char *ADDR;
extern char *DEREF;
extern char *DQ;

extern char *filename;

extern char *user_input;

extern char *user_input_orig;

// 制御演算子のナンバリングに使用
extern int id;


// 現在着目しているトークン
extern Token *token;

extern Node *code[10][100];

extern char *reg[6];

extern char *common_name[10];

extern Type *common_type[10];

extern Vector *vec[10];

extern Vector *lc;

extern Type *int_list[10];

extern Type *char_list[10];

extern ID common_id[10];

void common();
void program();
Node *assign();
Node *stmt();
Node *expr();
Node *declare();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();

void gen(Node *node);

void gen_lval(Node *node);

void gen_int(int index);

void gen_char(int index);

void calc_type_depth(Type *type, int *cnt);

bool consume(char *op);

Token *consume_ident();

void expect(char *op);

int expect_number();

void tokenize();

void error(char *fmt, ...);

LVar *find_lvar(Token *tok);

FVar *find_fvar(Token *tok);

GVar *find_gvar(Token *tok);

LC *find_lc(Token *tok);

int is_alnum(char c);

void init_node(Node **node);

#endif
