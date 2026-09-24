/************************************************

  CASL2C Compiler for COMET-II on FPGA
  header file for constants used in the compiler

  Taro Suzuki 
  Last updated: Mar. 25, 2016

************************************************/

#ifndef __CASL2C_CONST_H__
#define __CASL2C_CONST_H__

// 記号表エントリの種別
typedef enum {
  VarEntryID,		// 変数エントリ
  ProcEntryID		// 手続きエントリ
} SymEntryID;

// 変数の種別
typedef enum {
  GlobalVar,		// 大域変数
  LocalVar,		// 局所変数
  Param			// 仮引数
} VClass;


// 構文木の種別
typedef enum {
  Snum,			// 整数
  Sstr,			// 文字列
  Sarray,		// 配列(配列変数と定数の初期値としてのみ利用)
  Svar,			// 変数
  Savar,		// 配列変数(配列のアドレス)
  Shandler,		// 割り込みハンドラ名
  Sidx,			// 添字式 base[index]
  Sexp,			// 複合式
  Sif,			// if文
  Swhile,		// while文
  Sfor,			// for文
  Sbreak,		// break文
  Sreturn,		// return文
  Sin,			// in文
  Sout,			// out文
  Shalt,		// halt文
  Sseq,			// 文の列
  Sfunc,		// 関数
  Sintr			// 割り込みハンドラ
} STreeID;


// 複合式の構文木の種別
typedef enum {
  Eadd,			// 加算演算子 +
  Esub,			// 減算演算子 -
  Emul,			// 乗算演算子 *
  Ediv,			// 除算演算子 /
  Emod,			// 剰余算演算子 %
  Eneg,			// 符号反転演算子 -
  Epreinc,		// プレインクリメント演算子 ++
  Epredec,		// プレデクリメント演算子 --
  Epostinc,		// ポストインクリメント演算子 ++
  Epostdec,		// ポストデクリメント演算子 --
  Eeq,			// 等値演算子 ==
  Eneq,			// 非等値演算子 !=
  Egt,			// 関係演算子 >
  Elt,			// 関係演算子 <
  Egeq,			// 関係演算子 >=
  Eleq,			// 関係演算子 <=
  Eand,			// 論理積演算子 &&
  Eor,			// 論理和演算子 ||
  Enot,			// 論理否定演算子 !
  Eband,		// ビット積演算子 &
  Ebor,			// ビット和演算子 |
  Ebxor,		// ビット排他和演算子 ^
  Ebnot,		// ビット反転演算子 ~
  Elshift,		// 左シフト演算子 <<
  Ershift,		// 右シフト演算子 >>
  Eaddr,		// アドレス演算子 &
  Ederef,		// ポインタ演算子 *
  Eassign,		// 代入
  Ecall,		// 手続き呼び出し ID( )
  Ecomma		// コンマ演算子
} ETreeID;

// COMET-II に関する定数
#define MaxUnsignedNum	65535	// 符号無し整数の最大値
#define MinUnsignedNum	0	// 符号無し整数の最小値
#define MaxSignedNum	32767	// 符号付き整数の最大値
#define MinSignedNum	-32768	// 符号付き整数の最小値
#define WordSize  16		// 一語のビット長

// 変数エントリのフラグ
#define VENoFlag	0	// フラグが無いことを表す
#define VEArray		1	// 配列
#define VERegister	2	// レジスタ変数
#define VESaveReg	4	// レジスタ変数をスタックに退避

// 手続きエントリのフラグ
#define PENoFlag	0	// フラグが無いことを表す
#define PEDefined	1	// 手続きが定義済
#define PESystem	2	// システム定義手続き
#define PEUsed		4	// 手続き呼び出しが存在する
#define PEhandler   	8	// 割り込みハンドラ

// レジスタに関する定数
#define MaxReg		13	// 一時変数に利用可能なレジスタの最大の番号
#define MaxArgRegNum	4	// 引数に利用可能なレジスタの最大の番号
#define AllRegNum	15	// 全レジスタの個数
#define IDSetRegister	0
#define IDGetRegister	1
#define IDInterrupt	2
#define IDPrintNumber	3

// ラベルに関する定数
#define IdLabelHeader	"_"	// 識別子名から作られるラベルのヘッダ

#endif
