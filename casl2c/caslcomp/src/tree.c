/****************************************************

  CASL2C Compiler for COMET-II on FPGA
  funtion definitions for handling syntax trees

  Taro Suzuki
  Last updated: Mar 25, 2015

****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "type.h"
#include "tree.h"
#include "list.h"
#include "symtable.h"
#include "error.h"

// static関数のプロトタイプ宣言
static void deleteVarNode(Tree* p);
static void deleteAvarNode(Tree* p);
static void deleteNumNode(Tree* p);
static void deleteIdxTree(Tree* p);
static void deleteStrNode(Tree* p);
static void deleteArrayNode(Tree* p);
static void deleteSeqTree(Tree* p);
static void deleteETree(Tree* p);
static void deleteSTree(Tree* p);


/**************/
/* 構文木の生成 */
/**************/

// 変数の構文木の生成
Tree* newVarNode(VarEntry* entry)
{
  VarNode* p;

  if ((p=malloc(sizeof(VarNode))) == NULL)
    errorExit(EShortOfMemory);
  p->id = isArray(entry) ? Savar : Svar;
  p->entry = entry;
  return (Tree*)p;
}

// 割り込みハンドラ名の構文木の生成
Tree* newHandlerNode(ProcEntry* entry)
{
  HandlerNode* p;

  if ((p=malloc(sizeof(HandlerNode))) == NULL)
    errorExit(EShortOfMemory);
  p->id = Shandler;
  p->entry = entry;
  return (Tree*)p;
}

// 数の構文木の生成
Tree* newNumNode(int n)
{
  NumNode* p;

  if ((p=malloc(sizeof(NumNode))) == NULL)
    errorExit(EShortOfMemory);
  p->id = Snum;
  p->value = n;
  return (Tree*)p;
}

// 文字列の構文木の生成
Tree* newStrNode(char* str)
{
  StrNode* p;

  if ((p=malloc(sizeof(StrNode))) == NULL)
    errorExit(EShortOfMemory);
  p->id = Sstr;
  p->str = str;
  return (Tree*)p;
}

// 添字式の構文木の生成
Tree* newIdxTree(Tree* base, Tree* index)
{
  IdxTree* p;

  if ((p=malloc(sizeof(IdxTree))) == NULL)
    errorExit(EShortOfMemory);
  p->id = Sidx;
  p->base = base;
  p->index = index;
  return (Tree*)p;
}

// 配列の構文木の生成
Tree* newArrayNode(PList* array, int size)
{
  ArrayNode* p;

  if ((p=malloc(sizeof(ArrayNode))) == NULL)
    errorExit(EShortOfMemory);
  p->id = Sarray;
  p->list = array;
  p->size = size;
  return (Tree*)p;
}

// 列の構文木の生成
Tree* newSeqTree(PList* seq)
{
  SeqTree* p;

  if ((p=malloc(sizeof(SeqTree))) == NULL)
    errorExit(EShortOfMemory);
  p->id = Sseq;
  p->seq = seq;
  return (Tree*)p;
}

// 複合式の構文木の生成
Tree* _newETree(ETreeID id, Tree* child[], size_t size)
{
  ETree* p;
  int i;
  size_t memsize = sizeof(ETree) + (size > 0 ? sizeof(Tree*)*(size-1) : 0);

  if ((p=malloc(memsize)) == NULL)
    errorExit(EShortOfMemory);
  p->id = Sexp;
  p->eid = id;
  p->size = size;
  for (i=0; i<size; i++)
    p->child[i] = child[i];
  return (Tree*)p;
}

// 式以外の文の構文木の生成
Tree* _newSTree(STreeID id, Tree* child[], size_t size)
{
  STree* p;
  int i;
  size_t memsize = sizeof(STree) + (size > 0 ? sizeof(Tree*)*(size-1) : 0);

  if ((p=malloc(memsize)) == NULL)
    errorExit(EShortOfMemory);
  p->id = id;
  p->size = size;
  for (i=0; i<size; i++)
    p->child[i] = child[i];
  return (Tree*)p;
}

// 手続きまたは割り込みハンドラの構文木の生成
Tree* newProcTree(ProcEntry* entry, Tree* body)
{
  ProcTree* p;

  // 手続きエントリを調べ、割り込みハンドラのエントリなら
  // 割り込みハンドラの構文木にする
  if ((p=malloc(sizeof(ProcTree))) == NULL)
    errorExit(EShortOfMemory);
  p->id = isHandler(entry) ? Sintr : Sfunc;
  p->entry = entry;
  p->body = body;
  return (Tree*)p;
}

/**************/
/* 構文木の破棄 */
/**************/

// 変数の構文木の破棄
// entry は symtable.c により破棄されるので、ここでは破棄しない
static void deleteVarNode(Tree* p)
{
  // 変数エントリは破棄しない
  free(p);
}

static void deleteHandlerNode(Tree* p)
{
  // 手続きエントリは破棄しない
  free(p);
}

// 数の構文木の破棄
static void deleteNumNode(Tree* p)
{
  free(p);
}

// 配列要素の構文木の破棄
static void deleteIdxTree(Tree* p)
{
  deleteTree(p->idx.base);
  deleteTree(p->idx.index);
  free(p);
}

// 文字列の構文木の破棄
static void deleteStrNode(Tree* p)
{
  free(p);
  // 文字列本体は破棄しない *** なぜ？？？ '15.12.13 ***
}

// 配列の構文木の破棄
static void deleteArrayNode(Tree* p)
{
  //  deleteSeqTree((Tree*)p->array.list);
  deletePList(p->array.list, (void(*)(void*))deleteTree);
  free(p);
}

// 構文木の列の破棄
static void deleteSeqTree(Tree* p)
{
  //  deleteTree(p->seq.tree);
  //  deleteTree(p->seq.next);
  deletePList(p->seq.seq, (void(*)(void*))deleteTree);
  free(p);
}

// ダミーの破棄関数（構文木の破棄をしない）
void deleteNothing(void* tree)
{
}

// 複合式の構文木の破棄
static void deleteETree(Tree* p)
{
  int i;

  switch (p->etree.eid)  {
  case Ecall:
    deleteTree(p->etree.child[1]);
    deletePList((PList*)p->etree.child[2], (void(*)(void*))deleteTree);
    break;
  case Ecomma:
    deletePList((PList*)p->etree.child[0], (void(*)(void*))deleteTree);
    break;
  default:
    for (i=0; i<p->etree.size; i++)
      deleteTree(p->etree.child[i]);
  }
  free(p);
}

// 式でない文の構文木の破棄
static void deleteSTree(Tree* p)
{
  int i;
  for (i=0; i<p->stree.size; i++)  {
    deleteTree(p->stree.child[i]);
  }
  free(p);
}

// 手続きの構文木の破棄
static void deleteProcTree(Tree* p)
{
  // 手続きエントリは破棄しない
  deleteTree(p->ptree.body);
  free(p);
}

// 構文木の破棄
void deleteTree(Tree* p)
{
  if (p == NULL) return;

  switch (p->id)  {
  case Svar:
  case Savar:
    deleteVarNode(p);
    break;
  case Shandler:
    deleteHandlerNode(p);
    break;
  case Snum:
    deleteNumNode(p);
    break;
  case Sidx:
    deleteIdxTree(p);
    break;
  case Sarray:
    deleteArrayNode(p);
    break;
  case Sstr:
    deleteStrNode(p);
    break;
  case Sseq:
    deleteSeqTree(p);
    break;
  case Sexp:
    deleteETree(p);
    break;
  case Sfunc:
  case Sintr:
    deleteProcTree(p);
    break;
  default:
    deleteSTree(p);
  }
}

/************************/
/* その他ユーティリティ関数 */
/************************/

Tree* shrinkUnaryExp(Tree* tree)
{
  Tree* sub = tree->etree.child[0];
  free(tree);
  return sub;
}

