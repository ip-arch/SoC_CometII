/************************************************

  CASL2C Compiler for COMET-II on FPGA
  FLex source code
  Taro Suzuki 
  Last updated: Mar. 9, 2016

************************************************/

%{
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "token.h"
#include "const.h"
#include "error.h"

#define MaxStrLen	(1024+1)
#define MaxHexDigit	4

#define closeString() { strBuff[buffIndex] = '\0'; }

int lineNo;
static int lineNoBackUp;

static char strBuff[MaxStrLen];
static int buffIndex;
static FILE* yyinBackUp = NULL;
static YY_BUFFER_STATE yybufferBackUp;

static char* dupname(char* str);
static int atoh(char *str);
static void copyString(char* str);
static void checkSValue(int num);
static void checkUValue(int num);
static int toSignedNum(int num);
%}

%x INCLUDE INCFILE COMMENT COMMENT1 STRING

%option noyywrap

digit    [0-9]
integer  {digit}{digit}*
hexdigit [0-9A-Fa-f]
hex      0[xX]{hexdigit}{hexdigit}*
name     [A-Za-z_][A-Za-z0-9_]*
onechar  [&|~!^*/%(){}\[\];,=]
space    [ \t\r]
strelem  [ !#-&(-\[\]-~]
chrelem  [ !-&(-\[\]-~]

%%

"#include"	{ BEGIN(INCLUDE); }
<INCLUDE>{
  {space}	/* skip */
  "\n"		{ ++lineNo; }
  "\""		{ BEGIN(INCFILE); buffIndex = 0; }
}

<INCFILE>{
  [^\"\n]*	{ copyString(yytext); }
  "\n"		{ compileError(EUnfinishedFilename); return 0; }
  <<EOF>>	{ compileError(EUnfinishedFilename); return 0; }
  "\""		{ 
    closeString();
    if (yyinBackUp)
      errorExit(ETooDeepIncludeNest,strBuff);
    yyinBackUp = yyin; lineNoBackUp = lineNo;
    lineNo = 1; yybufferBackUp = YY_CURRENT_BUFFER;
    if ((yyin = fopen(strBuff,"r")) == NULL)
      errorExit(EFileNotFound,strBuff);
    yy_switch_to_buffer(yy_create_buffer(yyin,YY_BUF_SIZE));
    renewFilename(strBuff);
    BEGIN(INITIAL); }
}

<INITIAL>{
  <<EOF>>	{
    if (!yyinBackUp)
      yyterminate();
    else  {
      yy_delete_buffer(YY_CURRENT_BUFFER);
      yy_switch_to_buffer(yybufferBackUp);
      yyinBackUp = NULL; lineNo = lineNoBackUp;
      restoreFilename();
    }
  }
}

"//"		{ BEGIN(COMMENT1); }
<COMMENT1>{
  [^\n]*	/* skip */
  [\n]		{ ++lineNo; BEGIN(INITIAL); }
  <<EOF>>	{ return 0; }
}

"/*"		{ BEGIN(COMMENT); }
<COMMENT>{
  [^*\n]+	/* skip */
  "*"+"/"	{ BEGIN(INITIAL); }
  "*"+[^*/\n]*	/* skip */
  "\n"		{ ++lineNo; }
  <<EOF>>	{ compileError(EUnfinishedComment); return 0; }
}

"\""		{ BEGIN(STRING); buffIndex = 0; }
<STRING>{
  {strelem}*	{ copyString(yytext); }
  [\\]["]	{ copyString("\""); }
  [\\][n]	{ copyString("\n"); }
  [\\][t]	{ copyString("\t"); }
  <<EOF>>	{ compileError(EUnfinishedString); return 0; }
  "'"		{ copyString("''"); }
  "\""		{ BEGIN(INITIAL); closeString();
		  yylval.str = dupname(strBuff); return STR; }
  .		{ compileError(EIllegalString); return 0; }
}

"'"{chrelem}"'"	{ yylval.num = yytext[1]; return NUM; }
"'"[\\]"''"	{ yylval.num = '\'';      return NUM; }
"'"[\\]"t'"	{ yylval.num = '\t';      return NUM; }
"'"[\\]"n'"	{ yylval.num = '\n';      return NUM; }


"if"		{ return IF; }
"else"		{ return ELSE; }
"while"		{ return WHILE; }
"for"		{ return FOR; }
"break"		{ return BREAK; }
"return"	{ return RETURN; }
"halt"		{ return HALT; }
"var"		{ return VAR; }
"proc"		{ return PROC; }
"interrupt"	{ return INTR; }
"in"		{ return IN; }
"out"		{ return OUT; }

{name}		{ yylval.name = dupname(yytext); return ID; }
{integer}	{ yylval.value = atoi(yytext);   return NUM; }
{hex}		{ yylval.value = atoh(yytext);   return NUM; }
"+"		{ yylval.etype = Eadd;           return ADDOP; }
"-"		{ yylval.etype = Esub;           return ADDOP; }
">="		{ yylval.etype = Egeq;           return RELOP; }
"<="		{ yylval.etype = Eleq;           return RELOP; }
">"		{ yylval.etype = Egt;            return RELOP; }
"<"		{ yylval.etype = Elt;            return RELOP; }
"=="		{ yylval.etype = Eeq;            return EQOP; }
"!="		{ yylval.etype = Eneq;           return EQOP; }
"&&"		{ yylval.etype = Eand;           return LAND; }
"||"		{ yylval.etype = Eor;            return LOR; }
">>"		{ yylval.etype = Ershift;        return SHIFT; }
"<<"		{ yylval.etype = Elshift;        return SHIFT; }
"++"		{ return INC; }
"--"		{ return DEC; }

{onechar}	{ return yytext[0]; }

"\n"		{ ++lineNo; }

{space}		/* skip */

.		{ compileError(EIllegalChar,*yytext,*yytext);
		  return 0; }

%%

// 値が範囲内かチェック
static void checkValue(int num, int max, int min)
{
  if (num > max || num < min)
    compileError(EOverflowNum,num);
}

// 値が符号付整数の範囲内かチェック
static void checkSValue(int num)
{
  checkValue(num,MaxSignedNum,MinSignedNum);
}

// 値が符号無し整数の範囲内かチェック
static void checkUValue(int num)
{
  checkValue(num,MaxUnsignedNum,MinUnsignedNum);
}

// 下位16ビットを符号付き16ビット整数に変換
static int toSignedNum(int num)
{
  if (num > MaxSignedNum)
    return num - MaxUnsignedNum - 1;
  else
    return num;
}

// 文字列をバッファにコピー
static void copyString(char* str)
{
  int len = strlen(str);
  if (buffIndex + len < MaxStrLen)  {
    strcpy(strBuff + buffIndex, str);
    buffIndex += len;
  }
  else
    compileError(ETooLongString);
}

// 文字列の複製を作る
static char* dupname(char* str)
{
  char* name;
  if ((name=malloc(strlen(str)+1)) == NULL)
    errorExit(EShortOfMemory);
  return strcpy(name,str);
}

// 16進数を表す文字列から符号付き整数への変換
static int atoh(char *str)
{
  int num = 0;
  char *p;

  p = str + 2;
  if (strlen(p) > MaxHexDigit)
    compileError(EOverflowHex, p);
  while (*p)  {
    num = num * 16 + (isdigit(*p) ? *p - '0' : tolower(*p) - 'a' + 10);
    p++;
  }
  if (num & 0x8000) num |= 0xffff0000;
  return num;
}

// スキャナーの初期化
void initScanner(char *filename)
{
  setFilename(filename);
  if ((yyin=fopen(filename,"r")) == NULL)
    errorExit(EFileNotFound,filename);
  lineNo = 1;
}
