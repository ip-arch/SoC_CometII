/****************************************************

  CASL2C Compiler for COMET-II on FPGA
  header file for scanner genrated from casl2c.l

  Taro Suzuki 
  Last updated: Mar. 9, 2016

****************************************************/

#ifndef __CASL2C_SCANNER_H__
#define __CASL2C_SCANNER_H__

// casl2c.l で定義されている外部変数
extern int lineNo;	// ソースファイル中のトークンの行番号

// casl2c.l で定義されている関数のプロトタイプ宣言
int yylex(void);
void initScanner(char *filename);

#endif
