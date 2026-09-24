/****************************************************

  CASL2C Compiler for COMET-II on FPGA
  funtion definitions for list handling

  Taro Suzuki 
  Last updated: Mar. 9, 2016

****************************************************/

#include <stdlib.h>
#include <stddef.h>
#include "list.h"
#include "error.h"

// 整数のリストのためのノードを生成する
IList* newIList(int data, IList* next)
{
  IList* p;
  if ((p=malloc(sizeof(IList))) == NULL)
    errorExit(EShortOfMemory);
  p->data = data;
  p->next = next;
  return p;
}

// 整数のリストを破棄する
void deleteIList(IList *ilist)
{
  while (ilist)  {
    IList *current = ilist;
    ilist = ilist->next;
    free(current);
  }
}

// ポインタのリストのためのノードを生成する
PList* newPList(void* data, PList* next)
{
  PList* p;
  if ((p=malloc(sizeof(PList))) == NULL)
    errorExit(EShortOfMemory);
  p->data = data;
  p->next = next;
  return p;
}

// ポインタのリストを破棄する
void deletePList(PList *plist, void (*deleteData)(void*))
{
  while (plist)  {
    PList* current = plist;
    deleteData(plist->data);
    plist = plist->next;
    free(current);
  }
}

// 末尾に追加可能なリストを生成する
XList* newXList()
{
  XList* p;

  if ((p=malloc(sizeof(XList))) == NULL) errorExit(EShortOfMemory);
  return clearXList(p);
}

// 末尾に追加可能なリストの内容を初期化する
XList* clearXList(XList* p)
{
  p->list = NULL;
  p->length = 0;
  p->end = &(p->list);
  return p;
}

// 末尾に追加可能なリストを破棄する
void deleteXList(XList* xlist)
{
  // リスト本体は破棄しない
  free(xlist);
}

// 末尾に追加可能なリストの末尾にデータを追加する
XList* _addLast(XList* xlist, void* cell, void** nextp)
{
  *(xlist->end) = cell;
  xlist->length += 1;
  xlist->end = nextp;
  return xlist;
}

// 破壊的に２つのリストを結合する
XList* appendXList(XList *l1, XList *l2)
{
  // l1 のリストの末尾に l2 のリストが連結される
  *(l1->end) = l2->list;
  l1->length += l2->length;
  // l2 が空でないときだけ、l1->endがl2の末尾を指すように変更
  if (l2->length != 0)
    l1->end = l2->end;
  return l1;	// 変更された l1 が返る
}

// 末尾に追加可能なリストからリスト本体を取り出す
void* getXListBody(XList* xlist)
{
  void *list = xlist->list;
  return list;
}

// 末尾に追加可能なリストの長さを得る
int getXListLen(XList *xlist)
{
  return xlist->length;
}


