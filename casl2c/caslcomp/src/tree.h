/******************************************************

  CASL2C Compiler for COMET-II on FPGA
  header file for syntax trees used in the compiler

  Taro Suzuki
  Last updated: Mar 25, 2015

******************************************************/

#ifndef __CASL2C_TREE_H__
#define __CASL2C_TREE_H__

#include "const.h"
#include "type.h"
#include "list.h"

// 可変長引数を持つ関数 newSTree を型安全に定義するためのマクロ
#define newSTree(id, ...)				\
  _newSTree(id,						\
    (Tree*[]){ __VA_ARGS__ },				\
    sizeof((Tree*[]){ __VA_ARGS__ }) / sizeof(Tree*)	\
  )

// 可変長引数を持つ関数 newETree を型安全に定義するためのマクロ
#define newETree(id, ...)				\
  _newETree(id,						\
    (Tree*[]){ __VA_ARGS__ },				\
    sizeof((Tree*[]){ __VA_ARGS__ }) / sizeof(Tree*)	\
  )

// tree.c で定義されている関数のプロトタイプ宣言
Tree* newVarNode(VarEntry* entry);
Tree* newHandlerNode(ProcEntry* entry);
Tree* newNumNode(int num);
Tree* newStrNode(char* str);
Tree* newIdxTree(Tree* base, Tree* index);
Tree* newArrayNode(PList* array, int size);
Tree* newSeqTree(PList* seq);
Tree* newProcTree(ProcEntry* proc, Tree* body);
Tree* _newETree(ETreeID id, Tree* child[], size_t size);
Tree* _newSTree(STreeID id, Tree* child[], size_t size);
Tree* shrinkUnaryExp(Tree* tree);
void deleteTree(Tree* tree);
void deleteOnlySeq(SeqTree* seq);
void deleteNothing(void* tree);
int seqlen(Tree* tree);

#endif
