/****************************************************

  CASL2C Compiler for COMET-II on FPGA
  header file for list handling

  Taro Suzuki 
  Last updated: Mar. 9, 2016

****************************************************/

#ifndef __CASL2C_UTIL_H__
#define __CASL2C_UTIL_H__

#define addLast(xl,c)  _addLast(xl,c,(void **)&((c)->next))

// 整数のリスト
typedef struct _ilist {
  int data;			// リストの要素
  struct _ilist* next;		// 次の要素へのポインタ
} IList;

// ポインタのリスト
typedef struct _plist {
  void* data;			// リストの要素
  struct _plist* next;		// 次の要素へのポインタ
} PList;

// 末尾に追加可能なリスト
typedef struct {
  void* list;	// リスト本体
  int length;   // リスト中の要素の個数
  void** end;	// リストの末尾を指すポインタ
} XList;

// list.c で定義されている関数のプロトタイプ宣言
IList* newIList(int data, IList* next);
void deleteIList(IList* ilist);
PList* newPList(void* data, PList* next);
void deletePList(PList* plist, void (*deleteData)(void*));
XList* newXList(void);
XList* clearXList(XList* p);
void deleteXList(XList* xlist);
XList* _addLast(XList* xlist, void* cell, void** nextp);
XList* appendXList(XList* l1, XList* l2);
void* getXListBody(XList* xlist);
int getXListLen(XList* xlist);

#endif
