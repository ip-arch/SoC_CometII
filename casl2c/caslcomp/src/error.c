/****************************************************************

  CASL2C Compiler for COMET-II on FPGA
  function definitions for error handling

  Taro Suzuki 
  Last updated: Mar. 9, 2016

****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "error.h"


// ファイル内だけで有効な変数
static const char *fileName;		// 原始プログラムのファイル名
static const char *savedFileName;	// ファイル名の保存用バッファ


// エラーメッセージ
static const char *errMsg[] =
  { "ファイルが見つかりません: %s",		// EFileNotFound
    "引数は少なくとも一つ必要です",		// ETooLessCmdArgs
    "ファイル名を指定して下さい",		// ENoInputFile
    "メモリが不足しています",			// EShortOfMemory
    "配列以外の変数名%sを定数式に指定しました",	// EVarNotConstExp
    "通常の手続き名%sを定数式に指定しました",	// EProcNotConstExp
    "変数%sがありません",			// EVarNotFound
    "関数%sがありません",			// EProcNotFound
    "関数%sの定義がありません",			// EProcNotDefined
    "変数%sは既に宣言されています",		// EVarDuplicated
    "関数%sは既に宣言されています",		// EProcDuplicated
    "関数%sは既に定義されています",		// EProcDefDuplicated
    "%sは変数として既に宣言されています",	// EAlreadyAsVar
    "%sは関数として既に宣言されています",	// EAlreadyAsProc
    "配列%sのサイズが指定されていません",	// EArraySizeUndefined
    "配列%s[%d]の初期値が長過ぎます (%d)",	// ETooLongInitArray,
    "関数から割り込みハンドラ%sを呼んでいます",	// ECallHandler
    "%sは関数ではなく変数です",			// EProcDeclaredAsVar
    "%sは変数ではなく関数です",			// EVarDeclaredAsProc
    "%sは配列ではありません",			// EArrayDeclaredAsVar
    "%sは配列です",				// EVarDeclaredAsArray
    "関数%sの引数は%d個でなく%d個必要です",	// EParamNumMismatch
    "in文の第1引数が配列ではありません",		// EInNotArray
    "in文の第1引数の配列長が%dしかありません",	// EInShortArray
    "%sの第%d引数がグローバル変数ではありません",	// EIONotGlobalVar
    "予期しないファイルの終りを検出しました",	// EUnexpectedEOF
    "%c(16進コード: %x)は不正な文字です",	// EIllegalChar
    "%dは範囲外の数です",			// EOverflowNum
    "%sは桁数の上限を超えています",             // EOverflowHex
    "文字列が長過ぎます",			// ETooLongString
    "文字列が不正な文字を含んでいます",		// EIllegalString
    "文字列の途中でファイルが終わりました",	// EUnfinishedString
    "コメントの途中でファイルが終わりました",	// EUnfinishedComment
    "ファイル名が途中で終わりました",		// EUnfinishedFilename
    "インクルードのネストが深すぎます",		// ETooDeepIncludeNest
    "whileループのネストが深すぎます",		// ELoopNestTooDeep
    "breakがループの外にあります",		// EBreakOutsideLoop
    "定数を0で除算しました",			// EDivisionByZero
    "シフトの回数が負の数です",			// ENegativeShift
    "仮引数%sには & は適用できません",		// EAddrOfParameter
    "&を適用できません",			// EAddrNonLvalue
    "%sを適用できません",			// EModifyNonLvalue
    "左辺には値を代入できません",		// EAssignNonLvalue
    "構文エラー: %s"				// EParseError
  };

// エラー処理用共通関数
static void error(Error id, va_list ap)
{
  vfprintf(stderr,errMsg[id],ap);
  fputc('\n',stderr);
  va_end(ap);
  exit(2);
}

// コンパイル中のエラー処理用共通関数
static void compileErrorInternal(int line, Error id, va_list ap)
{
  fprintf(stderr,"%s:%d:",fileName,line);
  error(id,ap);
}

/* 現在の行で検出されたコンパイルエラーの処理	*/
/*   エラーメッセージを表示して終了		*/
/*   id: エラーメッセージの識別番号		*/
/*   ...: エラーメッセージで出力する情報		*/
void compileError(Error id, ...)
{
  va_list argptr;
  va_start(argptr,id);
  compileErrorInternal(lineNo,id,argptr);
}

/* 指定された行で検出されたコンパイルエラーの処理	*/
/*   エラーメッセージを表示して終了		*/
/*   line: コンパイルエラーが検出された行の番号	*/
/*   id: エラーメッセージの識別番号		*/
/*   ...: エラーメッセージで出力する情報		*/
void compileErrorWithLine(int line, Error id, ...)
{
  va_list argptr;
  va_start(argptr,id);
  compileErrorInternal(line,id,argptr);
}

// コンパイル以外のエラー処理を行う関数
void errorExit(Error id, ...)
{
  va_list argptr;

  va_start(argptr,id);
  error(id,argptr);
}

// ファイル名をstatic変数fileNameに保存
void setFilename(const char* name)
{
  fileName = name;
}

// ファイル名をnameに更新し、古いファイル名を保存
void renewFilename(const char* name)
{
  savedFileName = fileName;
  fileName = name;
}

// 保存しておいたファイル名を復帰
void restoreFilename()
{
  fileName = savedFileName;
}

// Yaccが生成する構文解析系が用いるエラー処理関数 yyerror()の定義
int yyerror(char *msg)
{
  // compileErrorを呼び出して終了
  compileError(EParseError,msg);
  // compileError() の中で終了するので，ここへは来ない．
  // error.ccがコンパイルできるようにするためだけに return が必要
  return 0;
}
