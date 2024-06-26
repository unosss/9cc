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
char *LLB = "[";
char *RLB = "]";
char *EQ = "==";
char *NOT_EQ = "!=";
char *LARGE = "<";
char *LARGE_EQ = "<=";
char *SMALL = ">";
char *SMALL_EQ = ">=";
char *EOS = ";";
char *ASS = "=";
char *COM = ",";
char *DEREF = "*";
char *ADDR = "&";
char *DQ = "\"";

char *reg[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

LVar *locals;

FVar *functions;

GVar *globals;

LC *lcs;



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


// エラーの起きた場所を報告するための関数
// 下のようなフォーマットでエラーメッセージを表示する
//
// foo.c:10: x = y + + 5;
//                   ^ 式ではありません
void error_at(char *loc, char *msg) {
  // locが含まれている行の開始地点と終了地点を取得
  char *line = loc;
  while (user_input < line && line[-1] != '\n')
    line--;

  char *end = loc;
  while (*end != '\n')
    end++;

  // 見つかった行が全体の何行目なのかを調べる
  int line_num = 1;
  for (char *p = user_input; p < line; p++)
    if (*p == '\n')
      line_num++;

  // 見つかった行を、ファイル名と行番号と一緒に表示
  int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  // エラー箇所を"^"で指し示して、エラーメッセージを表示
  int pos = loc - line + indent;
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf(stderr, "^ %s\n", msg);
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
	node->type = calloc(1,sizeof(Type));
	int lcnt = 0, rcnt = 0;
	calc_type_depth(node->lhs->type, &lcnt);
	calc_type_depth(node->rhs->type, &rcnt);
	if(lcnt < rcnt)node->type = node->rhs->type;
	else node->type = node->lhs->type;
	node->lex = true;
	node->rex = true;
        return node;
}

Node *new_node_num(int val){
        Node *node = calloc(1, sizeof(Node));
	init_node(&node);
        node->kind = ND_NUM;
        node->val = val;
	node->type = calloc(1,sizeof(Type));
	node->type->ty = INT;
        return node;
}

void gen_int(int index){
	int_list[index] = calloc(1,sizeof(Type));
	if(index){
		gen_int(index-1);
		int_list[index]->ptr_to = calloc(1,sizeof(Type));
		int_list[index]->ptr_to = int_list[index-1];
		int_list[index]->ty = PTR;
	} else {
		int_list[index]->ty = INT;
	}
}

void gen_char(int index){
        char_list[index] = calloc(1,sizeof(Type));
        if(index){
                gen_char(index-1);
                char_list[index]->ptr_to = calloc(1,sizeof(Type));
                char_list[index]->ptr_to = char_list[index-1];
                char_list[index]->ty = PTR;
        } else {
                char_list[index]->ty = CHAR;
        }
}

void calc_type_depth(Type *type, int *cnt){
	if(type->ty == PTR){
		*cnt = *cnt + 1;
		calc_type_depth(type->ptr_to, cnt);
	}
}

void common(){
	int index = 0;
	while(!at_eof()) {
		int type_list_index = 0;
		while(consume("*"))type_list_index++;
		if(consume_tk(TK_INT)){
			common_type[index] = int_list[type_list_index];
		} else if(consume_tk(TK_CHAR)){
			common_type[index] = char_list[type_list_index];
		}
		common_name[index] = calloc(1,sizeof(char));
		strncpy(common_name[index], token->str, token->len);
		token = token->next;
		if(!consume(LB)){
			common_id[index] = VAR;
			GVar *gvar = calloc(1,sizeof(GVar));
                        gvar->next = globals;
                        gvar->name = common_name[index];
                        gvar->len = strlen(gvar->name);
                        gvar->type = calloc(1,sizeof(Type));
			Node *node = calloc(1,sizeof(Node));
			node->kind = ND_GDECLARE;
			if(consume(LLB)){
				gvar->type->ptr_to = calloc(1,sizeof(Type));
				gvar->type->ptr_to = common_type[index];
				gvar->type->ty = ARRAY;
				int size = expect_number();
				gvar->type->array_size = size;
				if(gvar->type->ptr_to->ty == INT)node->memory = 4*size;
				else if(gvar->type->ptr_to->ty == CHAR)node->memory = size;
				else node->memory = 8*size;
				consume(RLB);
			} else {
				gvar->type = common_type[index];
				if(gvar->type->ty == INT)node->memory = 4;
				else if(gvar->type->ty == CHAR)node->memory = 1;
				else node->memory = 8;
			}
			globals = gvar;
			if(consume("=")){
				if(gvar->type->ty == INT){
					node->kind = ND_GINT;
					node->val = expect_number();
				}else if(gvar->type->ty == CHAR){
					node->kind = ND_GCHAR;
					node->val = expect_number();
				}else {// TODO: 配列の初期化
				}
			}
			code[index][0] = node;
                        code[index][1] = NULL;
                        index++;
			if(!consume(";")){
				error_at(token->str, "';'ではないトークンです");
			}
		} else {
			common_id[index] = FUNC;
			FVar *fvar = calloc(1,sizeof(FVar));
                	fvar->next = functions;
                	fvar->name = common_name[index];
                	fvar->len = strlen(fvar->name);
                	fvar->type = calloc(1,sizeof(Type));
                	fvar->type = common_type[index];
                	functions = fvar;
			
			locals = calloc(1, sizeof(LVar));
			vec[index] = calloc(1,sizeof(Vector));
			init_vector(vec[index],6);
			while(!consume(RB)){
				Node *buf = calloc(1,sizeof(Node));
				buf = expr();
				insert_vector(vec[index],buf);
				if(consume(RB))break;
				consume(COM);
			}		
			if(!consume(LMB))
				error_at(token->str, "'{'ではないトークンです");
			program(index++);
			free(locals);
		}
	}
	common_name[index] = NULL;
}

void program(int index){
	if(common_id[index] == FUNC){
		int i = 0;
		while (!consume(RMB)){
			Node *buf = stmt();
			if(buf->kind == ND_LC)continue;
			code[index][i++] = buf;
		}
		code[index][i] = NULL;
	}
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
	if (consume("=")){
		Node *buf = calloc(1,sizeof(Node));
		buf = assign();
		if(buf->kind == ND_LC){
			node->lc = calloc(1,sizeof(char));
			node->lc = buf->lc;
			node->kind = buf->kind;
			insert_vector(lc, node);
			LC *lc = calloc(1,sizeof(LC));
			lc->name = node->str;
			lc->len = strlen(lc->name);
			lc->next = lcs;
			lcs = lc;
		} else node = new_node(ND_ASSIGN, node, buf);
	}
	return node;
}

Node *expr(){
	if(consume_tk(TK_INT)){
                Node *node = calloc(1,sizeof(Node));
		node = declare(TK_INT);
		LVar *lvar = calloc(1,sizeof(LVar));
        	lvar->next = locals;
		lvar->name = node->str;
		lvar->len = strlen(lvar->name);
        	lvar->offset = node->offset;
		lvar->type = calloc(1,sizeof(Type));
		lvar->type = node->type;
        	locals = lvar;
		return node;
	} else if(consume_tk(TK_CHAR)){
                Node *node = calloc(1,sizeof(Node));
                node = declare(TK_CHAR);
                LVar *lvar = calloc(1,sizeof(LVar));
                lvar->next = locals;
                lvar->name = node->str;
                lvar->len = strlen(lvar->name);
                lvar->offset = node->offset;
                lvar->type = calloc(1,sizeof(Type));
                lvar->type = node->type;
                locals = lvar;
                return node;
	} else	return assign();
}

Node *declare(TokenKind kind) {
	Node *node = calloc(1,sizeof(Node));
        init_node(&node);
	LVar *lvar = calloc(1,sizeof(LVar));
	if (consume(DEREF)){
		node->kind = ND_LDECLARE;
		if(kind == TK_INT)node->lhs = declare(TK_INT);
		else if(kind == TK_CHAR)node->lhs = declare(TK_CHAR);
		else error("その型には対応していません");
		lvar->type = calloc(1,sizeof(Type));
		lvar->type->ty = PTR;
		lvar->type->ptr_to = calloc(1,sizeof(Type));
		lvar->type->ptr_to = node->lhs->type;
		lvar->next = locals;
        	lvar->offset = locals->offset + 8;
        	node->offset = lvar->offset;
        	locals = lvar;
		node->type = calloc(1,sizeof(Type));
		node->type = lvar->type;
		node->str = calloc(1,sizeof(char));
		node->str = node->lhs->str;
		return node;	
        }
	if (token->kind == TK_IDENT){
		Token *tok = consume_ident();
		node->str = calloc(1,sizeof(char));
                strncpy(node->str, tok->str, tok->len);
		if(consume("[")){
			node->type = calloc(1,sizeof(Type));
			node->type->ty = ARRAY;
			int size = expect_number();
			node->type->array_size = (size_t)size;
			node->v = calloc(1,sizeof(Vector));
			init_vector(node->v,size);
			Vector *vec = calloc(1,sizeof(Vector));
			init_vector(vec,size);
			for(int i = 0; i<size;i++){
				Node *buf = calloc(1,sizeof(Node));
				LVar *buf_lvar = calloc(1,sizeof(LVar));
				buf_lvar->next = locals;
                        	buf_lvar->offset = locals->offset + 8;
				buf->offset = buf_lvar->offset;
				locals = buf_lvar;
				Node *val = calloc(1,sizeof(Node));
				buf->lhs = calloc(1,sizeof(Node));
				buf->lhs = val;
				buf->kind = ND_LDECLARE;
				insert_vector(vec, buf);
			}
			for(int i = 0; i<size;i++){
				Node *buf = at_vector(vec,i);
				LVar *buf_lvar = calloc(1,sizeof(LVar));
                                buf_lvar->next = locals;
                                buf_lvar->offset = locals->offset + 8;
				buf->lhs->offset = buf_lvar->offset;
                                locals = buf_lvar;
                                buf->lhs->kind = ND_LVAR;
				insert_vector(node->v,buf);
			}
                        node->offset = at_vector(node->v,size-1)->offset;
                        node->kind = ND_BLOCK;
			node->type->ptr_to = calloc(1,sizeof(Type));
			if(kind == TK_INT)node->type->ptr_to = int_list[0];
			else if(kind == TK_CHAR)node->type->ptr_to = char_list[0];
			else error("その型には対応していません");
			consume("]");
			return node;
		} else {	
			lvar->next = locals;
			lvar->offset = locals->offset + 8;
			lvar->type = calloc(1,sizeof(Type));
			if(kind == TK_INT)lvar->type = int_list[0];
			else if(kind == TK_CHAR)lvar->type = char_list[0];
			else error("その型には対応していません");
			node->type = calloc(1,sizeof(Type));
			node->type = lvar->type;
			node->offset = lvar->offset;
			locals = lvar;
			node->kind = ND_LVAR;
			return node;
		}
	} else {
		error("変数がありません");
	}
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
	Node *buf = calloc(1,sizeof(Node));
        for (;;){
                if(consume(ADD)){
			if(node->type->ty == PTR){
				buf = mul();
				int type_size = 8;
				if(node->type->ptr_to->ty == INT)type_size = 4;
				else if(node->type->ptr_to->ty == CHAR)type_size = 1;
				buf = new_node(ND_MUL, buf, new_node_num(type_size));
                        	node = new_node(ND_ADD, node, buf);
			} else if(node->type->ty == ARRAY) {
				buf = mul();
                                int type_size = 8;
                                buf = new_node(ND_MUL, buf, new_node_num(type_size));
                                node = new_node(ND_ADD, node, buf);
			}
			else
				node = new_node(ND_ADD, node, mul());
		} else if(consume(SUB)){
			if(node->type->ty == PTR){
                                buf = mul();
				int type_size = 8;
                                if(node->type->ptr_to->ty == INT)type_size = 4;
				else if(node->type->ptr_to->ty == CHAR)type_size = 1;
                                buf = new_node(ND_MUL, buf, new_node_num(type_size));
                                node = new_node(ND_SUB, node, buf);
			}
                        else
                                node = new_node(ND_SUB, node, mul());
		} else
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
	Node *node = calloc(1,sizeof(Node));
        init_node(&node);
        // 次のトークンが"("なら、"(" expr ")"のはず
        if(consume(LB)){
		node = expr();
                expect(RB);
                return node;
        }
	if (consume(DEREF)){
                node->kind = ND_DEREF;
                node->lhs = primary();
                node->type = calloc(1,sizeof(Type));
                node->type = node->lhs->type->ptr_to;
                return node;
        }
        if (consume(ADDR)){
                node = new_node(ND_ADDR, unary(), new_node_num(0));
                node->type = calloc(1,sizeof(Type));
                node->type->ty = PTR;
                node->type->ptr_to = calloc(1,sizeof(Type));
                node->type->ptr_to = node->lhs->type;
                return node;
        }
	if(token->kind == TK_LC){
		node->kind = ND_LC;
		node->lc = calloc(1,sizeof(char));
		strncpy(node->lc, token->str, token->len);
		consume_tk(TK_LC);
		return node;
	}
	Token *tok = consume_ident();
	if(tok->kind == TK_IDENT){
		if (consume(LB)){
			node->v = calloc(1,sizeof(Vector));
			init_vector(node->v, 6);
			node->kind = ND_FUNC;
			node->str = calloc(1,sizeof(char));
			strncpy(node->str, tok->str, tok->len);
			while(!consume(RB)) {
				Node *buf = unary();
				insert_vector(node->v, buf);
				if(consume(RB))break;
				consume(COM);
			}
			FVar *fvar = find_fvar(tok);
                        if (fvar) {
                                node->type = calloc(1,sizeof(Type));
                                node->type = fvar->type;
                        } else {
				node->type = calloc(1,sizeof(Type));
				node->type->ty = INT;
				// TODO: 外部関数を呼び出す場合、以下のエラー処理をコメントアウトする必要あり
                                //error("関数 %s が定義されていません", tok->str);
                        }
		} else if (consume(LLB)) {
			int index = expect_number();
			Node *buf1 = calloc(1,sizeof(Node));
			Node *buf2 = calloc(1,sizeof(Node));
			buf2->kind = ND_LVAR;
                        LVar *lvar = find_lvar(tok);
			GVar *gvar = find_gvar(tok);
			node->type = calloc(1,sizeof(Type));
                        if (lvar) {
                                buf2->offset = lvar->offset;
                                buf2->type = calloc(1,sizeof(Type));
                                buf2->type = lvar->type;
				buf1 = new_node_num(index);
                         	int type_size = 8;
                         	buf1 = new_node(ND_MUL, buf1, new_node_num(type_size));
                         	buf2 = new_node(ND_ADD, buf2, buf1);
                         	node->kind = ND_DEREF;
                         	node->lhs = buf2;
                         	node->type = node->lhs->type->ptr_to;
				node->str = calloc(1,sizeof(char));
                                node->str = lvar->name;
			} else if (gvar) {
				// TODO: グローバル変数の処理
				// グローバル変数の値は書き換え不可らしい
				node->kind = ND_GVAR;
				node->str = calloc(1,sizeof(char));
				node->str = gvar->name;
				node->type = gvar->type;
				if(gvar->type->ptr_to->ty == INT)node->memory = index * 4;
				else if(gvar->type->ty == CHAR)node->memory = index;
				else node->memory = index * 8;
                        } else {
				
                                error("配列 %s が定義されていません", tok->str);
                        }
			if (!consume(RLB)){
                                error_at(token->str, "']'ではないトークンです");
                        }
		} else {
			
			LVar *lvar = find_lvar(tok);
			GVar *gvar = find_gvar(tok);
			LC *lc = find_lc(tok);
			node->type = calloc(1,sizeof(Type));
			if (lc) {
				node->kind = ND_GLC;
				//	node->kind = ND_GVAR;
                                node->str = calloc(1,sizeof(char));
                                node->str = lc->name;
			} else if (lvar) {
				node->kind = ND_LVAR;
				node->offset = lvar->offset;
				node->type = lvar->type;
				node->str = calloc(1,sizeof(char));
				node->str = lvar->name;
			} else if (gvar) {
				// TODO: グローバル変数の処理
				// グローバル変数の値は書き換え不可らしい
				node->kind = ND_GVAR;
                                node->str = calloc(1,sizeof(char));
                                node->str = gvar->name;
				node->type = gvar->type;
                                if(gvar->type->ty == INT)node->memory = 4;
				else if(gvar->type->ty == CHAR)node->memory = 1;
                                else node->memory = 8;
			} else {
				error("変数 %s が定義されていません", tok->str);
			}
		}
		return node;
	}

        // そうでなければ数値のはず
	node = new_node_num(expect_number());
        return node;
}

Node *unary() {
	Node *node = calloc(1,sizeof(Node));
	if (consume_tk(TK_SIZEOF)) {
		node = unary();
		if(node->type->ty == PTR){
			return new_node_num(8);
		} else {
			return new_node_num(4);
		}
	}
        if (consume(ADD)){
		node = primary();
                return node;
	}
        if (consume(SUB)){
		node = new_node(ND_SUB, new_node_num(0), primary());
                return node;
	}
	node = primary();
        return node;
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
		// 行コメントをスキップ
    		if (strncmp(user_input, "//", 2) == 0) {
      			user_input += 2;
      			while (*user_input != '\n')
        		user_input++;
      			continue;
    		}

    		// ブロックコメントをスキップ
    		if (strncmp(user_input, "/*", 2) == 0) {
      			char *q = strstr(user_input + 2, "*/");
      			if (!q)
        			error_at(user_input, "コメントが閉じられていません");
      			user_input = q + 2;
      			continue;
    		}

		if (*user_input == *DQ) {
			int len = 0;
			int dq_cnt = 0;
			char *ptr = calloc(1,sizeof(char));
			ptr = user_input;
			while(dq_cnt < 2){
				if( *ptr == *DQ){
					dq_cnt++;
				}
				ptr++;
				len++;
			}
			cur = new_token(TK_LC, cur, user_input, len);
			continue;
		}

		if (strlen(user_input) >= 7 && strncmp(user_input, "sizeof", 6) == 0 && !is_alnum(user_input[6])) {
			cur = new_token(TK_SIZEOF, cur, user_input, 6);
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

		if(strlen(user_input) >= 4 && strncmp(user_input, "int", 3) == 0 && !is_alnum(user_input[3])) {
			cur = new_token(TK_INT, cur, user_input, 3);
			continue;
		}

		if (strlen(user_input) >= 5 && strncmp(user_input, "char", 4) == 0 && !is_alnum(user_input[4])) {
                        cur = new_token(TK_CHAR, cur, user_input, 4);
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
			       	|| *user_input == *LB || *user_input == *RB || *user_input == *LMB || *user_input == *RMB 
				|| *user_input == *COM || *user_input == *ADDR || *user_input == *DEREF || *user_input == *LLB
				|| *user_input == *RLB) {
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

                error_at(user_input, "トークナイズできません");
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

GVar *find_gvar(Token *tok) {
        for (GVar *var = globals; var; var = var->next)
                if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
                        return var;
        return NULL;
}

FVar *find_fvar(Token *tok) {
        for (FVar *var = functions; var; var = var->next)
                if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
                        return var;
        return NULL;
}

LC *find_lc(Token *tok) {
	for (LC *lc = lcs; lc; lc = lc->next)
		if (lc->len == tok->len && !memcmp(tok->str, lc->name, lc->len))
			return lc;
	return NULL;
}

int is_alnum(char c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
}
