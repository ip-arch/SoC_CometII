/******************************************************

  CASL2C Compiler for COMET-II on FPGA
  header file for symbol tables 

  Taro Suzuki 
  Last updated: Mar. 25, 2015

******************************************************/

#ifndef __CASL2C_SYMTABLE_H__
#define __CASL2C_SYMTABLE_H__

#include <stdbool.h>
#include "type.h"

// 記号表エントリeの種別を判定
#define isVarEntry(e)  ((e)->id == VarEntryID)
#define isProcEntry(e) ((e)->id == ProcEntryID)

// 変数エントリveに対応する変数の種別を判定
#define isGlobalVar(ve) ((ve)->class == GlobalVar)
#define isLocalVar(ve)  ((ve)->class == LocalVar)
#define isParameter(ve) ((ve)->class == Param)
// スタック上の仮引数か、引数レジスタか、引数レジスタから退避された仮引数かを判定
#define isParamReg(ve) (isParameter(ve) && \
                   ((ve)->flag & (VERegister|VESaveReg)) == VERegister)
#define isParamStack(ve) (isParameter(ve) && !((ve)->flag & VERegister))
#define isSavedParamReg(ve) (isParameter(ve) && \
        ((ve)->flag & (VERegister|VESaveReg)) == (VERegister|VESaveReg))

// 変数エントリveに対応する変数が配列か
#define isArray(ve)	((ve)->flag & VEArray)

// 手続きエントリpeは割り込みハンドラに対応するか
#define isHandler(pe)	((pe)->flag & PEhandler)



#define arraySize(ve)	((ve)->index)

// 大域的な記号表から手続きエントリ以外のエントリを順に取り出すための型
typedef struct _symit* SymTableIterator;


// symtable.c で定義されている関数のプロトタイプ宣言
void initSymTables(void);
int getLocalVarNum(void);
int getParamNum(void);
VarEntry* addGlobalVar(char* name, Tree* init);
VarEntry* addArray(char* name, unsigned int size, Tree* init);
VarEntry* addLocalVar(char* name);
VarEntry* addParam(char* name);
ProcEntry* addProc(char* name, int paramc);
ProcEntry* addSystemProc(char* name, int paramc, int id);
ProcEntry* defineProc(char* name, int paramc, bool intr);
void clearGlobalSymTable(void);
void clearLocalSymTable(void);
SymbolEntry* findGlobalSymbol(char* name);
VarEntry* findVar(char* name);
VarEntry* findArray(char* name);
ProcEntry* findProc(char* name, int paramc);
SymTableIterator initSymtableIterator();
SymbolEntry* getNextSymbolEntry(SymTableIterator it);
void markVariable(VarEntry* entry, unsigned int flag);
void checkProgram(void);
void initProcDef(void);
void cleanProcDef(void);

#endif
