#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
	TK_RESERVED, // 記号
	TK_NUM,      // 整数トークン
	TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
	TokenKind kind; // トークンの型
	Token *next;    // 次の入力トークン
	int val;        // kindがTK_NUMの場合、その数値
	char *str;      // トークン文字列
	int len;	// トークンの長さ
};

// 現在着目しているトークン
Token *token;

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

// 入力プログラム
char *user_input;

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

// 抽象構文木のノードの種類
typedef enum {
	ND_ADD,  // +
	ND_SUB,	 // -
	ND_MUL,	 // *
	ND_DIV,	 // /
	ND_NUM,	 // 整数
	ND_EQ,	 // ==
	ND_NOT_EQ,	 // !=
	ND_LARGE,	 // <
	ND_LARGE_EQ,	 // <= 
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
	NodeKind kind; // ノードの型
	Node *lhs;     // 左辺
	Node *rhs;     // 右辺
	int val;       // kindがND_NUMの場合のみ使う
};

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


Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();


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
		printf("	push %d\n", node->val);
		return;
	}
	gen(node->lhs);
	gen(node->rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");

	switch(node->kind){
	case ND_ADD:
		printf("	add rax, rdi\n");
		break;
	case ND_SUB:
		printf("	sub rax, rdi\n");
		break;
	case ND_MUL:
		printf("	imul rax, rdi\n");
		break;
	case ND_DIV:
		printf("	cqo\n");
		printf("	idiv rdi\n");
		break;
	case ND_EQ:
		printf("	cmp rax, rdi\n");
		printf("	sete al\n");
		printf("	movzb rax, al\n");
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
	printf("	push rax\n");
}


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
	printf("	pop rax\n");
	printf("	ret\n");
	return 0;
}
