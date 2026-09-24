/****************************************************************

  CASL2C Compiler for COMET-II on FPGA
  header file for error handling in the compiler

  Taro Suzuki 
  Last updated: Mar. 9, 2016

****************************************************************/

#ifndef __CASL2C_ERROR_H__
#define __CASL2C_ERROR_H__

#include <stdarg.h>	// 可変個の引数をもつ関数のためのインクルードファイル


// エラーの種類を表す定数
typedef enum {
  EFileNotFound,	// 原始プログラムのファイルが見つからない
  ETooLessCmdArgs,	// コマンドライン引数が足りない
  ENoInputFile,		// コマンドライン引数でファイル名が指定されていない
  EShortOfMemory,	// メモリが足りない
  EVarNotConstExp,	// 定数式ではない変数(配列以外の変数名)
  EProcNotConstExp,	// 定数式ではない手続き(通常の手続き名)
  EVarNotFound,		// 変数が未宣言
  EProcNotFound,	// 関数が未宣言
  EProcNotDefined,	// 関数が未定義
  EVarDuplicated, 	// 変数の二重宣言
  EProcDuplicated,	// 関数の二重宣言
  EProcDefDuplicated,	// 関数の二重定義
  EAlreadyAsVar,	// 既に変数として宣言されている
  EAlreadyAsProc,	// 既に関数として宣言されている
  EArraySizeUndefined,	// 配列のサイズが定義されていない
  ETooLongInitArray,	// 配列の初期値のサイズが定義より長い
  ECallHandler,		// 通常の手続きから割り込みハンドラを呼んだ
  EProcDeclaredAsVar,	// 変数を関数として使用
  EVarDeclaredAsProc,	// 関数を変数として使用
  EArrayDeclaredAsVar,	// 変数を配列として使用
  EVarDeclaredAsArray,	// 配列を変数として使用
  EParamNumMismatch,	// 引数個数の不一致
  EInNotArray,		// int命令の第一引数が配列でない
  EInShortArray,	// int命令の第一引数の配列長が不十分
  EIONotGlobalVar,	// in,out命令の第二引数が大域変数でない
  EUnexpectedEOF,	// 予期しないEOF
  EIllegalChar,		// 不正な文字を読み込んだ
  EOverflowNum,		// 数値のオーバーフロー
  EOverflowHex,		// 16進数の桁数が多すぎる
  ETooLongString,	// 長すぎる文字列
  EIllegalString,	// 不正な文字を含む文字列
  EUnfinishedString,	// 文字列の途中でファイルが終わった
  EUnfinishedComment,	// コメントの途中でファイルが終わった
  EUnfinishedFilename,	// ファイル名の途中で改行かEOF
  ETooDeepIncludeNest,	// インクルードのネストが深すぎる
  ELoopNestTooDeep,	// ループのネストが深すぎる
  EBreakOutsideLoop,	// breakがループの外にある
  EDivisionByZero,	// 定数を0で割った
  ENegativeShift,	// シフトの回数が負の数
  EAddrOfParameter,	// 仮引数にアドレス演算子 & を適用した
  EAddrNonLvalue,	// 左辺値以外にアドレス演算子 & を適用した
  EModifyNonLvalue,	// 左辺値以外に演算子 ++, --を適用した
  EAssignNonLvalue,	// 左辺値以外への代入を行った
  EParseError		// 構文エラー
}  Error;

// error.ccで定義されている関数のプロトタイプ宣言

void compileError(Error id, ...);	// コンパイルエラーの処理
void compileErrorWithLine(int line, Error id, ...);
void errorExit(Error id, ...);		// コンパイルエラー以外のエラー処理
void setFilename(const char* name);	// ファイル名の保存
void renewFilename(const char* name);	// ファイル名の更新
void restoreFilename(void);		// ファイル名の復帰
int yyerror(char* msg);			// Yaccプログラム用のエラー処理関数


#endif
