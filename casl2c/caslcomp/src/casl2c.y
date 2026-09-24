/************************************************

  CASL2C Compiler for COMET-II on FPGA
  Yacc source code
  Taro Suzuki
  Last updated: Mar 25, 2016

************************************************/

// C宣言部
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "scanner.h"
#include "const.h"
#include "type.h"
#include "tree.h"
#include "error.h"
#include "symtable.h"
#include "list.h"
#include "gencode.h"


// 相続属性
static int loopNest = 0;	// while, forループのネストの深さ
static bool intr = false;	// 割り込みハンドラの本体の中ならtrue

static void usage(char *cmd);
static Tree* generateLabelConst(char* name);
static Tree* generateArrayNode(XList* xlist);
static Tree* generateIOTree(STreeID id, Tree* buff, Tree* len);
static Tree* generateExp(XList* list);
static Tree* generateBinArithExp(ETreeID id, Tree* e1, Tree* e2);
static Tree* generateUniArithExp(ETreeID id, Tree* exp);
static int evalBinConstExp(ETreeID id, int v1, int v2);
static int evalUniConstExp(ETreeID id, int v);
static Tree* generateAssignTree(Tree* lhs, Tree* rhs);
static Tree* generateModifyExp(ETreeID eid, Tree* exp);
static Tree* generateAddrTree(Tree* lvalue);
//static Tree* generateCallTree(char* name, XList* args);
static Tree* generateCallTree(char* name, PList* args);
static XList* addSTreeMaybe(XList* xlist, Tree* tree);
static XList* addSTree(XList* xlist, Tree* elem);
//static XList* addXListMaybe(XList* xlist, Tree* tree);
//static XList* addSSeq(XList* xlist, Tree* elem);
static void checkBreakOcc(void);
static int toSignedWord(int num);
static int toUnsignedNum(int num);
static int divideConstant(int n1, int n2);
static int moduloConstant(int n1, int n2);
static char* genNameLab(char* name);

%}

%union {
  int value;		// データ値
  unsigned int len;	// 長さ
  char* name;		// 識別子名
  char* str;		// 文字列
  int num;		// 数
  Tree* ast;		// 構文木
  ETreeID etype;	// 式の構文木の種別
  XList* xlist;         // 末尾に追加可能なリスト
  PList* list;		// 構文木のリスト
  ProcEntry* pentry;	// 手続きエントリ
};

%type <ast> stmt estmt stmtList lvar cexp
%type <ast> exp asgnexp binexp uexp postexp primexp variable
%type <ast> procDef intrDef procDefCommon
%type <num> cnum
%type <xlist> elemList stmtList1 body argList1 commaexp
%type <list> argList
%type <ast> vinit ainit
%type <xlist> localDecl lvarList
%type <len> paramList paramList2
%type <etype > premodify postmodify
%type <name> idLab

%token <value> NUM
%token <name> ID
%token <str> STR
%token <etype> ADDOP RELOP EQOP SHIFT
%token INC DEC
%token VAR PROC INTR
%token IF WHILE FOR RETURN BREAK IN OUT HALT

%nonassoc IFX
%nonassoc ELSE

%right '='
%left LOR
%left LAND
%left '|'
%left '^'
%left '&'
%left EQOP
%left RELOP
%left SHIFT
%left ADDOP
%left '*' '/' '%'
%right SIGNOP '~' '!' ADDR PNT


%%

// プログラム
program
  : decl
  | procDef             { emitCode($1); deleteTree($1); cleanProcDef(); }
  | intrDef             { emitCode($1); deleteTree($1); cleanProcDef(); }
  | program decl
  | program procDef     { emitCode($2); deleteTree($2); cleanProcDef(); }
  | program intrDef     { emitCode($2); deleteTree($2); cleanProcDef(); }
  ;

// 宣言(変数宣言と手続きのプロトタイプ宣言)
decl
  : VAR gvarList ';'
  | PROC procList ';'
  ;

// 大域変数宣言リスト
gvarList
  : gvar
  | gvarList ',' gvar
  ;

// 大域変数宣言
gvar
  : idLab vinit             { addGlobalVar($1,$2); }
  | idLab '[' NUM ']' ainit { addArray($1,$3,$5); }
  | idLab '[' ']' ainit     { addArray($1,0,$4); }
  ;

// 単純変数の初期値
vinit
  : /* empty */             { $$ = NULL; }
  | '=' cexp                { $$ = $2; }
  | '=' STR                 { $$ = newStrNode($2); }
  ;

// 配列変数の初期値
ainit
  : /* empty */             { $$ = NULL; }
  | '=' '{' elemList '}'    { $$ = generateArrayNode($3);
                              deleteXList($3); }
  | '=' STR                 { $$ = newStrNode($2); }
  ;

// 配列要素のリスト
elemList	/* 配列要素を出現順に並べたリストを作る */
  : cexp                    { $$ = addSTree(newXList(),$1); }
  | elemList ',' cexp       { $$ = addSTree($1,$3); }
//  : cexp                    { $$ = addSSeq(newXList(),$1); }
//  | elemList ',' cexp       { $$ = addSSeq($1,$3); }
  ;

// 定数式（コンパイル時に評価される）
// 識別子は配列変数名と割込みハンドラ名のみ利用可能。これらはアセンブル時に
// アドレスが決まって定数になる。ただし、これらの識別子を演算子の引数とする
// ことは不可（アセンブラが演算子を含むDC命令に対応していないため）。
cexp
  : cnum                    { $$ = newNumNode(toSignedWord($1)); }
  | idLab                   { $$ = generateLabelConst($1); }

cnum
  : NUM                     { $$ = $1; }
  | cnum ADDOP cnum         { $$ = evalBinConstExp($2,$1,$3); }
  | cnum '*' cnum           { $$ = evalBinConstExp(Emul,$1,$3); }
  | cnum '/' cnum           { $$ = evalBinConstExp(Ediv,$1,$3); }
  | cnum '%' cnum           { $$ = evalBinConstExp(Emod,$1,$3); }
  | cnum '&' cnum           { $$ = evalBinConstExp(Eband,$1,$3); }
  | cnum '|' cnum           { $$ = evalBinConstExp(Ebor,$1,$3); }
  | cnum '^' cnum           { $$ = evalBinConstExp(Ebxor,$1,$3); }
  | cnum EQOP cnum          { $$ = evalBinConstExp($2,$1,$3); }
  | cnum RELOP cnum         { $$ = evalBinConstExp($2,$1,$3); }
  | '~' cnum                { $$ = evalUniConstExp(Ebnot,$2); }
  | ADDOP cnum %prec SIGNOP { $$ = $1==Eadd ? $2 : evalUniConstExp(Eneg,$2); }
  | cnum LAND cnum          { $$ = evalBinConstExp(Eand,$1,$3); }
  | cnum LOR cnum           { $$ = evalBinConstExp(Eor,$1,$3); }
  | '!' cnum                { $$ = evalUniConstExp(Enot,$2); }
  | cnum SHIFT cnum         { $$ = evalBinConstExp($2,$1,$3); }
  | '(' cnum ')'            { $$ = $2; }
  ;

// 手続き宣言リスト
procList
  : procDecl
  | procList ',' procDecl
  ;

// 手続き宣言
procDecl
  : idLab '/' NUM        { addProc($1,$3); }
  ;

// 手続き定義 (PROC は省略可能)
procDef
  : PROC procDefCommon   { $$ = $2; }
  | procDefCommon        { $$ = $1; }
  ;

// 割り込みハンドラ定義
intrDef
  : INTR                 { intr = true; }
    procDefCommon        { intr = false; $$ = $3; }
  ;

// 手続き定義本体
procDefCommon
  : idLab '('            { initProcDef(); }
    paramList            { $<pentry>$ = defineProc($1,$4,intr); }
    ')' '{' localDecl
    body '}'             { PList* list = getXListBody(appendXList($8,$9));
                           Tree* code = newSeqTree(list);
                           deleteXList($8); deleteXList($9);
                           $$ = newProcTree($<pentry>5,code); }
  ;

// 仮引数リスト
paramList
  : /* 空列 */        { $$ = 0; }
  | paramList2        { $$ = $1; }

paramList2
  : idLab                       { addParam($1); $$ = 1; }
  | paramList ',' idLab         { addParam($3); $$ = $1 + 1; }
  ;

// 局所変数宣言の並び
localDecl
  : /* empty */                 { $$ = newXList(); }
  | localDecl VAR lvarList ';'  { $$ = appendXList($1,$3); }
  ;

// 局所変数宣言
lvarList
  : lvar              { $$ = addSTreeMaybe(newXList(),$1); }
  | lvarList ',' lvar { $$ = addSTreeMaybe($1,$3); }
  ;

// 局所変数
lvar
  : idLab             { addLocalVar($1); $$ = NULL; }
  | idLab '=' asgnexp { VarEntry* entry = addLocalVar($1);
                        $$ = newETree(Eassign,newVarNode(entry),$3); }
  ;

// 手続き本体
body
  : /* empty */   { $$ = newXList(); }
  | body stmt     { addSTree($1,$2); }
  ;

// 文
stmt
  : estmt                               { $$ = $1; }
  | IF '(' exp ')' stmt %prec IFX       { $$ = newSTree(Sif,$3,$5); }
  | IF '(' exp ')' stmt ELSE stmt       { $$ = newSTree(Sif,$3,$5,$7); }
  | FOR '(' estmt estmt exp ')'         { loopNest++; }
    stmt                                { $$ = newSTree(Sfor,$3,$4,$5,$8);
                                          loopNest--; }
  | FOR '(' estmt estmt ')'             { loopNest++; }
    stmt                                { $$ = newSTree(Sfor,$3,$4,NULL,$7);
                                          loopNest--; }
  | WHILE '(' exp ')'                   { loopNest++; }
    stmt                                { $$ = newSTree(Swhile,$3,$6);
                                          loopNest--; }
  | RETURN exp ';'                      { $$ = newSTree(Sreturn,$2); }
  | RETURN ';'                          { $$ = newSTree(Sreturn); }
  | BREAK ';'                           { checkBreakOcc();
                                          $$ = newSTree(Sbreak); }
  | IN variable ',' variable ';'        { $$ = generateIOTree(Sin,$2,$4); }
  | OUT variable ',' variable ';'       { $$ = generateIOTree(Sout,$2,$4); }
  | HALT ';'                            { $$ = newSTree(Shalt); }
  | '{' stmtList '}'                    { $$ = $2; }
  ;

// 文リスト(空かもしれない)
stmtList
  : /* empty */     { $$ = NULL; }
  | stmtList1       { $$ = newSeqTree(getXListBody($1));
                      deleteXList($1); }

// 空でない文リスト
stmtList1	/* 文を出現順に並べたリストを作る */
  : stmt            { $$ = addSTree(newXList(),$1); }
  | stmtList1 stmt  { $$ = addSTree($1,$2); }
  ;

// 式文
estmt
  : ';'             { $$ = NULL; }
  | exp ';'         { $$ = $1; }


// 式（カンマ演算子を含むすべての右辺値と、代入を含む）
exp
  : commaexp                  { $$ = generateExp($1);
                                deleteXList($1); }
  ;

commaexp
  : asgnexp                   { $$ = addSTree(newXList(),$1); }
  | commaexp ',' asgnexp      { $$ = addSTree($1,$3); }
  ;

asgnexp
  : binexp                    { $$ = $1; }
  | uexp '=' asgnexp          { $$ = generateAssignTree($1,$3); }
  ;

binexp
  : uexp                      { $$ = $1; }
  | binexp LAND binexp        { $$ = newETree(Eand,$1,$3); }
  | binexp LOR binexp         { $$ = newETree(Eor,$1,$3); }
  | binexp '|' binexp         { $$ = generateBinArithExp(Ebor,$1,$3); }
  | binexp '^' binexp         { $$ = generateBinArithExp(Ebxor,$1,$3); }
  | binexp '&' binexp         { $$ = generateBinArithExp(Eband,$1,$3); }
  | binexp EQOP binexp        { $$ = generateBinArithExp($2,$1,$3); }
  | binexp RELOP binexp       { $$ = generateBinArithExp($2,$1,$3); }
  | binexp SHIFT binexp       { $$ = generateBinArithExp($2,$1,$3); }
  | binexp ADDOP binexp       { $$ = generateBinArithExp($2,$1,$3); }
  | binexp '*' binexp         { $$ = generateBinArithExp(Emul,$1,$3); }
  | binexp '/' binexp         { $$ = generateBinArithExp(Ediv,$1,$3); }
  | binexp '%' binexp         { $$ = generateBinArithExp(Emod,$1,$3); }
  ;

uexp
  : postexp                   { $$ = $1; }
  | '~' uexp                  { $$ = generateUniArithExp(Ebnot,$2); }
  | ADDOP uexp %prec SIGNOP   { $$ = generateUniArithExp($1,$2); }
  | '&' uexp %prec ADDR       { $$ = generateAddrTree($2); }
  | '*' uexp %prec PNT        { $$ = newETree(Ederef,$2); }
  | premodify uexp            { $$ = generateModifyExp($1,$2); }
  | '!' uexp                  { $$ = generateUniArithExp(Enot,$2); }
  ;

postexp
  : primexp                   { $$ = $1; }
  | postexp postmodify        { $$ = generateModifyExp($2,$1); }
  ;

primexp
  : variable                  { $$ = $1; }
  | primexp '[' exp ']'       { $$ = newIdxTree($1,$3); }
  | idLab '(' argList ')'     { $$ = generateCallTree($1,$3); }
  | STR                       { $$ = newStrNode($1); }
  | NUM                       { $$ = newNumNode(toSignedWord($1)); }
  | '(' exp ')'               { $$ = $2; }
  ;

variable
  : idLab                     { $$ = newVarNode(findVar($1)); }

idLab
  : ID                        { $$ = genNameLab($1); }
  ;

// 後置インクリメント・デクリメント演算子
premodify
  : INC    { $$ = Epreinc; }
  | DEC    { $$ = Epredec; }
  ;

// 前置インクリメント・デクリメント演算子
postmodify
  : INC    { $$ = Epostinc; }
  | DEC    { $$ = Epostdec; }
  ;

// 実引数リスト(空かもしれない)
argList
  : /* empty */      { $$ = NULL; }
  | argList1         { $$ = getXListBody($1); deleteXList($1); }
  ;

// 空でない実引数リスト
argList1	/* 実引数を出現順に並べたリストを作る */
  : asgnexp               { $$ = addSTree(newXList(),$1); }
  | argList1 ',' asgnexp  { $$ = addSTree($1,$3); }
  ;

%%

// ラベル定数の構文木生成
static Tree* generateLabelConst(char* name)
{
  SymbolEntry* entry = findGlobalSymbol(name);
  // 名前が配列名なら、配列名の構文木を生成
  if (isVarEntry(entry))  {
    if  (isArray((VarEntry*)entry))
      return newVarNode((VarEntry*)entry);
    else
      compileError(EVarNotConstExp,entry->var.name);
  }
  // 名前が割り込みハンドラなら、割り込みハンドラ名の構文木を生成
  else  {
    if (isHandler((ProcEntry*)entry))
      return newHandlerNode((ProcEntry*)entry);
    else
      compileError(EProcNotConstExp,entry->proc.name);
  }
  // それ以外の名前ならエラー
}

// 配列の構文木生成
static Tree* generateArrayNode(XList* xlist)
{
  //  SeqTree* list = getXListBody(xlist);
  PList* list = getXListBody(xlist);
  int length = getXListLen(xlist);
  return newArrayNode(list,length);
}


// in文、out文の構文木生成
// buff と len は変数の構文木
static Tree* generateIOTree(STreeID id, Tree* buff, Tree* len)
{
  if (id == Sin)  {
    // in文の第一引数が配列でなければエラー
    if (!isArray(buff->var.entry))
      compileError(EInNotArray);
    // in文の第一引数の配列長が256以下ならエラー
    if (arraySize(buff->var.entry) <= 256)
      compileError(EInShortArray,arraySize(buff->var.entry));
  }
  // out文の第一引数がグローバル変数でなければエラー
  // (out文の第一引数は配列でなくてもよいが、
  //  第二引数の値が1でないときの動作は保証されない)
  else if (!isGlobalVar(buff->var.entry))
    compileError(EIONotGlobalVar,"out",1);
  // in,out文の第二引数がグローバル変数でなければエラー
  if (!isGlobalVar(len->var.entry))
    compileError(EIONotGlobalVar,(id == Sin ? "in" : "out"),2);

  return newSTree(id,buff,len);
}

// 式の列の長さが２以上ならコンマ演算子の構文木を作る。
// 長さが１なら、列から要素を取り出して返す。
static Tree* generateExp(XList* list)
{
  Tree* exp;
  PList* p = getXListBody(list);
  if (p->next == NULL)  {
    exp = p->data;
    deletePList(p,deleteNothing);
  }
  else
    exp = newETree(Ecomma,(Tree*)p);
  return exp;
}

// 二項演算子をもつ式の構文木生成
static Tree* generateBinArithExp(ETreeID id, Tree* e1, Tree* e2)
{
  if (e1->id == Snum && e2->id == Snum)  {
    int v1 = ((NumNode*)e1)->value;
    int v2 = ((NumNode*)e2)->value;
    deleteTree(e1);
    deleteTree(e2);
    // 32ビット定数の下位16ビットを数とする
    return newNumNode(toSignedWord(evalBinConstExp(id,v1,v2)));
  }
  else
    return newETree(id,e1,e2);
}

// 二項演算子をもつ定数式のコンパイル時計算
static int evalBinConstExp(ETreeID id, int v1, int v2)
{
  int val;
  switch (id)  {
  case Eadd:
    val = v1 + v2;
    break;
  case Esub:
    val = v1 - v2;
    break;
  case Emul:
    val = v1 * v2;
    break;
  case Ediv:
    if (v2 == 0) compileError(EDivisionByZero);
    val = v1 / v2;
    break;
  case Emod:
    if (v2 == 0) compileError(EDivisionByZero);
    val = v1 % v2;
    break;
  case Eband:
    val = (v1 & v2) & 0xffff;
    break;
  case Ebor:
    val = (v1 | v2) & 0xffff;
    break;
  case Ebxor:
    val = (v1 ^ v2) & 0xffff;
    break;
  case Eeq:
    val = (v1 == v2);
    break;
  case Eneq:
    val = (v1 != v2);
    break;
  case Egt:
    val = (v1 > v2);
    break;
  case Elt:
    val = (v1 < v2);
    break;
  case Egeq:
    val = (v1 >= v2);
    break;
  case Eleq:
    val = (v1 <= v2);
    break;
  case Eand:
    val = (v1 && v2);
    break;
  case Eor:
    val = (v1 || v2);
    break;
  case Elshift:
    if (v2 < 0) compileError(ENegativeShift);
    val = (v1 << v2) & 0xffff;
    break;
  case Ershift:
    if (v2 < 0) compileError(ENegativeShift);
    // 右シフトを論理シフトにする
    val = (v1 & 0xffff) >> v2;
    break;
  }
  return val;
}

// 単項演算子をもつ式の構文木生成
static Tree* generateUniArithExp(ETreeID id, Tree* exp)
{
  if (id == Esub) id = Eneg;
  if (id == Eadd) return exp;

  if (exp->id == Snum)  {
    int v = ((NumNode*)exp)->value;
    deleteTree(exp);
    // 32ビット定数の下位16ビットを数とする
    return newNumNode(toSignedWord(evalUniConstExp(id,v)));
  }
  else
    return newETree(id,exp);
}

// 単項演算子をもつ定数式のコンパイル時計算
static int evalUniConstExp(ETreeID id, int v)
{
  int val;
  switch (id)  {
  case Eneg:
    val = -v;
    break;
  case Enot:
    val = !v;
    break;
  case Ebnot:
    val = ~v & 0xffff;
    break;
  }
  return val;
}

// 左辺値(変数、配列要素、*expの形の式)ならtrueを返す
#define isLeftValue(e)  (e->id == Svar || e->id == Sidx || \
                         (e->id == Sexp && e->etree.eid == Ederef))

// 代入の構文木生成
static Tree* generateAssignTree(Tree* lhs, Tree* rhs)
{
   // 左辺は左辺値でなければならない
  if (isLeftValue(lhs))
    return newETree(Eassign,lhs,rhs);
  else
    compileError(EAssignNonLvalue);
}

// ++, -- 式の構文木生成
static Tree* generateModifyExp(ETreeID eid, Tree* exp)
{
  // 引数は左辺値でなければならない
  if (isLeftValue(exp))
    return newETree(eid,exp);
  else  {
    char* name;
    if (eid == Epreinc || eid == Epostinc)
      name = "++";
    else
      name = "--";
    compileError(EModifyNonLvalue,name);
  }
}

// & 式の構文木生成
static Tree* generateAddrTree(Tree* lvalue)
{
  if (lvalue->id == Svar && isParameter(lvalue->var.entry))
    // 仮引数への & の適用はエラー
    compileError(EAddrOfParameter,lvalue->var.entry->name);
  // &*e は e と同じなので e を返す
  else if (lvalue->id == Sexp && lvalue->etree.eid == Ederef)
    return shrinkUnaryExp(lvalue);
  else if (isLeftValue(lvalue))
    return newETree(Eaddr,lvalue);
  else
    compileError(EAddrNonLvalue);
}

// COMETの数にするため、16ビット符号付き整数の範囲に収める
static int toSignedWord(int num)
{
  // 下位２バイトだけを残す
  num &= 0xffff;
  // 下位２バイトの最上位ビットが1なら符号拡張
  if (num & 0x8000)
    return (num | 0xffff0000);
  else
    return num;
}

// 手続き呼び出しの構文木生成
//static Tree* generateCallTree(char* name, XList* args)
static Tree* generateCallTree(char* name, PList* args)
{
  //  int n = getXListLen(args);
  int n = 0;
  PList* p = args;
  while (p)  {
    p = p->next;
    ++n;
  }
  ProcEntry* entry = findProc(name,n);
  Tree* ct = newETree(Ecall,(Tree*)entry,newNumNode(n),(Tree*)args);
  return ct;
}

static XList* addSTreeMaybe(XList* xlist, Tree* tree)
{
  if (tree)
    return addSTree(xlist,tree);
  else
    return xlist;
}

// 構文木の列の末尾に構文木を追加する
static XList* addSTree(XList* xlist, Tree* elem)
{
#if YYDEBUG
  fprintf(stderr,"Enter addSTree(%p,%p)\n", xlist, elem);
#endif
  PList* cell = newPList(elem,NULL);
  return addLast(xlist,cell);
}

// break文の出現がループの中であることをチェック
static void checkBreakOcc()
{
  if (loopNest == 0)
    compileError(EBreakOutsideLoop);
}

// 識別子名からそのラベルを作る
static char* genNameLab(char* name)
{
  char *p = generateNameLabel(name);
  free(name);	// 字句解析系で確保した識別子名の領域を解放
  return  p;
}

static void usage(char* cmd)
{
  char* p = strrchr(cmd,'/');
  if (p == NULL) p = cmd;
  errorExit(ETooLessCmdArgs,p);
}


int main(int argc, char* argv[])
{
#if YYDEBUG
  yydebug = 1;
#endif

  if (argc < 2) usage(*argv);
  if(!argc) errorExit(ENoInputFile);
  char* filename = *++argv;
  initScanner(filename);
  initSymTables();
  emitStart();
  yyparse();
  emitBuiltins();
  checkProgram();
  emitData();
  clearGlobalSymTable();
  emitEnd();
  return 0;
}
