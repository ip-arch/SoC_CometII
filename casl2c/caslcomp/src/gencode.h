/************************************************

  CASL2C Compiler for COMET-II on FPGA
  header file for code generation

  Taro Suzuki
  Last updated: Mar. 25, 2016

************************************************/

#ifndef __CASL2C_GENCODE_H__
#define __CASL2C_GENCODE_H__

#include "type.h"

// gencode.c で定義されている関数のプロトタイプ宣言
void emitStart(void);
void emitEnd(void);
void emitCode(Tree *tree);
void emitData();
void emitBuiltins();
void assocParamEntry(int index, VarEntry* entry);
void initSystemProcedures(void);
void emitSystemProcedure(int id);
char* generateNameLabel(char* name);

#endif
