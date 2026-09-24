/************************************************

  CASL2C Compiler for COMET-II on FPGA
  function definitions for COMET-II code generation

  Taro Suzuki 
  Last updated: Mar. 25, 2016

************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gencode.h"
#include "const.h"
#include "type.h"
#include "tree.h"
#include "list.h"
#include "error.h"
#include "symtable.h"

// レジスタ情報の型
typedef struct {
  int usedHead;		// 使用済みレジスタ群の先頭
  int availableHead;	// 使用可能なレジスタ群の先頭
} RegInfo;

typedef enum { INC, DEC } ModifyOp;
typedef enum { PRE, POST } ModifyOrder;

#define varName(e)	(e)->var.entry->name
#define streeArgNum(t)	(t)->stree.size

// r の次にあたる一時変数レジスタ番号を返す
#define nextTmpReg(r) ((r-MaxArgRegNum)%(MaxReg-MaxArgRegNum) + MaxArgRegNum+1)
// r のひとつ前にあたる一時変数レジスタ番号を返す
#define prevTmpReg(r) ((r+MaxReg-2*MaxArgRegNum-2)%(MaxReg-MaxArgRegNum) + MaxArgRegNum+1)
// r が一時変数レジスタの番号なら真(最大レジスタ番号を超えるかはチェックしない)
#define isTmpReg(r) (MaxArgRegNum < r)

// ラベルを生成するマクロ
#define ILabelHeader			"@L"
#define DLabelHeader			"@D"
#define emitInstLabel(lab)		printf("%s\n",lab)
#define emitDataLabel(lab)		printf("%s\n",lab)
#define emitInternalILabel(lab)		printf("%s%d\n",ILabelHeader,lab)
#define emitInternalDLabel(lab)		printf("%s%d\n",DLabelHeader,lab)
#define newILabel()			labelID++
#define newDLabel()			dlabelID++

// 命令を生成するマクロ(末尾がvのものは可変長引数を取る)
#define _instOpenEnd(op)	printf("\t" #op "\t\t")
#define inst0(op)		printf("\t" #op "\n")
#define inst1(op,a1)	  	printf("\t" #op "\t\t" #a1 "\n")
#define inst2(op,a1,a2)  	printf("\t" #op "\t\t" #a1 "," #a2 "\n")
#define inst3(op,a1,a2,a3)  	printf("\t" #op "\t\t" #a1 "," #a2 "," #a3 "\n")
#define inst0v(op,...)		printf("\t" #op "\n", __VA_ARGS__)
#define inst1v(op,a1,...)  	printf("\t" #op "\t\t" #a1 "\n", __VA_ARGS__)
#define inst2v(op,a1,a2,...)  	printf("\t" #op "\t\t" #a1 "," #a2 "\n", __VA_ARGS__)
#define inst3v(op,a1,a2,a3,...)	printf("\t" #op "\t\t" #a1 "," #a2 "," #a3 "\n", __VA_ARGS__)

#define modifyOp(op,reg) 						\
  inst3v(LAD, GR%d, %s1, GR%d, reg, (op == INC ? "" : "-"), reg)

#define isAddSubInst(inst) (!strcmp(inst,"ADDA") || !strcmp(inst,"SUBA"))

// breakの行き先を更新
#define updateBreakLabel(lab)  { savedBreakLabel = breakLabel;	\
                                 breakLabel = lab; }

// breakの行き先を復帰
#define restoreBreakLabel()  { breakLabel = savedBreakLabel; }
  
#define min(x,y)	(x < y ? x : y)
#define max(x,y)	(x > y ? x : y)

// 一時変数レジスタの最初の番号
#define FirstTmpReg	(MaxArgRegNum+1)

// 一時変数レジスタが未使用であることを表す定数
#define TmpRegNotUsed	-1

// 式の値を一時変数レジスタに保存するかどうかを表す定数
#define StoreValue	false
#define TrashValue	true

#define QF		((1<<14)&0xffff)
#define NotQF		((~(1<<14))&0xffff)

// break文の行き先ラベルの管理情報
static int breakLabel;		// break文の行き先ラベル

static int labelID = 1;		// 一時ラベル
static int dlabelID = 1;	// 一時データラベル
static int epilogueLabel;	// 関数エピローグ領域のラベル

static int localVarAreaSize;	// 局所変数領域のサイズ
static int savedAreaSize;	// 一時的退避領域のサイズ
static int offsetSavedRegVar;	// SPから退避されたレジスタ変数までのオフセット
static int argRegNum;		// 手続きの仮引数を保持するレジスタの個数

// 乗算、除算手続きを生成するかを判定するフラグ
static bool useMult = false, useDiv = false, useMod = false, useDivRem = false;

// レジスタ情報の初期値(それぞれ、一時変数を0,1,2個使った状態を表す)
static RegInfo Used0RegInfo = { TmpRegNotUsed, FirstTmpReg };
static RegInfo Used1RegInfo = { FirstTmpReg, FirstTmpReg+1 };
static RegInfo Used2RegInfo =  { FirstTmpReg, FirstTmpReg+2 };

// 関数呼び出しのコード生成用の補助データ
static int callNest = 0;			// 関数呼び出しのネストの深さ
static VarEntry* paramEntry[MaxArgRegNum];	// 仮引数の変数エントリ


// 内部関数のプロトタイプ宣言
static void emitPrologue(ProcTree* proc);
static void emitEpilogue(STreeID id);
static void emitArrayElements(PList* list);
static void emitEstmt(Tree* astmt);
static void emitAssignment(ETree* tree, int reg, RegInfo info, bool trashVal);
static void emitExp(Tree* tree, int reg, RegInfo info, bool trashVal);
void emitCondition(Tree* cond, int lab);
static void emitCompoundExp(ETree* tree, int reg, RegInfo info, bool trashVal);
static void emitCall(char* name, int argc, PList* args,
                     int reg, RegInfo info, bool system, bool trashVal);
static void emitMulExp(ETree* exp, int reg, RegInfo info, bool trashVal);
static void emitDivExp(ETree* exp, int reg, RegInfo info, bool trashVal);
static void emitModExp(ETree* exp, int reg, RegInfo info, bool trashVal);
static void emitSimpleBopExp(char* inst, Tree* lhs, Tree* rhs,
                             int reg, RegInfo info, bool trashVal);
static void emitShiftExp(char* inst, ETree* exp, int reg, RegInfo info,
                         bool trashVal);
static void emitRelEqExp(ETree* exp, char* jmp, int v1, int v2,
                         int reg, RegInfo info, bool trashVal);
static void emitBitNot(Tree* exp, int reg, RegInfo info, bool trashVal);
static void emitVar(char* inst, VarEntry* var, int reg, RegInfo info);
static char* emitIdxExp(Tree* exp, int reg, RegInfo info, bool trashVal);
static char* emitIdxEaddr(Tree* addr, Tree* exp, int reg, RegInfo info);
static void emitStoreVar(VarEntry* var, int reg);
static void emitModifyop(ModifyOrder ord, ModifyOp op, Tree* tree,
                         int reg, RegInfo info, bool trashVal);
static void _emitNeg(int reg, int reg2);
static int allocReg(RegInfo* info, int* reg);
static void rollBackRegAlloc(RegInfo info, RegInfo* rollBack);
static int allocBuiltinRegs(int num);
static void emitMulExpNum(Tree* exp, int num, int reg, RegInfo info,
                          bool trashVal);
static void emitMultiplication();
static void emitDivision();
static void emitModulo();
static void emitDivisionWithRemainder();
static bool branchLast(Tree* cond);
static void saveRegisterVar(int index);
static void restoreRegisterVar(int index);
static void push(int index);
static void pop(int index);
static void popIfNecessary(int saved);
void extendStack(int num);
void shrinkStack(int num);

// 組み込み関数のコード生成
static void emitSetVector();
static void emitEnableIntr();
static void emitDisableIntr();
static void emitStoreRegisters();	// デバッグ用

// START命令を出力する
void emitStart()
{
  // 入口ラベルは、すべてのプログラムで @PROG とする（ダミー）。
  printf("@PROG\tSTART\t%smain\n", IdLabelHeader);
}

// END命令を出力する
void emitEnd()
{
  inst0(END);
}

// 手続き先頭のコード生成と初期化
static void emitPrologue(ProcTree* proc)
{
  int i;

  // 変数の初期化
  epilogueLabel = newILabel();
  localVarAreaSize = getLocalVarNum();
  argRegNum = min(MaxArgRegNum,proc->entry->paramNum);
  offsetSavedRegVar = -1;

  // 手続きラベルの生成
  emitInstLabel(proc->entry->name);
  // 局所変数領域の確保
  if (localVarAreaSize)
    inst3v(LAD, GR15, -%d, GR15, localVarAreaSize);
  // 割り込みハンドラなら全レジスタを退避するコードを生成
  if (proc->id == Sintr)  {
    inst0(RPUSH);
    savedAreaSize += AllRegNum;
  }
}

// 手続き末尾のコード生成
static void emitEpilogue(STreeID id)
{
  emitInternalILabel(epilogueLabel);
  // 割り込みハンドラなら全レジスタを復帰するコードを生成
  // GR0も復帰するので割り込みハンドラは戻り値を返せない
  if (id == Sintr)  {
    inst0(RPOP);
    savedAreaSize -= AllRegNum;
  }
  if (localVarAreaSize)
    inst3v(LAD, GR15, %d, GR15, localVarAreaSize);
  // 割り込みハンドラならRETIで復帰
  if (id == Sintr)
    inst0(RETI);
  else
    inst0(RET);
}

// 大域的な変数とデータ確保のためのコード生成
void emitData()
{
  SymbolEntry* e;
  int label, n;
  SymTableIterator it = initSymtableIterator();

  // 大域的な記号表から変数エントリだけを順に処理する
  while ((e=getNextSymbolEntry(it)) != NULL)  {
    if (isVarEntry(e))  {
      Tree* init = e->var.init;
      int size = e->var.index;	// データのサイズを取得
      // 変数ラベルの生成
      emitDataLabel(e->var.name);
      if (init)  {
        // 初期値があるデータの領域確保
        if (isArray((VarEntry*)e))  {
          // 初期値がある配列変数のための領域確保
          switch (init->id)  {
          case Sstr:  {
            // 配列変数の初期値が文字列のとき
            // 文字列
            char* str = init->str.str;
            inst1v(DC, '%s',  str);
            n = size - strlen(str); }
            break;
          case Sarray:
            // 配列変数の初期値が配列のとき
            emitArrayElements(init->array.list);
            n = size - init->array.size;
          }
          // 配列のサイズが初期値のサイズより大きければパディング
          if (n > 0) inst1v(DS, %d,  n);
        }
        else  {
          // 初期値がある単純変数のための領域確保
          switch (init->id)  {
          case Sstr:
            // 変数の初期値が文字列のとき
            // 変数の初期値は文字列の領域のラベル
            label = newDLabel();
            inst1v(DC, %s%d, DLabelHeader, label);
            // 文字列の領域を指すラベルを作り、文字列の領域を確保する
            emitInternalDLabel(label);
            inst1v(DC, '%s', init->str.str);
            break;
          case Snum:
            // 変数の初期値が数のとき
            inst1v(DC, %d, init->num.value);
            break;
          case Savar:
            // 変数の初期値が配列変数のとき
            inst1v(DC, %s, init->var.entry->name);
            break;
          case Shandler:
            // 変数の初期値が割り込みハンドラ名のとき
            inst1v(DC, %s%s, ILabelHeader, init->handler.entry->name);
            break;
          }
        }
      }
      else  {
        // 初期値のないデータの領域確保
        inst1v(DS, %d, size);
      }
    }
  }
}

#define ElemsPerLine	8

// 配列の初期値の要素からDC命令を生成する
static void emitArrayElements(PList* list)
{
  while (list)  {
    int i;
    _instOpenEnd(DC);
    for (i=0; i<ElemsPerLine; i++)  {
      Tree* tree = list->data;
      if (i != 0) putchar(',');
      switch (tree->id)  {
      case Snum:
        printf("%d", tree->num.value);
        break;
      case Savar:
        printf("%s", tree->var.entry->name);
        break;
      case Shandler:
        printf("%s", tree->handler.entry->name);
        break;
      }
      list = list->next;
      if (!list) break;
    }
    putchar('\n');
  }
}

// 文のコード生成
void emitCode(Tree* tree)
{
  int savedBreakLabel;

  if (tree == NULL) return;

  switch (tree->id)  {
  // 式文のコード生成(変数、配列変数、数、文字列はコード生成しない)
  case Sidx:
  case Sexp:
    // 式文の値はレジスタに保存しない（式の値は不要）
    emitExp(tree,TmpRegNotUsed,Used0RegInfo,TrashValue);
    break;
  // break文のコード生成
  case Sbreak:
    inst1v(JUMP, %s%d, ILabelHeader, breakLabel);
    break;
  // if文のコード生成
  case Sif: {
    Tree* cond = tree->stree.child[0];
    int lab = newILabel(), lab2;
    // 分岐条件判断コードの生成
    emitCondition(cond,lab);
    if (branchLast(cond))  {
      // 条件が真のときにジャンプするコードの生成
      if (tree->stree.size == 3)  {
        emitCode(tree->stree.child[2]);		// else節があればコード生成
      }
      lab2 = newILabel();
      inst1v(JUMP, %s%d, ILabelHeader, lab2);	// then節を飛び越すコード生成
       emitInternalILabel(lab);
      emitCode(tree->stree.child[1]);		// then節のコード生成
      emitInternalILabel(lab2);
    }
    else  {
      // 条件が偽のときにジャンプするコードの生成
      emitCode(tree->stree.child[1]);	// then節のコード生成
      if (tree->stree.size == 3)  {
        // else節があればコード生成
        lab2 = newILabel();
        // else節を飛び越す命令のコード生成
        inst1v(JUMP, %s%d, ILabelHeader, lab2);
        emitInternalILabel(lab);
        emitCode(tree->stree.child[2]);	// else節のコード生成
        emitInternalILabel(lab2);
      }
      else
        emitInternalILabel(lab);
    }
    break; }
  // for文のコード生成
  case Sfor:  {
    Tree* cond = tree->stree.child[1];
    int loop = newILabel();
    int exit = newILabel();
    // 初期設定コードの生成
    emitEstmt(tree->stree.child[0]);
    if (branchLast(cond))  {
      // 条件式による判断をループの最後で行う場合
      int check = newILabel();
      // 条件判断へのジャンプの生成
      inst1v(JUMP, %s%d, ILabelHeader, check);
      // ループ入口ラベルの生成
      emitInternalILabel(loop);
      // ループ本体のコード生成
      updateBreakLabel(exit);		// breakの行き先を更新
      emitCode(tree->stree.child[3]);
      restoreBreakLabel();		// breakの行き先を復帰
      // ループ変数などの更新コード生成
      emitEstmt(tree->stree.child[2]);
      // 終了条件判断コードの生成
      emitInternalILabel(check);
      emitCondition(cond,loop);
    }
    else  {
      // ループ入口ラベルの生成
      emitInternalILabel(loop);
      // 終了条件判断コードの生成
      emitCondition(cond,exit);
      // ループ本体のコード生成
      updateBreakLabel(exit);		// breakの行き先を更新
      emitCode(tree->stree.child[3]);
      restoreBreakLabel();		// breakの行き先を復帰
      // ループ変数などの更新コード生成
      emitEstmt(tree->stree.child[2]);
      // ループ入口ラベルへのジャンプ
      inst1v(JUMP, %s%d, ILabelHeader, loop);
    }
    // ループ出口ラベルの生成
    emitInternalILabel(exit); }
    break;
  // while文のコード生成
  case Swhile: {
    Tree* cond = tree->stree.child[0];
    int loop = newILabel();
    int exit = newILabel();
    bool inf = (cond->id == Snum);
    if (inf)  {
      // 条件式が定数の場合
      if (cond->num.value == 0) return;	// 0 ならコード生成しない
      // 条件式が0でない定数ならは無限ループを生成
      // ループ入口ラベルの生成
      emitInternalILabel(loop);
      // ループ本体のコード生成
      updateBreakLabel(exit);	// breakの行き先を更新
      emitCode(tree->stree.child[1]);
      restoreBreakLabel();	// breakの行き先を復帰
      // ループ入口ラベルへのジャンプ
      inst1v(JUMP, %s%d, ILabelHeader, loop);
    }
    else if (branchLast(cond))  {
      // 条件式による判断をループの最後で行う場合
      int check = newILabel();
      // 条件判断へのジャンプの生成
      inst1v(JUMP, %s%d, ILabelHeader, check);
      // ループ入口ラベルの生成
      emitInternalILabel(loop);
      // ループ本体のコード生成
      updateBreakLabel(exit);	// breakの行き先を更新
      emitCode(tree->stree.child[1]);
      restoreBreakLabel();	// breakの行き先を復帰
      // 終了条件判断コードの生成
      emitInternalILabel(check);
      emitCondition(cond,loop);
    }
    else  {
      // 条件式による判断をループの最初で行う場合
      // ループ入口ラベルの生成
      emitInternalILabel(loop);
      // 終了条件判断コードの生成
      emitCondition(cond,exit);
      // ループ本体のコード生成
      updateBreakLabel(exit);	// breakの行き先を更新
      emitCode(tree->stree.child[1]);
      restoreBreakLabel();	// breakの行き先を復帰
      // ループ入口ラベルへのジャンプ
      inst1v(JUMP, %s%d, ILabelHeader, loop);
    }
    // ループ出口ラベルの生成
    emitInternalILabel(exit);
    break; }
  // return文のコード生成
  case Sreturn:
    if (streeArgNum(tree))  {
      // 戻り値があるときは、その値を求めるコードを生成
      Tree* exp = tree->stree.child[0];
      emitExp(exp,FirstTmpReg,Used1RegInfo,StoreValue);
      inst2v(LD, GR0, GR%d, FirstTmpReg);
    }
    inst1v(JUMP, %s%d, ILabelHeader, epilogueLabel);
    break;
  // halt文のコード生成
  case Shalt:
    inst0(HLT);
    break;
  // in文のコード生成
  case Sin: {
    char* buffName = tree->stree.child[0]->var.entry->name;
    char* lenName = tree->stree.child[1]->var.entry->name;
    inst2v(IN,%s,%s, buffName, lenName);
    inst2v(LD,GR%d,%s, FirstTmpReg, lenName);
    inst2(XOR,GR0,GR0);
    inst3v(ST,GR0,%s,GR%d, buffName, FirstTmpReg); }
    break;
  // out文のコード生成
  case Sout: {
    char* buffName = tree->stree.child[0]->var.entry->name;
    char* lenName = tree->stree.child[1]->var.entry->name;
    inst2v(OUT,%s,%s, buffName, lenName); }
    break;
  // 文の列のコード生成
  case Sseq: {
    PList* seq = tree->seq.seq;
    do  {
      emitCode(seq->data);
      seq = seq->next;
    } while (seq);
    break; }
  // 手続き（関数、割り込みハンドラ）のコード生成
  case Sfunc:
  case Sintr:
    emitPrologue((ProcTree*)tree);
    emitCode(tree->ptree.body);
    emitEpilogue(tree->id);
  }
}

// 式文（for文の中）のコードを生成する
static void emitEstmt(Tree* estmt)
{
  if (estmt == NULL)
    return;
  else
    // 式文なので、式の値をレジスタに保存しない
    emitExp(estmt,TmpRegNotUsed,Used0RegInfo,TrashValue);
}

// 右辺値をレジスタregに格納するコードを生成
// 変数、数、文字列については、式文でないときだけコード生成
static void emitExp(Tree* exp, int reg, RegInfo info, bool trashVal)
{
  switch (exp->id)  {
  // 変数(配列名ではない)
  case Svar:
    if (!trashVal)
      emitVar("LD",exp->var.entry,reg,info);
    break;
  // 変数(配列名)
  case Savar:
    if (!trashVal)
      inst2v(LAD, GR%d, %s, reg, varName(exp));
    break;
  // 数
  case Snum:
    if (!trashVal)
      inst2v(LAD, GR%d, %d, reg, exp->num.value);
    break;
  // 文字列
  case Sstr:
    if (!trashVal)
      inst2v(LAD, GR%d, =%s, reg,exp->str.str);
    break;
  // それ以外の式について、式の値が不要なら値をレジスタに保存しない
  case Sidx: {
    char* addr = emitIdxExp(exp,reg,info,trashVal);
    if (!trashVal)
      inst3v(LD,GR%d,%s,GR%d, reg, addr, reg); }
    break;
  // 複合式
  case Sexp:
    emitCompoundExp((ETree*)exp,reg,info,trashVal);
    break;
  }
}

// if文、for文、while文の条件判断のためのコード生成
void emitCondition(Tree* cond, int lab)
{
  if (cond->id == Sexp)  {
    ETreeID id = cond->etree.eid;
    Tree* lhs = cond->etree.child[0];
    Tree* rhs = cond->etree.child[1];
    // 条件式が関係式(==, !=, >, <, >=, <=を持つ式)のとき
    if (Eeq <= id && id <= Eleq)  {
      emitSimpleBopExp("CPA",lhs,rhs,FirstTmpReg,Used1RegInfo,StoreValue);
      switch (id)  {
      case Eeq:
        inst1v(JNZ, %s%d, ILabelHeader, lab);
        break;
      case Eneq:
        inst1v(JZE, %s%d, ILabelHeader, lab);
        break;
      case Elt:
        inst1v(JMI, %s%d, ILabelHeader, lab);
        break;
      case Egt:
        inst1v(JPL, %s%d, ILabelHeader, lab);
        break;
      case Eleq:
        inst1v(JPL, %s%d, ILabelHeader, lab);
        break;
      case Egeq:
        inst1v(JMI, %s%d, ILabelHeader, lab);
        break;
      }
      return;
    }
  }
  // 条件式が関係式でないとき
  emitExp(cond,FirstTmpReg,Used1RegInfo,StoreValue);
  inst2v(AND, GR%d, GR%d, FirstTmpReg, FirstTmpReg);
  inst1v(JZE, %s%d, ILabelHeader, lab);
}

// 繰り返し文で条件判定コードを最後に置くかどうかの判定
// if文のときはelse節をthen節より先に生成するかどうかの判定
static bool branchLast(Tree* cond)
{
  if (cond->id == Sexp)  {
    ETreeID id = cond->etree.eid;
    return (Egt <= id && id <= Elt);
  }
  else
    return false;
}

// 複合式のコードを生成する
// 式の値が不要なときはregの値は無効なので、レジスタが必要なときは新たに確保する。
static void emitCompoundExp(ETree* exp, int reg, RegInfo info, bool trashVal)
{
  int lab1, lab2;
  int reg2, saved;

  switch (exp->eid)  {
  case Eadd:	// 式 + 式
    emitSimpleBopExp("ADDA",exp->child[0],exp->child[1],reg,info,trashVal);
    break;
  case Esub:	// 式 - 式
    emitSimpleBopExp("SUBA",exp->child[0],exp->child[1],reg,info,trashVal);
    break;
  case Emul:	// 式 * 式
    emitMulExp(exp,reg,info,trashVal);
    break;
  case Ediv:	// 式 / 式
    emitDivExp(exp,reg,info,trashVal);
    break;
  case Emod:	// 式 % 式
    emitModExp(exp,reg,info,trashVal);
    break;
    break;
  case Elshift:	// 式 << 式
    emitShiftExp("SLL",exp,reg,info,trashVal);
    break;
  case Ershift:	// 式 >> 式
    emitShiftExp("SRL",exp,reg,info,trashVal);
    break;
  case Eband:	// 式 & 式
    emitSimpleBopExp("AND",exp->child[0],exp->child[1],reg,info,trashVal);
    break;
  case Ebor:	// 式 | 式
    emitSimpleBopExp("OR",exp->child[0],exp->child[1],reg,info,trashVal);
    break;
  case Ebxor:	// 式 ^ 式
    emitSimpleBopExp("XOR",exp->child[0],exp->child[1],reg,info,trashVal);
    break;
  case Ebnot:	// ~ 式
    emitBitNot(exp->child[0],reg,info,trashVal);
    break;
  case Eneg:	// - 式
    // ビット反転後１を加えて２の補数を作るコードの生成
    emitBitNot(exp->child[0],reg,info,trashVal);
    if (!trashVal)
      inst3v(LAD, GR%d, 1, GR%d, reg, reg);
    break;
  case Egt:	// 式 > 式
    emitRelEqExp(exp,"JPL",0,1,reg,info,trashVal);
   break;
  case Elt:	// 式 < 式
    emitRelEqExp(exp,"JMI",0,1,reg,info,trashVal);
    break;
  case Egeq:	// 式 >= 式
    emitRelEqExp(exp,"JMI",1,0,reg,info,trashVal);
    break;
  case Eleq:	// 式 <= 式
    emitRelEqExp(exp,"JPL",1,0,reg,info,trashVal);
    break;
  case Eeq:	// 式 == 式
    emitRelEqExp(exp,"JNZ",1,0,reg,info,trashVal);
    break;
  case Eneq:	// 式 != 式
    emitRelEqExp(exp,"JZE",1,0,reg,info,trashVal);
    break;
  case Ederef:	// * 式
    // アドレスを求めるコードの生成
    emitExp(exp->child[0],reg,info,trashVal);
    // 求めたアドレスにある値をレジスタに格納する命令の生成
    if (!trashVal)
      inst3v(LD, GR%d, 0, GR%d, reg, reg);
    break;
  case Eaddr:	// & 左辺値
    switch (exp->child[0]->id)  {
    case Svar:
      // 変数のとき
      // 変数のアドレスをレジスタに格納する命令の生成
      if (!trashVal)
        emitVar("LAD",exp->child[0]->var.entry,reg,info);
      break; 
    case Sidx: {
      // 添字式のとき
      // 添字式が指す実効アドレスをaddr(reg)に格納する命令の生成
      char* addr = emitIdxExp(exp->child[0],reg,info,trashVal);
      if (!trashVal && addr[0] != '0')
        // 式の値が必要で、アドレスが0でなければ、実効アドレスを
        // reg番目のレジスタに格納する命令を生成
        inst3v(LAD,GR%d,%s,GR%d, reg, addr, reg);
      break; }
    case Sexp:
      // 間接参照(*exp の形の式)のとき
      // & と * が相殺されるので、式 expのコードを生成
      emitExp(exp->child[0],reg,info,trashVal);
    }
    break;
  case Epreinc:		// ++lvalue
    emitModifyop(PRE,INC,exp->child[0],reg,info,trashVal);
    break;
  case Epredec:		// --lvalue
    emitModifyop(PRE,DEC,exp->child[0],reg,info,trashVal);
    break;
  case Epostinc:	// lvalue++
    emitModifyop(POST,INC,exp->child[0],reg,info,trashVal);
    break;
  case Epostdec:	// lvalue--
    emitModifyop(POST,DEC,exp->child[0],reg,info,trashVal);
    break;
  // e1 && e2 のコード生成(ショートサーキット)
  case Eand:
    lab1 = newILabel();
    // e1 && e2 の値が不要でも、e2の実行要否判定のためe1の値を使う
    // 式の値が不要だったときは、e1の値のためのレジスタを確保する
    if (trashVal)
      saved = allocReg(&info,&reg);
    emitExp(exp->child[0],reg,info,StoreValue);	// e1 のコード生成
    inst2v(AND,GR%d,GR%d, reg, reg);		// e1の値をチェック
    // e1 の値が 0 なら e1 && e2 は偽なので、e2は実行しない
    inst1v(JZE, %s%d, ILabelHeader, lab1);
    // e1 && e2 の値が不要なら、e2の値はレジスタに格納しない
    emitExp(exp->child[1],reg,info,trashVal);	// e2 のコード生成
    if (!trashVal)  {
      inst2v(AND,GR%d,GR%d, reg, reg);	// e2 の値をチェック
      // e2 の値が0なら e1 && e2 は偽なので、e2の値をそのまま返すためにジャンプ
      inst1v(JZE,%s%d, ILabelHeader, lab1);
      inst2v(LAD,GR%d, 1, reg);		// e1 && e2 が真なら1を返す
    }
    emitInternalILabel(lab1);
    if (trashVal) popIfNecessary(saved);
    break;
  // e1 || e2 のコード生成(ショートサーキット)
  case Eor: {
    lab1 = newILabel();
    if (!trashVal) lab2 = newILabel();
    // e1 || e2 の値が不要でも、e2の実行要否判定のためe1の値を使う
    // 式の値が不要だったときは、e1の値のためのレジスタを確保する
    if (trashVal)
      saved = allocReg(&info,&reg);
    emitExp(exp->child[0],reg,info,StoreValue);	// e1 のコード生成
    inst2v(LD, GR%d, GR%d, reg, reg);		// e1の値をチェック
    // e1 の値が非0なら e1 || e2 は真なので、e2は実行しない
    inst1v(JNZ, %s%d, ILabelHeader, lab1);
    // e1 || e2 が式文なら、e2の値はレジスタに格納しない
    emitExp(exp->child[1],reg,info,trashVal);	// e2 のコード生成
    if (!trashVal)  {
      inst2v(LD, GR%d, GR%d, reg, reg);		// e2の値をチェック
      // e2 の値が0なら e1 || e2 は偽なので、e2の値をそのまま返すためにジャンプ
      inst1v(JZE, %s%d, ILabelHeader, lab2);
    }
    emitInternalILabel(lab1);
    if (!trashVal)  {
      inst2v(LAD, GR%d, 1, reg);	// e1 || e2 が真なら1を返す
      emitInternalILabel(lab2);
    }
    if (trashVal) popIfNecessary(saved);
    break; }
  // ! e1 のコード生成
  case Enot:
    if (!trashVal)  {
      lab1 = newILabel();
      lab2 = newILabel();
    }
    emitExp(exp->child[0],reg,info,trashVal);	// e1 のコード生成
    if (!trashVal)  {
      inst2v(LD, GR%d, GR%d, reg, reg);		// e1 の値をチェック
      inst1v(JZE, %s%d, ILabelHeader, lab1);
      inst2v(XOR, GR%d, GR%d, reg, reg);	// e1 が非0なら0にする
      inst1v(JUMP, %s%d, ILabelHeader, lab2);
      emitInternalILabel(lab1);
      inst2v(LAD, GR%d, 1, reg);		// e1 が0なら1にする
      emitInternalILabel(lab2);
    }
    break;
  // 手続き呼び出しのコード生成
  case Ecall: {
    // 構文木から手続きエントリを取り出す
    ProcEntry* proc = (ProcEntry*)exp->child[0];
    int argc = exp->child[1]->num.value;
    bool system = (proc->flag & PESystem ? true : false);
    PList* args = (PList*)exp->child[2];
    emitCall(proc->name,argc,args,reg,info,system,trashVal);
    break; }
  // 代入文のコード生成
  case Eassign:
    emitAssignment(exp,reg,info,trashVal);
    break;
  // カンマ演算子をもつ式
  case Ecomma: {
    PList* list = (PList*)exp->child[0];
    RegInfo info2;
    if (!trashVal)
      rollBackRegAlloc(info,&info2);
    else
      info2 = info;
    // 最初の式のコード生成(値は不要なので捨てる)
    emitExp(list->data,reg,info2,TrashValue);
    // ２つ目以降の式のコード生成
    for (list=list->next; list; list=list->next)  {
      if (list->next)
        // 最後の式でなければ値を捨てるコードを生成
        emitExp(list->data,reg,info2,TrashValue);
      else
        // 最後の式なら(必要であれば)値をregに保存するコードを生成
        emitExp(list->data,reg,info,trashVal);
    }
    break; }
  }
}

// 代入文のコード生成
static void emitAssignment(ETree* tree, int reg, RegInfo info, bool trashVal)
{
  int reg2, saved = 0, saved2;
  Tree* lhs = tree->child[0];
  Tree* rhs = tree->child[1];

  // この代入文の値が不要なときは、右辺の値を一時変数レジスタに保存しなくてもよい。
  // 以下に示す場合(else節以外)では、標準的なコード生成は行わず、個別に対応する。
  // それ以外の場合(else節)は、次に利用可能な一時変数レジスタをregにセットして、
  // 標準的なコード生成を行う。
  if (trashVal)  {
    // 左辺がスタックに退避されていない引数レジスタのとき、
    // 右辺の形によっては右辺の値をレジスタに保存しなくてもよい
    if (lhs->id == Svar && isParamReg(lhs->var.entry))  {
      VarEntry* lent = lhs->var.entry;
      switch (rhs->id)  {
      // 右辺が変数なら、LD命令一つだけで代入ができる。
      case Svar: {
        VarEntry* rent = rhs->var.entry;
        int offset;
        if (isGlobalVar(rent))
          inst2v(LD,GR%d,%s, lent->index, varName(rhs));
        else if (isLocalVar(rent))  {
          offset = savedAreaSize + rent->index;
          inst3v(LD,GR%d,%d,GR15, lent->index, offset);
        }
        else if (isParamReg(rent))
          inst2v(LD,GR%d,GR%d, lent->index, rent->index);
        else if (isParamStack(rent))  {
          offset = localVarAreaSize + 1 + savedAreaSize + rent->index;
          inst3v(LD,GR%d,%d,GR15, lent->index, offset);
        }
        else if (isSavedParamReg(rent))  {
          offset = offsetSavedRegVar + rent->index - 1;
          inst3v(LD,GR%d,%d,GR15, lent->index, offset);
        }
        return; }
       // 右辺が配列名なら、LAD命令一つだけで代入ができる
      case Savar:
        inst2v(LAD,GR%d,%s, lent->index, varName(rhs));
        return;
       // 右辺が文字列なら、リテラルを使えば、LAD命令一つだけで代入ができる
      case Sstr:
        inst2v(LAD,GR%d,=%s, lent->index, rhs->str.str);
        return;
       // 右辺が数なら、LAD命令一つだけで代入ができる
      case Snum:
        inst2v(LAD,GR%d,%d, lent->index, rhs->num.value);
        return;
       // 右辺が添字式のときも、左辺が仮引数なら命令数を削減できる
      case Sidx: {
        allocReg(&info,&reg);
        char* addr = emitIdxExp(rhs,reg,info,StoreValue);
        inst3v(LD,GR%d,%s,GR%d, lent->index, addr, reg);
        return; }
      // それ以外の場合は、右辺の値を保存するための一時変数レジスタを確保して、
      // そのレジスタに右辺の式の値を格納するコードを生成する
      default:
        saved = allocReg(&info,&reg);
        emitExp(rhs,reg,info,StoreValue);
      }
    }
    // 右辺がスタックに退避されていない引数レジスタのときは、
    // 右辺の値として引数レジスタを直接指定する。
    // したがって、右辺の式の値をレジスタに格納するコードは生成しない
    else if (rhs->id == Svar && isParamReg(rhs->var.entry))  {
      reg = rhs->var.entry->index;
      saved = 0;
    }
    // 左辺が変数で右辺が手続き呼び出しなら、
    // GR0に右辺の式の値を格納するコードを生成する

    else if (lhs->id == Svar && rhs->id == Sexp && rhs->etree.eid == Ecall)  {
      saved = reg = 0;
      emitExp(rhs,reg,info,StoreValue);
    }
    // それ以外の場合は、右辺の値を保存するための一時変数レジスタを確保して、
    // そのレジスタに右辺の式の値を格納するコードを生成する
    else  {
      saved = allocReg(&info,&reg);
      emitExp(rhs,reg,info,StoreValue);
    }
  }
  else
    // 引数の値が必要なときは、一時変数レジスタregに右辺の式の値を
    // 格納するコードを生成する
    emitExp(rhs,reg,info,StoreValue);

  // 左辺値のコード生成
  switch (lhs->id)  {
  case Svar:
    // 左辺の変数に右辺の値を格納する命令の生成
    emitStoreVar(lhs->var.entry,reg);
    break;
  case Sidx: {
    saved2 = allocReg(&info,&reg2);
    // 添字式が指す実効アドレスのレジスタ部にアドレスを格納するコードを生成
    char* addr = emitIdxExp(lhs,reg2,info,StoreValue);
    // 添字式が指す実効アドレスに右辺の値を格納する命令の生成
    inst3v(ST,GR%d,%s,GR%d, reg, addr, reg2);
    popIfNecessary(saved2); }
    break;
  case Sexp:
    // 左辺が *exp の形のときに、exp の値を求めるコードを生成
    saved2 = allocReg(&info,&reg2);
    emitExp(lhs->etree.child[0],reg2,info,StoreValue);
    // 左辺のアドレスに右辺の値を格納する命令の生成
    inst3v(ST, GR%d, 0, GR%d, reg, reg2);
    popIfNecessary(saved2);
  }
  if (trashVal)
    popIfNecessary(saved);
}

static void emitCall(char* name, int argc, PList* args,
                     int reg, RegInfo info, bool system, bool trashVal)
{
  int i, n, last;
  // スタックに置く引数の個数を求める
  int argStackNum = max(argc - MaxArgRegNum, 0);
  // 引数レジスタを退避するコードの生成
  for (i=argRegNum; i>=1; i--)  {
    // regはまだ値を持たないので退避不要
    if (reg != i) push(i);
  }
  // ネストされた手続き呼び出しでなければ、引数レジスタがスタックに
  // 退避されたことと、仮引数のスタック上の位置を仮引数の記号表
  // エントリに記憶する。
  if (callNest == 0)  {
    // ネストしてないので、呼び出し側の引数レジスタを退避
    offsetSavedRegVar = 0;
    for (i=1; i<=argRegNum; i++)
      saveRegisterVar(i);	// 仮引数が退避されたことを変数エントリに記憶
  }
  if (!system)  {
    // システム関数の呼び出しでなければ、使用中の一時変数レジスタを退避
    n = 0;	// 退避する一時変数レジスタの個数を初期化
    if (info.usedHead != TmpRegNotUsed)  {
      // reg がGR0か戻り値が不要なら、使用中の一時変数レジスタをすべて退避
      // それ以外なら reg 以外の使用中の一時変数レジスタをすべて退避
      last = (reg == 0 || trashVal) ? info.availableHead : reg;
      // 使用中の一時変数レジスタを退避(regはまだ使用中でないので退避しない)
      for (i=info.usedHead; i!=last; i=nextTmpReg(i))  {
        push(i);
        ++n;	// プッシュした一時変数レジスタの個数を記憶
      }
    }
  }
  // 引数をスタックに積むなら、その領域を確保
  if (argStackNum)  {
    extendStack(argStackNum);
  }
  // 呼び出され側の引数レジスタの保護のために argRegNum を後で変更する
  // ので、呼び出し側の引数レジスタ数をもつargRegNumの値を保存しておく
  int savedArgRegNum = argRegNum;
  ++callNest;
  // 呼び出され側の引数をレジスタとスタックに格納するコードを生成
  for (i=1; args != NULL; i++)  {
    // MaxArgRegNum番目以下の実引数ならば引数レジスタへ格納
    if (i <= MaxArgRegNum)  {
      // すでに引数の値をもつ、i-1番目までの呼び出され側の引数レジスタが
      // 壊されないよう保護する。
      // i も一時変数レジスタとして使用されることがあるので保護する。
      argRegNum = i;
      // i番目の引数レジスタに値を格納するので、一時変数レジスタは未使用
      emitExp(args->data,i,Used0RegInfo,StoreValue);
    }
    else  {
      // それより後の実引数はスタックへ格納
      //      emitExp(args->seq.tree,FirstTmpReg,Used1RegInfo,StoreValue);
      emitExp(args->data,FirstTmpReg,Used1RegInfo,StoreValue);
      inst3v(ST, GR%d, %d, GR15, FirstTmpReg, i-MaxArgRegNum-1);
    }
    //    args = args->seq.next;
    args = args->next;
  }
  // 関数呼び出しのコード生成
  inst1v(CALL,%s, name);
  // 実引数領域を解放するコードの生成
  if (argStackNum) shrinkStack(argStackNum);
  // 一時変数レジスタを復帰するコードの生成
  if (!system)
    for (i=prevTmpReg(last); n; i=prevTmpReg(i), n--) pop(i);
  // 呼び出し側の引数レジスタの個数を復帰
  argRegNum = savedArgRegNum;
  --callNest;
  // 呼び出し側の引数を持つレジスタを復帰する場合
  if (callNest == 0)  {
    offsetSavedRegVar = -1;
    for (i=1; i<=argRegNum; i++)
      // 仮引数をレジスタに復帰したことを変数エントリに記憶
      restoreRegisterVar(i);
  }
  // 引数レジスタを復帰するコードの生成
  for (i=1; i<=argRegNum; i++)
    if (i != reg) pop(i);
  // 関数呼び出しの戻り値が必要で、かつ、戻り値をGR0以外に保存するなら、
  // 戻り値をそのレジスタに保存する
  if (!trashVal && reg != 0)
    inst2v(LD, GR%d, GR0, reg);
}

// 乗算のためのコード生成
static void emitMulExp(ETree* exp, int reg, RegInfo info, bool trashVal)
{
  Tree* exp1 = exp->child[0];
  Tree* exp2 = exp->child[1];
  if (exp1->id == Snum)
    // 左辺が定数のときのコード生成
    emitMulExpNum(exp2,exp1->num.value,reg,info,trashVal);
  else if (exp2->id == Snum)  {
    // 右辺が定数のときのコード生成
    emitMulExpNum(exp1,exp2->num.value,reg,info,trashVal);
  }
  else if (trashVal)  {
    // 乗算の値が不要なら乗算を実行しない
    emitExp(exp1,reg,info,trashVal);		// 左辺のコード生成
    emitExp(exp2,reg,info,trashVal);		// 右辺のコード生成
  }
  else  {
    // 引数がどちらも定数でないときは、乗算ルーチンを呼ぶ
    useMult = 1;	// 乗算ルーチンを目的プログラムに含める
    PList* args = newPList(exp1,newPList(exp2,NULL));
    emitCall("@multiply",2,args,reg,info,true,StoreValue);
    deletePList(args,deleteNothing);
  }
}

// 除算のためのコード生成
static void emitDivExp(ETree* exp, int reg, RegInfo info, bool trashVal)
{
  Tree* exp1 = exp->child[0];
  Tree* exp2 = exp->child[1];
  // 除算の値が不要なら除算を生成しない
  if (trashVal)  {
    emitExp(exp1,reg,info,trashVal);	// 左辺のコード生成a
    emitExp(exp2,reg,info,trashVal);	// 右辺のコード生成a
  }
  else  {
    // 除算の値が必要なら除算ルーチンを呼ぶ
    useDiv = 1;		// 除算ルーチンを目的プログラムに含める
    useDivRem = 1;	// 除算・剰余算ルーチンを目的プログラムに含める
    PList* args = newPList(exp1,newPList(exp2,NULL));
    emitCall("@divide",2,args,reg,info,true,StoreValue);
    deletePList(args,deleteNothing);
  }
}

// 剰余算のためのコード生成
static void emitModExp(ETree* exp, int reg, RegInfo info, bool trashVal)
{
  Tree* exp1 = exp->child[0];
  Tree* exp2 = exp->child[1];
  // 剰余算の値が不要なら除算を生成しない
  if (trashVal)  {
    emitExp(exp1,reg,info,trashVal);	// 左辺のコード生成a
    emitExp(exp2,reg,info,trashVal);	// 右辺のコード生成a
  }
  else  {
    // 剰余算の値が必要なら除算ルーチンを呼ぶ
    int i;
    useMod = 1;		// 剰余算ルーチンを目的プログラムに含める
    useDivRem = 1;	// 除算・剰余算ルーチンを目的プログラムに含める
    PList* args = newPList(exp1,newPList(exp2,NULL));
    emitCall("@modulo",2,args,reg,info,true,StoreValue);
    deletePList(args,deleteNothing);
  }
}

// レジスタ reg の符号反転を reg にセット(reg2は作業レジスタ)
static void _emitNeg(int reg, int reg2)
{
  inst2v(LAD, GR%d, -1, reg2);
  inst2v(XOR, GR%d, GR%d, reg, reg2);
  inst3v(LAD, GR%d, 1, GR%d, reg, reg);
}

// ビット否定のためのコード生成
static void emitBitNot(Tree* exp, int reg, RegInfo info, bool trashVal)
{
  // 引数の式の値のコード生成
  emitExp(exp,reg,info, trashVal);
  // ビット否定式の値が必要なら、式の値をビット否定するコードの生成 
  if (!trashVal)  {
    inst2(LAD, GR0, -1);
    inst2v(XOR, GR%d, GR0, reg);
  }
}

// 変数のためのコード生成
static void emitVar(char* inst, VarEntry* var, int reg, RegInfo info)
{
  int offset = 0;
  switch (var->class)  {
  case GlobalVar:
    inst2v(%s, GR%d, %s, inst, reg, var->name);
    break;
  case Param:
    if (var->flag & VERegister)  {
      if (var->flag & VESaveReg)  {
        // 引数レジスタの値がスタックに退避されていたとき
        offset = offsetSavedRegVar + var->index - 1;
        inst3v(%s, GR%d, %d, GR15, inst, reg, offset);
      }
      else  {
        // 引数レジスタの値がレジスタにあるなら、レジスタをコピーして終了
        inst2v(%s, GR%d, GR%d, inst, reg, var->index);
      }
      break;
    }
    else
      // スタック上の仮引数のとき
      offset = localVarAreaSize + 1;	// 1 は戻りアドレスの領域
  case LocalVar:
    offset += savedAreaSize + var->index;
    inst3v(%s, GR%d, %d, GR15, inst, reg, offset);
  }
}

// 添字式の形に応じて実効アドレスのコードを生成
static char* emitIdxExp(Tree* exp, int reg, RegInfo info, bool trashVal)
{
  Tree* base  = exp->idx.base;
  Tree* index = exp->idx.index;

  if (!trashVal)  {
    if (base->id == Savar || base->id == Snum)
      return emitIdxEaddr(base,index,reg,info);
    else if (index->id == Savar || index->id == Snum)
      return emitIdxEaddr(index,base,reg,info);
    else  {
      emitSimpleBopExp("ADDA",base,index,reg,info,StoreValue);
      return "0";
    }
  }
  else  {
    // 式の値が不要のときは(必要なら)引数のコードだけを生成
    emitExp(exp->idx.base,  FirstTmpReg, Used1RegInfo, trashVal);
    emitExp(exp->idx.index, FirstTmpReg, Used1RegInfo, trashVal);
    return NULL;
  }
}

// 添字式の実効アドレスを生成（アドレス部はこの後で生成される命令に
// 埋め込まれるので、文字列にして返す）
static char* emitIdxEaddr(Tree* addr, Tree* exp, int reg, RegInfo info)
{
  emitExp(exp,reg,info,StoreValue);
  if (addr->id == Snum)  {
    static char numstr[10];
    snprintf(numstr, 10, "%d", addr->num.value);
    return numstr;
  }
  else
    return varName(addr);
}

// 変数に値を格納するコードの生成
static void emitStoreVar(VarEntry* var, int reg)
{
  int offset = 0;
  switch (var->class)  {
  // グローバル変数に格納するとき
  case GlobalVar:
    inst2v(ST, GR%d, %s, reg, var->name);
    break;
  // 仮引数に格納するとき
  case Param:
    // 引数レジスタのとき
    if (var->flag & VERegister)  {
      if (var->flag & VESaveReg)  {
        // 一時的にスタックに退避されているときはスタックに値を格納
        offset = offsetSavedRegVar + var->index - 1;
        inst3v(ST, GR%d, %d, GR15, reg, offset);
      }
      else  {
        // それ以外ならレジスタ間コピー
        inst2v(LD, GR%d, GR%d, var->index, reg);
      }
      break;
    }
    else
      // スタック上の仮引数のときは、局所変数と同じ処理
      offset = localVarAreaSize + 1;	// 戻りアドレスの領域を飛び越す
      // そのまま下へ行く
  // 局所変数に格納するとき
  case LocalVar:
    offset += savedAreaSize;
    offset += var->index;
    inst3v(ST, GR%d, %d, GR15, reg, offset);
  }
}

// ++, -- 演算子のためのコード生成
static void emitModifyop(ModifyOrder ord, ModifyOp op, Tree* tree,
                         int reg, RegInfo info, bool trashVal)
{
  int reg2, saved, saved2;
  // sreg はpost-modifierによる増減の間、値を退避しておくレジスタ
  int sreg;

  // 式の値が不要なときは、レジスタを確保する。
  if (trashVal)
    saved = allocReg(&info,&reg);

  // 式の値が必要なpost-modifierのときは、増減前の値をGR0に退避。
  // 増減した値を左辺値に格納した後、GR0の値をregに復帰する。
  // それ以外のときは、増減した値をそのまま返してよい。
  sreg = (ord == POST && !trashVal) ? 0 : reg;

  // ++, -- の引数に応じたコードを生成
  switch (tree->id)  {
  // 引数が変数のとき
  case Svar:  {
    VarEntry *var = tree->var.entry;
    // 変数の値を reg に代入する命令の生成
    emitVar("LD",var,reg,info);
    // 式の値が必要なpost-modifier のときだけ reg と mreg は異なるので、
    // そのときだけ reg の内容を mreg にコピーする命令を生成
    if (ord == POST && !trashVal)
      inst2v(LD,GR%d,GR%d, sreg, reg);
    // reg に対して値を１増減する
    modifyOp(op, reg);
    // 増減後の値を変数に戻す
    emitStoreVar(var,reg);
    // 式の値が必要なpost-modifier のときだけ、
    // sregに退避した増減前の値をregに復帰する。
    if (ord == POST && !trashVal)
      inst2v(LD,GR%d,GR%d, reg, sreg);
    }
    break;
  // 引数が配列要素のとき
  case Sidx: {
    saved2 = allocReg(&info,&reg2);
    char* addr = emitIdxExp(tree,reg2,info,StoreValue);
    inst3v(LD,GR%d,%s,GR%d, reg, addr, reg2);
    // 式の値が必要なpost-modifier のときだけ reg と mreg は異なるので、
    // そのときだけ reg の内容を mreg にコピーする命令を生成
    if (ord == POST && !trashVal)
      inst2v(LD,GR%d,GR%d, sreg, reg);
    // reg に対して値を１増減する
    modifyOp(op, reg);
    // 増減後の値を配列要素に戻す
    inst3v(ST,GR%d,%s,GR%d, reg, addr, reg2);
    // 式の値が必要なpost-modifier のときだけ、
    // sregに退避した増減前の値をregに復帰する。
    if (ord == POST && !trashVal)
      inst2v(LD,GR%d,GR%d, reg, sreg);
    popIfNecessary(saved2); }
    break;
  // 引数が *exp の形のとき(他の複合式は現れない)
  case Sexp:
    // 値のためにreg、増減後の値の格納先のためにreg2を使用
    saved2 = allocReg(&info,&reg2);
    // * の引数に対するコードを生成(reg2にアドレスを代入)
    emitExp(tree->etree.child[0],reg2,info,StoreValue);
    // *exp の値をregに代入する命令の生成(reg2が指すアドレスの値をregに代入)
    inst3v(LD,GR%d,0,GR%d, reg, reg2);
    // 式の値が必要なpost-modifier のときだけ reg と smreg は異なるので、
    // そのときだけ reg の内容を sreg にコピーする命令を生成
    if (ord == POST && !trashVal)
      inst2v(LD,GR%d,GR%d, sreg, reg);
    // reg に対して値を１増減する
    modifyOp(op, reg);
    // 増減後の値を reg2 が指すアドレスに戻す
    inst3v(ST,GR%d,0,GR%d, reg, reg2);
    // 式の値が必要なpost-modifier のときだけ、
    // sregに退避した増減前の値をregに復帰する。
    if (ord == POST && !trashVal)
      inst2v(LD,GR%d,GR%d, reg, sreg);
    popIfNecessary(saved2);
  }
  if (trashVal)
    popIfNecessary(saved);
}

// 関係演算子を持つ式のコード生成
// jmp は真偽判定で使うジャンプ命令。v1とv2は一方が1、他方が0になる
static void emitRelEqExp(ETree* exp, char* jmp, int v1, int v2, 
                         int reg, RegInfo info, bool trashVal)
{
  int lab, lab2;
  Tree* lhs = exp->child[0];
  Tree* rhs = exp->child[1];

  // 引数を比較する CPA 命令の生成
  // 式の値が不要なら、CPA命令は生成されず、副作用のある部分式のコードだけが
  // 生成される
  emitSimpleBopExp("CPA",lhs,rhs,reg,info,trashVal);
  // 式の値が不要なら、判定のための命令は生成しない
  if (!trashVal)  {
    // フラグレジスタを見て、1か0をreg番目のレジスタに格納するコードの生成
    lab = newILabel();
    inst1v(%s,%s%d, jmp, ILabelHeader, lab);
    inst2v(LAD,GR%d,%d, reg, v1);
    lab2 = newILabel();
    inst1v(JUMP,%s%d, ILabelHeader, lab2);
    emitInternalILabel(lab);
    inst2v(LAD,GR%d,%d, reg, v2);
    emitInternalILabel(lab2);
  }
}

// シフト演算のためのコード生成
static void emitShiftExp(char* inst, ETree* exp, int reg, RegInfo info,
                         bool trashVal)
{
  int saved, reg2;
  Tree* op2 = exp->child[1];

  // シフト演算の左辺のコード生成
  emitExp(exp->child[0],reg,info,trashVal);

  // シフト演算の右辺とシフト演算のコード生成
  if (trashVal)  {
    // シフト演算の値が不要なら、右辺の値もシフト演算も不要なので、
    // 右辺の値を保存するレジスタを確保せずに右辺のコード生成関数の
    // 呼び出しだけを行う
    emitExp(op2,reg,info,trashVal);
  }
  // これ以降はシフト演算の値が必要な場合
  else if (op2->id == Snum)  {
    // 右辺が数のとき
    inst2v(%s, GR%d, %d, inst, reg, op2->num.value);
  }
  else  {
    // 右辺の値をreg2番目のレジスタに格納するコードの生成
    saved = allocReg(&info,&reg2);
    switch (op2->id)  {
    // 右辺が複合式か文字列のとき
    case Sexp:
    case Sstr:
      emitExp(op2,reg2,info,StoreValue);
      break;
    // 右辺が変数のとき
    case Svar:
      emitVar("LD",op2->var.entry,reg2,info);
      break;
    // 右辺が配列変数(配列のアドレス)のとき
    case Savar:
      inst2v(LAD, GR%d, %s, reg2, varName(op2));
      break;
    // 右辺が添字式のとき
    case Sidx: {
      char* addr = emitIdxExp(op2,reg2,info,StoreValue);
      inst3v(LD,GR%d,%s,GR%d, reg2, addr, reg); }
    }
    // reg番目のレジスタをreg2番目のレジスタの内容だけシフトする命令の生成
    inst3v(%s, GR%d, 0, GR%d, inst, reg, reg2);
    popIfNecessary(saved);
  }
}

// 二項演算子をもつ式で命令と直接対応するもののコード生成
// オペランドが実効アドレスのとき、そのアドレスの内容を使う命令に対応
static void emitSimpleBopExp(char* inst, Tree* lhs, Tree* rhs,
                             int reg, RegInfo info, bool trashVal)
{
  int saved, reg2;

  // 式の左辺のコード生成
  emitExp(lhs,reg,info,trashVal);

  // 式の右辺と式全体のコード生成
  if (trashVal)  {
    // 演算子を持つ式が式文の中に現れるときは値を使わないので、
    // 新たに一時変数レジスタを確保しない
    emitExp(rhs,reg,info,trashVal);
  }
  else  {
    if (isAddSubInst(inst))  {
      if (rhs->id == Snum)  {
        // 右辺が数の加減算は、加減算命令ではなくLAD命令で行う
        int val = (!strcmp(inst,"ADDA") ? rhs->num.value : -rhs->num.value);
        inst3v(LAD, GR%d, %d, GR%d, reg, val, reg);
        return;
      }
      if (rhs->id == Savar && !strcmp(inst,"ADDA"))  {
        // 右辺が配列変数の加算は、加算命令ではなくLAD命令で行う
        inst3v(LAD,GR%d,%s,GR%d, reg, varName(rhs), reg);
        return;
      }
    }
    switch (rhs->id)  {
    // 右辺が数、文字列、複合式のとき
    case Snum:
    case Sstr:
    case Sexp:
      // 右辺が手続き呼び出しのときは右辺の値をGR0に保存
      // それ以外なら新たな一時変数レジスタに保存
      if (rhs->id == Sexp && rhs->etree.eid == Ecall)
        reg2 = saved = 0;
      else
        saved = allocReg(&info,&reg2);
      // 右辺の値を reg2 番目のレジスタに格納するコードの生成
      emitExp(rhs,reg2,info,StoreValue);
      // 命令 inst による reg と reg2 との演算命令の生成
      inst2v(%s, GR%d, GR%d, inst, reg, reg2);
      popIfNecessary(saved);
      break;
    // 右辺が変数のとき
    case Svar:
      emitVar(inst,rhs->var.entry,reg,info);
      break;
    // 右辺が配列変数のとき
    case Savar:
      inst2v(LAD,GR0,%s, varName(rhs));
      inst2v(%s, GR%d, GR0, inst, reg);
      break;
    // 右辺が添字式のとき
    case Sidx: {
      saved = allocReg(&info,&reg2);
      // 添字式の指すアドレスを reg2 番目のレジスタに格納するコードの生成
      char* addr = emitIdxExp(rhs,reg2,info,StoreValue);
      // 命令 inst によるreg と添字式の実効アドレスとの演算命令の生成
      inst3v(%s,GR%d,%s,GR%d, inst, reg, addr, reg2);
      popIfNecessary(saved); }
    }
  }
}


// レジスタを一つ割り当てて、レジスタ情報を更新する
static int allocReg(RegInfo* info, int* reg)
{
  int saved = 0;	// プッシュしたレジスタの番号(なければ0)

  // 利用可能なレジスタ群の先頭レジスタを割り当てる
  *reg = info->availableHead;
  // 割り当てたレジスタが使用中か？
  if (info->usedHead == info->availableHead)  {
    // レジスタを空けるためにプッシュ命令を生成
    saved = info->usedHead;
    push(saved);
    // 使用済み一時変数レジスタ群の先頭を一つ後ろにずらす
    info->usedHead = nextTmpReg(info->usedHead);
  }
  // 一時変数レジスタ未使用だったなら、最初の一時変数レジスタを使用済にする
  if (info->usedHead == TmpRegNotUsed)
    info->usedHead = FirstTmpReg;
  // 利用可能な一時変数レジスタ群の先頭を一つ後ろにずらす
  info->availableHead = nextTmpReg(info->availableHead);
  // プッシュしたレジスタの番号を返す
  return saved;
}

// 確保したレジスタをいったん手放す（コンマ演算子の第一引数の式を
// 評価するときに行う）
static void rollBackRegAlloc(RegInfo info, RegInfo* rollBack)
{
  rollBack->availableHead = prevTmpReg(info.availableHead);
  rollBack->usedHead = info.usedHead;
}

// 指定されたレジスタを割り当てる
int allocBuiltinRegs(int num)
{
  int n = min(argRegNum,num), i;
  for (i=1; i<=n; i++)
    push(i);
  return n;
}

// 定数 num と式 exp の乗算コードの生成
static void emitMulExpNum(Tree* exp, int num, int reg, RegInfo info,
                          bool trashVal)
{
  int c = 0;
  // 式のコード生成
  emitExp(exp,reg,info,trashVal);
  if (!trashVal)  {
    // GR0 を0で初期化
    inst2(LAD, GR0, 0);
    if (num < 0) num = (1 << WordSize) + num;
    while (num)  {
      if (num & 1)  {
        if (c) inst2v(SLL, GR%d, %d, reg, c);
        inst2v(ADDL, GR0, GR%d, reg);
        c = 0;
      }
      num >>= 1;
      ++c;
    }
    inst2v(LD, GR%d, GR0, reg);
  }
}


/*********************************/
/*  組込関数生成補助関数群 */
/*********************************/

void emitBuiltins()
{
  if (useMult)   emitMultiplication();
  if (useDiv)    emitDivision();
  if (useMod)    emitModulo();
  if (useDivRem) emitDivisionWithRemainder();
}

typedef struct {
  char* name;
  int argc;
  int id;
  void (*sysproc)();
} SysProc;

static SysProc systems[] = {
  { "setVector",      1, 0, emitSetVector },
  { "enableIntr",     0, 1, emitEnableIntr },
  { "disableIntr",    0, 2, emitDisableIntr },
  { "storeRegisters", 0, 3, emitStoreRegisters } };

void initSystemProcedures()
{
  int n = sizeof(systems)/sizeof(SysProc);
  int i;
  for (i=0; i<n; i++)  {
    char* nameLab = generateNameLabel(systems[i].name);
    addSystemProc(nameLab,systems[i].argc,systems[i].id);
  }
}

void emitSystemProcedure(int id)
{
  (systems[id].sysproc)();
}

/************************/
/* 組込み関数群の定義   */
/************************/

// 割り込みベクタ領域の先頭アドレスGR1の値をGR14に格納
static void emitSetVector()
{
  printf("%s%s\n", IdLabelHeader, systems[0].name);
  inst2(LD, GR14, GR1);
  inst0(RET);
}

// 割り込み許可
// GR1とGR2は破壊される
static void emitEnableIntr()
{
  printf("%s%s\n", IdLabelHeader, systems[1].name);
  inst0(PUSHF);
  inst3(LD, GR1, 0, GR15);
  inst2v(LAD,GR2,#%x, QF);
  inst2(OR,GR1,GR2);
  inst3(ST,GR1,0,GR15);
  inst0(POPF);
  inst0(RET);
}

// 割り込み禁止
// GR1とGR2は破壊される
static void emitDisableIntr()
{
  printf("%s%s\n", IdLabelHeader, systems[2].name);
  inst0(PUSHF);
  inst3(LD, GR1, 0, GR15);
  inst2v(LAD,GR2,#%X, NotQF);
  inst2(AND,GR1,GR2);
  inst3(ST, GR1, 0, GR15);
  inst0(POPF);
  inst0(RET);
}

// 16個の要素を持つ配列GRにレジスタの値を保存（デバッグ用）
static void emitStoreRegisters()
{
  printf("%s%s\n", IdLabelHeader, systems[3].name);
  inst2(ST, GR0, _GR);		// GR0の値をGR[0]に保存
  inst2(LD, GR0, GR1);		// GR1の値をGR0に退避
  inst2(LAD, GR1, 1);		// GR1の値を1にする
  inst3(ST, GR0, _GR, GR1);	// GR0に保存されたGR1の値をGR[1]に保存
  inst3(LAD, GR1, 1, GR1);	// GR1の値を2にする
  inst3(ST, GR2, _GR, GR1);	// GR2の値をGR[2]に保存
  inst3(LAD, GR1, 1, GR1);	// 以下同様に、レジスタの値を配列GRに保存する
  inst3(ST, GR3, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR4, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR5, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR6, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR7, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR8, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR9, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR10, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR11, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR12, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR13, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR14, _GR, GR1);
  inst3(LAD, GR1, 1, GR1);
  inst3(ST, GR15, _GR, GR1);	// GR15の値をGR[15]に保存
  inst2(LD, GR1, GR0);		// GR0に保存されたGR1の値を復帰
  inst2(LD, GR0, _GR);		// GR[0]に保存されたGR0の値を復帰
  inst0(RET);
}	

// GR1とGR2の乗算。
// 結果はGR0に格納される。
static void emitMultiplication()
{
  printf("@multiply\n");
  inst2(LAD, GR0, 0);
  int loop = newILabel();
  emitInternalILabel(loop);
  inst2(AND, GR2, GR2);
  int end = newILabel();
  inst1v(JZE, %s%d, ILabelHeader, end);
  inst2(SRL, GR2, 1);
  int add = newILabel();
  inst1v(JOV, %s%d, ILabelHeader, add);
  int next = newILabel();
  emitInternalILabel(next);
  inst2(SLL, GR1, 1);
  inst1v(JUMP, %s%d, ILabelHeader, loop);
  emitInternalILabel(add);
  inst2(ADDL, GR0, GR1);
  inst1v(JUMP, %s%d, ILabelHeader, next);
  emitInternalILabel(end);
  inst0(RET);
}

// GR1のGR2による除算
// 結果はGR0に格納される。
// GR0からGR4は破壊される。
static void emitDivision()
{
  printf("@divide\n");
  // 商の符号を得るコードの生成
  inst2(LD, GR3, GR1);
  inst2(XOR, GR3, GR2);
  inst2(SLL, GR3, 1);
  int lab1 = newILabel();
  // 商の符号をチェックする命令の生成
  inst1v(JOV, %s%d, ILabelHeader, lab1);
  // 商の符号が正になるとき
  int lab2 = newILabel();
  // 除算ルーチン呼び出しコードの生成(GR1に商の絶対値が返る)
  inst1(CALL, @divrem);
  inst1v(JUMP, %s%d, ILabelHeader, lab2);
  // 商の符号が負になるとき
  emitInternalILabel(lab1);
  // 除算ルーチン呼び出しコードの生成(GR1に商の絶対値が返る)
  inst1(CALL, @divrem);
  // GR1の符号反転コードの生成
  _emitNeg(1,3);
  emitInternalILabel(lab2);
  // GR0 に商を保存
  inst2(LD, GR0, GR1);
  inst0(RET);
}

// GR1のGR2による剰余算
// 結果はGR0に格納される。
// GR0からGR4は破壊される。
static void emitModulo()
{
  printf("@modulo\n");
  // 余りの符号を得るコードの生成
  inst2(LD, GR3, GR1);
  inst2(SLL, GR3, 1);
  int lab1 = newILabel();
  // 余りの符号をチェックする命令の生成
  inst1v(JOV, %s%d, ILabelHeader, lab1);
  // 余りの符号が正になるとき
  int lab2 = newILabel();
  // 除算ルーチン呼び出しコードの生成(GR4に余りの絶対値が返る)
  inst1(CALL, @divrem);
  inst1v(JUMP, %s%d, ILabelHeader, lab2);
  // 余りの符号が負になるとき
  emitInternalILabel(lab1);
  // 除算ルーチン呼び出しコードの生成(GR4に余りの絶対値が返る)
  inst1(CALL, @divrem);
  // GR4の符号反転コードの生成
  _emitNeg(4,3);
  emitInternalILabel(lab2);
  // 余りを GR0 に保存
  inst2(LD, GR0, GR4);
  inst0(RET);
}

// GR1のGR2による除算と剰余算。
// 商はGR1、余りはGR4、GR0の下位１ビットに商の符号、
// GR0の下位2ビットに余りの符号がそれぞれ格納される。
// レジスタGR3をカウンタとして用いる。
// GR0からGR4は破壊される。
static void emitDivisionWithRemainder()
{
  printf("@divrem\n");
  // 第二引数がゼロなら商にゼロを、
  // 余りに第一引数の値を格納して終了
  inst2(AND, GR2, GR2);
  int lab1 = newILabel();
  inst1v(JNZ, %s%d, ILabelHeader, lab1);
  inst2(LD, GR4, GR1);
  inst2(LAD, GR1, 0);
  inst2(LAD, GR0, 0);
  inst0(RET);
  // 第二引数がゼロでないとき
  emitInternalILabel(lab1);
  // 余りの初期値の元となる値としてGR4に第二引数をコピー
  inst2(LD, GR4, GR2);
  int arg2Plus = newILabel();
  inst1v(JPL, %s%d, ILabelHeader, arg2Plus);
  // 第二引数が負のとき、
  //   余りの初期値GR4は第二引数(既にコピー済み)。
  //   第二引数を符号反転する。
  _emitNeg(2,3);
  int chkArg1 = newILabel();
  inst1v(JUMP, %s%d, ILabelHeader, chkArg1);
  // 第二引数が正のとき、
  //   余りの初期値は第二引数の符号反転なので、GR4を符号反転
  emitInternalILabel(arg2Plus);
  _emitNeg(4,3);
  emitInternalILabel(chkArg1);
  // 第一引数が0なら商(GR1)と余り(GR4)を0にしてリターン。
  inst2(AND, GR1, GR1);
  int arg1Nonzero = newILabel();
  inst1v(JNZ, %s%d, ILabelHeader, arg1Nonzero);
  inst2(LAD, GR4, 0);
  inst0(RET);
  emitInternalILabel(arg1Nonzero);
  int arg1Plus = newILabel();
  inst1v(JPL, %s%d, ILabelHeader, arg1Plus);
  // 第一引数が負なら符号反転する
  _emitNeg(1,3);
  // (余り:商) を左に１ビットシフト
  emitInternalILabel(arg1Plus);
  inst2(SLL, GR4, 1);
  inst2(SLL, GR1, 1);
  int carry1 = newILabel();
  inst1v(JOV, %s%d, ILabelHeader, carry1);
  int initAdd = newILabel();
  inst1v(JUMP, %s%d, ILabelHeader, initAdd);
  // 余りの初期値に第二引数の絶対値を加える
  emitInternalILabel(carry1);
  inst3(LAD, GR4, 1, GR4);
  emitInternalILabel(initAdd);
  inst2(ADDA, GR4, GR2);
  // カウンタの初期設定
  inst2(LAD, GR3, 15);
  // ここからメインループ
  int loop = newILabel();
  emitInternalILabel(loop);
  inst2(AND, GR3, GR3);
  // カウンタが０ならループを抜ける
  int end = newILabel();
  inst1v(JZE, %s%d, ILabelHeader, end);
  // 余りを左に１ビットシフト
  inst2(SLL, GR4, 1);
  // シフト前の余りは負か？
  int add1 = newILabel();
  inst1v(JOV, %s%d, ILabelHeader, add1);
  // シフト前の余りが負でないなら
  //   商の最下位ビットを１にする
  inst3(LAD, GR1, 1, GR1);
  //   商を左に１ビットシフト後、余りから第二引数の絶対値を引く
  inst2(SLL, GR1, 1);
  int sub1 = newILabel();
  inst1v(JOV, %s%d, ILabelHeader, sub1);
  int sub2 = newILabel();
  inst1v(JUMP, %s%d, ILabelHeader, sub2);
  emitInternalILabel(sub1);
  inst3(LAD, GR4, 1,GR4);
  emitInternalILabel(sub2);
  inst2(SUBA, GR4, GR2);
  int decCnt = newILabel();
  inst1v(JUMP, %s%d, ILabelHeader, decCnt);
  // シフト前の余りが負なら
  //   商の最下位ビットを０にする(既に０なので何もしない)
  //   商を左に１ビットシフト後、余りに第二引数の絶対値を加える
  emitInternalILabel(add1);
  inst2(SLL, GR1, 1);
  int add2 = newILabel();
  inst1v(JOV, %s%d, ILabelHeader, add2);
  int add3 = newILabel();
  inst1v(JUMP, %s%d, ILabelHeader, add3);
  emitInternalILabel(add2);
  inst3(LAD, GR4, 1, GR4);
  emitInternalILabel(add3);
  inst2(ADDA, GR4, GR2);
  // カウンタを１減らす
  emitInternalILabel(decCnt);
  inst3(LAD, GR3, -1, GR3);
  inst1v(JUMP, %s%d, ILabelHeader, loop);
  // ループを抜けた後の処理
  emitInternalILabel(end);
  // 余りが負か？
  inst2(AND, GR4, GR4);
  int end2 = newILabel();
  inst1v(JMI, %s%d, ILabelHeader, end2);
  // 余りが負でなければ、商の最下位ビットを１にしてリターン
  inst3(LAD, GR1, 1, GR1);
  inst0(RET);
  // 余りが負なら、余りに第二引数の絶対値を加えてリターン
  emitInternalILabel(end2);
  inst2(ADDA, GR4, GR2);
  inst0(RET);
}

// 識別子名にヘッダを加えてラベルを作る
char* generateNameLabel(char* name)
{
  char *p;
  if ((p=malloc(sizeof(IdLabelHeader)+strlen(name)+1)) == NULL)
    errorExit(EShortOfMemory);
  strcpy(p, IdLabelHeader);		 // ラベルのヘッダをセット
  strcpy(p+strlen(IdLabelHeader), name); // ヘッダの後に識別子名をセット
  return  p;
}


// 引数レジスタに確保する仮引数の変数エントリを保存
void assocParamEntry(int index, VarEntry* entry)
{
  paramEntry[index-1] = entry;
}

// 引数レジスタの値をスタックに退避したことを変数エントリに記憶
static void saveRegisterVar(int index)
{
  paramEntry[index-1]->flag |= VESaveReg;
}

// 引数レジスタの値をスタックから復帰したことを変数エントリに記憶
static void restoreRegisterVar(int index)
{
  paramEntry[index-1]->flag &= ~VESaveReg;
}

// PUSH命令の生成とオフセットの増加
static void push(int index)
{
  inst2v(PUSH, 0, GR%d, index);
  // 一時退避領域のサイズを一つ増やす
  ++savedAreaSize;
  // 引数レジスタをスタックに退避しているときは、
  // 退避された引数レジスタの値までのオフセットを一つ増やす
  if (offsetSavedRegVar != -1)
    ++offsetSavedRegVar;
}

// PUSH命令の生成とオフセットの減少
static void pop(int index)
{
  inst1v(POP, GR%d, index);
  // 一時退避領域のサイズを一つ減らす
  --savedAreaSize;
  // 引数レジスタをスタックに退避しているときは、
  // 退避された引数レジスタの値までのオフセットを一つ減らす
  if (offsetSavedRegVar != -1)
    --offsetSavedRegVar;
}

// 必要があるときだけ POP命令を生成
static void popIfNecessary(int saved)
{
  if(saved) pop(saved);
}

// スタックを num だけ伸ばす
void extendStack(int num)
{
  inst3v(LAD, GR15, -%d, GR15, num);
  // 一時退避領域のサイズを num だけ増やす
  savedAreaSize += num;
  // レジスタ変数をスタックに退避しているときは、
  // 退避されたレジスタ変数までのオフセットを num だけ増やす
  if (offsetSavedRegVar != -1)
    offsetSavedRegVar += num;
}

// スタックを num だけ縮める
void shrinkStack(int num)
{
  inst3v(LAD, GR15, %d, GR15, num);
  // 一時退避領域のサイズを num だけ減らす
  savedAreaSize -= num;
  // レジスタ変数をスタックに退避しているときは、
  // 退避されたレジスタ変数までのオフセットを num だけ減らす
  if (offsetSavedRegVar != -1)
    offsetSavedRegVar -= num;
}

void emitSystemFunction(int id)
{
}
