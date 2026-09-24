/******************************************************

  CASL2C Compiler for COMET-II on FPGA
  header file for specific types for the compiler

  Taro Suzuki
  Last updated: Mar 25, 2016

******************************************************/

#ifndef __CASL2C_TYPE_H__
#define __CASL2C_TYPE_H__

#include "const.h"
#include "list.h"


// 構文木の型を前方参照するための不完全型の定義
union _tree;

/******************/
/* 記号表エントリ */
/*****************/


// 変数エントリ
typedef struct {
  SymEntryID id;	// つねに VarEntryID
  char* name;		// 変数名
  VClass class;		// 変数の種別
  int index;		// 局所変数のインデックスか、大域変数の語数
  unsigned short flag;	// フラグ
  union _tree* init;	// 初期値
} VarEntry;

// 手続きエントリ
typedef struct {
  SymEntryID id;	// つねに ProcEntryID
  char* name;		// 手続き名
  int paramNum;		// 仮引数個数
  unsigned short flag;	// フラグ
  short systemID;	// 組込関数のID
} ProcEntry;

// 記号表エントリ
typedef union {
  SymEntryID id;	// 記号表エントリの種別
  VarEntry var;		// 変数エントリ
  ProcEntry proc;	// 手続きエントリ
} SymbolEntry;

/**********/
/* 構文木 */
/**********/

// 構文木の列の構文木
typedef struct _seq {
  STreeID id;			// つねに SSeq
  //  union _tree* tree;		// 構文木
  //  union _tree* next;		// 次の要素へのポインタ
  PList* seq;			// 構文木の列の実体
} SeqTree;

// 変数の構文木
typedef struct {
  STreeID id;			// Svar か Savar
  VarEntry* entry;		// 変数エントリ
} VarNode;

// 割り込みハンドラ名の構文木
typedef struct {
  STreeID id;			// つねに Shandler
  ProcEntry* entry;		// 手続きエントリ
} HandlerNode;

// 数の構文木
typedef struct {
  STreeID id;			// つねに Snum
  int value;			// 数本体
} NumNode;

// 文字列の構文木
typedef struct {
  STreeID id;			// つねに Sstr
  char* str;			// 文字列本体
} StrNode;

// 配列の構文木
typedef struct {
  STreeID id;			// つねに Sarray
  PList* list;			// 配列要素のリスト
  //  SeqTree* list;		// 配列要素のリスト
  int size;			// 配列のサイズ
} ArrayNode;

// 添字式の構文木
typedef struct {
  STreeID id;			// つねに Sidx
  union _tree* base;		// ベースとなる式
  union _tree* index;		// 添字の式
} IdxTree;

// 複合式の構文木
typedef struct _etree {
  STreeID id;			// つねに Sexp
  ETreeID eid;			// 式の構文木の種別
  size_t size;			// 引数個数
  union _tree* child[1];	// 引数
} ETree;

// 式以外の文の構文木
typedef struct _stree {
  STreeID id;			// 構文木の種別
  size_t size;			// 引数個数
  union _tree* child[1];	// 引数
} STree;

// 手続きの構文木
typedef struct {
  STreeID id;			// Sfunc か Sintr
  ProcEntry* entry;		// 手続きエントリ
  union _tree* body;		// 手続き本体
} ProcTree;

// 構文木
typedef union _tree {
  STreeID id;			// 構文木の種別
  VarNode var;			// 変数
  HandlerNode handler;		// 割り込みハンドラ名
  NumNode num;			// 数
  StrNode str;			// 文字列
  ArrayNode array;		// 配列
  IdxTree idx;			// 添字式
  SeqTree seq;			// 列
  STree stree;			// 式以外
  ETree etree;			// 複合式
  ProcTree ptree;		// 手続き
} Tree;

#endif
