/****************************************************

  CASL2C Compiler for COMET-II on FPGA
  funtion definitions for symbol tables

  Taro Suzuki 
  Last updated: Mar 11, 2015

****************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "const.h"
#include "type.h"
#include "error.h"
#include "symtable.h"
#include "scanner.h"
#include "gencode.h"
#include "tree.h"
#include "list.h"

#define UnknownParamNum		-1

// 記号表の型
typedef XList SymbolTable;

// 記号表が持つキー(識別子名)と値(記号表エントリ)のペアの型
typedef struct  {
  char* key;
  SymbolEntry* val;
} KV;

// static 関数のプロトタイプ宣言
static VarEntry* addVar(char* name, VClass vc, int index, Tree* init,
                        int flag, SymbolTable* table);
static ProcEntry* addProcEntry(char* name, int paramc);
static void addEntry(char* name, SymbolEntry* entry, SymbolTable* table);
static SymbolEntry* find(char* name, SymbolTable* table);
static void clearSymbolTable(SymbolTable* table);
static void checkParamNum(ProcEntry* proc, int paramc);


static SymbolTable* globalSymTable;	// 大域的な記号表
static SymbolTable* localSymTable;	// 局所的な記号表
static char* mainLab;			// 関数main()のラベル

static int lvarIndex;		// 局所変数のインデックス
static int paramIndex;		// 仮引数のインデックス
static int lvarNum;		// 局所変数の個数
static int paramNum;		// 仮引数の個数

// プログラムのコンパイル終了後に、宣言済のユーザ手続きが
// すべて定義されたかどうかを調べる関数
// 宣言されたのに定義されていない関数があったらエラーを報告する
void checkProgram()
{
  PList *p;

  // 大域的な記号表のエントリを順に調べる
  for (p=getXListBody(globalSymTable); p != NULL; p=p->next)  {
    SymbolEntry *entry = ((KV*)p->data)->val;
    if (entry->id == ProcEntryID)  {
      if ((entry->proc.flag & PEDefined) == 0)
        // 手続きエントリが未定義ならエラーを報告する
        compileError(EProcNotDefined,entry->proc.name);
      if ((entry->proc.flag & (PESystem|PEUsed)) == (PESystem|PEUsed))
        emitSystemProcedure(entry->proc.systemID);
    }
  }
}

// 記号表に登録されたキーと記号表エントリのペアの解放
static void deleteEntryKeyValue(KV *pair)
{
  // キーと記号表エントリが持つ名前は同じものなので、キーは解放しない
  // 変数エントリだけを解放(局所的な記号表には変数エントリしかない)
  SymbolEntry* entry = pair->val;
  if (entry->id == VarEntryID)  {
    free(entry->var.name);
    deleteTree(entry->var.init);
    free(entry);
  }
  else  {
    if (strcmp(mainLab,entry->proc.name) && !(entry->proc.flag & PESystem))
      free(entry->proc.name);	// 手続き名の解放(main以外のユーザ手続き)
  }
  // キーと記号表エントリのペアを解放
  free(pair);
}

void cleanProcDef()
{
  // 局所的な記号表をクリアする
  clearLocalSymTable();
}

// 大域的な記号表のすべてのエントリの解放
void clearGlobalSymTable()
{
  clearSymbolTable(globalSymTable);
}

// 局所的な記号表のすべてのエントリの解放
void clearLocalSymTable()
{
  // すべての記号表エントリを解放
  clearSymbolTable(localSymTable);
}

// 記号表のすべてのエントリの解放
static void clearSymbolTable(SymbolTable* table)
{
  PList* p = getXListBody(table);
  // 記号表のすべてのエントリの解放
  deletePList(p,(void(*)(void*))deleteEntryKeyValue);
  // 記号表の初期化
  clearXList(table);
}

ProcEntry* addSystemProc(char* name, int paramc, int id)
{
  ProcEntry* proc = addProcEntry(name,paramc);
  proc->flag |= (PESystem | PEDefined);
  proc->systemID = id;
  return proc;
}


// 記号表の初期化を行う関数。
// 大域的な記号表に、あらかじめmain()を登録しておく。
// こうすることで、main()はユーザが宣言する必要はなくなる。
// （定義はユーザがしなければならない）
void initSymTables()
{
  mainLab = generateNameLabel("main");
  globalSymTable = newXList();
  localSymTable = newXList();
  addProcEntry(mainLab,0);	// main()を登録
  initSystemProcedures();	// システム関数のエントリを登録
}

// 現在コンパイル中の手続きの局所変数の個数を返す
int getLocalVarNum()
{
  return lvarNum;
}

// 現在コンパイル中の手続きの仮引数の個数を返す
int getParamNum()
{
  return paramNum;
}



// 識別子名nameと変数の種別vcをもつ変数エントリを生成し、
// tableが指す記号表にその変数エントリを登録する
// 登録した変数エントリへのポインタを返す
static VarEntry* addVar(char* name, VClass vc, int index, Tree* init,
                        int flag, SymbolTable* table)
{
  // 同じ名前の識別子が与えられた記号表に存在するかチェック、あればエラー
  VarEntry* var;
  SymbolEntry* symbol = find(name,table);
  if (symbol != NULL)  {
    if (symbol->id == VarEntryID)
      compileError(EVarDuplicated,name);
    else
      compileError(EAlreadyAsProc,name);
  }
  // なければ変数エントリを生成し、与えられた記号表に登録
  if ((var=malloc(sizeof(VarEntry))) == NULL) errorExit(EShortOfMemory);
  var->id = VarEntryID;
  var->name = name;
  var->class = vc;
  var->index = index;
  var->init = init;
  var->flag = flag;
  addEntry(name,(SymbolEntry*)var,table);
  return var;
}

static ProcEntry* addProcEntry(char* name, int paramc)
{
  ProcEntry* proc;
  // 手続きエントリを生成
  if ((proc=malloc(sizeof(ProcEntry))) == NULL) errorExit(EShortOfMemory);
  proc->id = ProcEntryID;
  proc->name = name;
  proc->paramNum = paramc;
  proc->flag = 0;
  // 大域的な記号表に手続きエントリを登録
  addEntry(name,(SymbolEntry*)proc,globalSymTable);
  return proc;
}

// 記号表エントリを指定された記号表に登録
static void addEntry(char *name, SymbolEntry* entry, SymbolTable* table)
{
  KV* pair;
  PList* cell;

  // 識別子名と記号表エントリのペアを生成
  if ((pair=malloc(sizeof(KV))) == NULL) errorExit(EShortOfMemory);
  pair->key = name;
  pair->val = entry;
  // 識別子名と記号表エントリのペアを指定された記号表に登録
  if ((cell=malloc(sizeof(PList))) == NULL) errorExit(EShortOfMemory);
  cell->data = pair;
  cell->next = NULL;
  addLast(table,cell);
}

// 大域変数のための変数エントリを生成し、大域的な記号表に登録する
// 登録した変数エントリへのポインタを返す
VarEntry* addGlobalVar(char* name, Tree* init)
{
  // 大域変数の場合、addVarの第3引数はデータのサイズである1を指定
  return addVar(name,GlobalVar,1,init,VENoFlag,globalSymTable);
}

// 配列のための変数エントリを生成し、大域的な記号表に登録する
// 登録した変数エントリへのポインタを返す
VarEntry* addArray(char* name, unsigned int size, Tree* init)
{
  if (size == 0)  {
    if (init == NULL)  {
      // サイズ指定も初期値もなければ、サイズ不明なのでエラー
      compileError(EArraySizeUndefined,name);
      return NULL;
    }
    else  {
      // 初期値から配列のサイズを取得
      if (init->id == Sstr)  {
        // 初期値が文字列ならその長さを計る(末尾に加えるNULLも数える)
        size = strlen(init->str.str) + 1;
      }
      else  {
        // 初期値が配列なら配列の構文木からサイズを取得
        size = init->array.size;
      }
    }
  }
  else  {
    int isize;
    // 初期値のサイズを取得
    if (init == NULL)
      isize = 0;
    else if (init->id == Sstr)  {
      // 初期値が文字列ならその長さを計る
      isize = strlen(init->str.str);
    }
    else  {
      // 初期値が配列なら配列の構文木からサイズを取得
      isize = init->array.size;
    }
    if (size < isize)  {
      compileError(ETooLongInitArray,name,size,isize);
      return NULL;
    }
  }
  // 配列のときは、addVarの第3引数に配列のサイズを指定
  return addVar(name,GlobalVar,size,init,VEArray,globalSymTable);
}

// 局所変数のための変数エントリを生成し、局所的な記号表に登録する。
// 登録した変数エントリへのポインタを返す。
VarEntry* addLocalVar(char* name)
{
  ++lvarNum;
  return addVar(name,LocalVar,lvarIndex++,NULL,VENoFlag,localSymTable);
}

// 仮引数のための変数エントリを生成し、局所的な記号表に登録する。
// 登録した変数エントリへのポインタを返す。
VarEntry* addParam(char* name)
{
  ++paramNum;
  if (paramNum <= MaxArgRegNum)  {
    // レジスタに置く仮引数(最小のインデックスは1)
    VarEntry* entry =
      addVar(name,Param,++paramIndex,NULL,VERegister,localSymTable);
    assocParamEntry(paramIndex,entry);
    return entry;
  }
  else  {
    // スタックに置く仮引数(最小のインデックスは0)
    if (paramNum > MaxArgRegNum) paramIndex = 0;
    return addVar(name,Param,paramIndex++,NULL,VENoFlag,localSymTable);
  }
}

ProcEntry* addProc(char* name, int paramc)
{
  ProcEntry* proc = (ProcEntry*)find(name,globalSymTable);

  // 名前nameをもつ識別子が大域的な記号表に登録済ならエラー
  if (proc != NULL)  {
    if (proc->id == ProcEntryID)  {
      compileError(EProcDuplicated,name);
      return NULL;
    }
    if (proc->id == VarEntryID)  {
      compileError(EAlreadyAsVar,name);
      return NULL;
    }
  }
  // 手続きエントリを生成し、大域的な記号表に登録
  proc = addProcEntry(name,paramc);
  return proc;
}

void initProcDef()
{
  int i;
  // 局所変数と仮引数のインデックスを保持する変数を初期化する
  lvarNum = paramNum = lvarIndex = paramIndex = 0;
  // レジスタに置く仮引数の変数エントリを初期化する
  for (i=0; i<MaxArgRegNum; i++)
    assocParamEntry(i, NULL);
}

ProcEntry* defineProc(char* name, int paramc, bool intr)
{
  // 大域的な記号表から識別子nameをもつ記号表エントリを探す
  ProcEntry* proc = (ProcEntry*)find(name,globalSymTable);

  // 記号表にエントリがなければ手続きエントリを登録
  if (proc == NULL)
    proc = addProcEntry(name,paramc);
  // 記号表エントリが変数ならエラー
  else  {
    if (proc->id == VarEntryID)  {
      compileError(EAlreadyAsVar,name);
      return NULL;
    }
    // 手続きが定義済みならエラー
    if (proc->flag & PEDefined)  {
      compileError(EProcDefDuplicated,name);
      return NULL;
    }
    // 引数個数が宣言時と一致しなければエラー
    checkParamNum(proc,paramc);
  }
  // 手続きを定義済にする
  proc->flag |= PEDefined;
  // 割り込みハンドラなら、フラグをセット
  if (intr) proc->flag |= PEhandler;
  return proc;
}

// 識別子名nameをもつ変数エントリを記号表から探し、
// 見つかったらその変数エントリへのポインタを返す
// 見つからなければエラー
VarEntry* findVar(char* name)
{
  // 局所的な記号表から識別子nameをもつ記号表エントリを探す
  SymbolEntry* var = find(name,localSymTable);

  // 局所的な記号表に識別子nameをもつ記号表エントリがなければ、
  // 大域的な記号表から識別子nameをもつ記号表エントリを探す
  if (var == NULL)
    var = find(name,globalSymTable);

  // 記号表エントリが見つからなければエラー
  if (var == NULL)
    compileError(EVarNotFound,name);
  // 手続きエントリが見つかったらエラー
  else if (var->id != VarEntryID)
    compileError(EVarDeclaredAsProc,name);
  // 配列が見つかったらエラー
  //  else if (isArray(var))
  //    compileError(EVarDeclaredAsArray,name);

  // 見つかった変数エントリへのポインタを返す
  return (VarEntry*)var;
}

// 識別子名nameをもつ配列変数エントリを記号表から探し、
// 見つかったらその配列変数エントリへのポインタを返す
// 見つからなければエラー
VarEntry* findArray(char* name)
{
  // 局所的な記号表から識別子nameをもつ記号表エントリを探す
  if (find(name,localSymTable))  {
    compileError(EArrayDeclaredAsVar,name);
    return NULL;
  }
  // 大域的な記号表から識別子nameをもつ記号表エントリを探す
  SymbolEntry* array = find(name,globalSymTable);
  // 大域的な記号表エントリが見つからなければエラー
  if (array == NULL)
    compileError(EVarNotFound,name);
  // 手続きエントリが見つかったらエラー
  else if (array->id != VarEntryID)
    compileError(EVarDeclaredAsProc,name);
  // 配列でなければエラー
  else if (!isArray((VarEntry*)array))
    compileError(EArrayDeclaredAsVar,name);

  // 見つかった配列変数エントリへのポインタを返す
  return (VarEntry*)array;
}

// 大域的な記号表から手続きエントリを探す
// 見つからなければ手続きエントリを生成し登録
ProcEntry* findProc(char* name, int paramc)
{
  // 局所的な記号表から識別子nameをもつ記号表エントリを探す。
  if (find(name,localSymTable))  {
    // 局所的な記号表にエントリがあればエラー
    compileError(EProcDeclaredAsVar,name);
    return NULL;
  }
  // 大域的な記号表から識別子nameをもつ記号表エントリを探す
  ProcEntry* proc = (ProcEntry*)find(name,globalSymTable);
  // 大域的な記号表にエントリがなければエラー
  if (proc == NULL)  {
    compileError(EProcNotFound,name);
    return NULL;
  }
  // 変数エントリが見つかったらエラー
  if (proc->id != ProcEntryID)  {
    // 変数として宣言されている
    compileError(EProcDeclaredAsVar,name);
    return NULL;
  }
  // 割り込みハンドラを呼び出したらエラー
  if (isHandler(proc))  {
    compileError(ECallHandler,name);
    return NULL;
  }
  // 引数個数が一致しなければエラー
  checkParamNum(proc,paramc);

  // 手続きが使用されたことを手続きエントリに記録
  proc->flag |= PEUsed;
  // 手続きエントリへのポインタを返す
  return proc;
}

// 引数個数が手続きエントリ中の引数個数と一致するかチェック
static void checkParamNum(ProcEntry* proc, int paramc)
{
 if (proc->paramNum != paramc)
    // 引数個数が不一致
    compileError(EParamNumMismatch,proc->name,paramc,proc->paramNum);
}

void markVariable(VarEntry* entry, unsigned int flag)
{
  entry->flag |= flag;
}

struct _symit {
  PList* current;
};

SymTableIterator initSymtableIterator()
{
  static struct _symit itBody;
  itBody.current = getXListBody(globalSymTable);
  return &itBody;
}

SymbolEntry* getNextSymbolEntry(SymTableIterator it)
{
  PList* p = it->current;
  if (p)  {
    it->current = p->next;
    return ((KV*)(p->data))->val;
  }
  else
    return NULL;
}


// 識別子名nameをもつ記号表エントリを、tableが指す記号表から探す
// 見つかったらその記号表エントリへのポインタを、見つからなければ
// NULLを返す
static SymbolEntry *find(char *name, SymbolTable *table)
{
  PList* p = getXListBody(table);

  while (p)  {
    KV* pair = p->data;
    if (!strcmp(pair->key,name))
      return pair->val;
    p = p->next;
  }
  return NULL;
}

// 識別子名nameをもつ記号表エントリを、tableが指す記号表から探す
// 見つかったらその記号表エントリへのポインタを、見つからなければ
// NULLを返す
SymbolEntry *findGlobalSymbol(char *name)
{
  return find(name,globalSymTable);
}
