%{
#include "parser.h"
#include "yacc_secrec.h"

#define step(loc) {\
    (loc)->first_line = (loc)->last_line;\
    (loc)->first_column = (loc)->last_column;\
}

#define newline(loc) {\
    step(loc);\
    (loc)->last_line++;\
    (loc)->last_column = 1;\
}

#define stepLen(loc,len) {\
    step(loc);\
    (loc)->last_column += len;\
}

#define stepChar(loc) {\
    step(loc);\
    (loc)->last_column++;\
}

#define saveText(s) yyget_lval(s)->str = strndup(yyget_text(s), yyget_leng(s));

%}

%option bison-bridge bison-locations reentrant noyywrap nounput noinput

/* =============================================================================
  Character classes
============================================================================= */
BINARYDIGIT     [0-1]
OCTALDIGIT      [0-7]
DIGIT           [0-9]
HEXDIGIT        [0-9a-fA-F]
UNDERSCORE      _
WHITESPACE      [ \t]
UPPERCASE       [ABCDEFGHIJKLMNOPQRSTUVWXYZ]
LOWERCASE       [abcdefghijklmnopqrstuvwxyz]
LETTER          ({UPPERCASE}|{LOWERCASE})
NONDIGIT        ({UPPERCASE}|{LOWERCASE}|{UNDERSCORE})
DECIMAL         0|[1-9]{DIGIT}*
EXPONENT        (e|E)[+-]?{DECIMAL}

%x COMMENT_C
%x COMMENT_CPP
%%

"/*"                      { stepLen(yyget_lloc(yyscanner), 2); BEGIN(COMMENT_C); }
<COMMENT_C>[^*\n\r]*      { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); }
<COMMENT_C>"*"+[^*/\n\r]* { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); }
<COMMENT_C>\n\r           { newline(yyget_lloc(yyscanner)); }
<COMMENT_C>\r\n           { newline(yyget_lloc(yyscanner)); }
<COMMENT_C>\n             { newline(yyget_lloc(yyscanner)); }
<COMMENT_C>\r             { newline(yyget_lloc(yyscanner)); }
<COMMENT_C>"*"+"/"        { BEGIN(INITIAL); }

 /* Ignore C++-style comments: */
"//"               { stepLen(yyget_lloc(yyscanner), 2); BEGIN(COMMENT_CPP); }
<COMMENT_CPP>[^\n] { stepChar(yyget_lloc(yyscanner)); }
<COMMENT_CPP>[\n]  { newline(yyget_lloc(yyscanner)); BEGIN(INITIAL); }

"assert"     { stepLen(yyget_lloc(yyscanner), 6); return ASSERT; }
"bool"       { stepLen(yyget_lloc(yyscanner), 4); return BOOL; }
"break"      { stepLen(yyget_lloc(yyscanner), 5); return BREAK; }
"cat"        { stepLen(yyget_lloc(yyscanner), 3); return CAT; }
"continue"   { stepLen(yyget_lloc(yyscanner), 8); return CONTINUE; }
"declassify" { stepLen(yyget_lloc(yyscanner), 8); return DECLASSIFY; }
"do"         { stepLen(yyget_lloc(yyscanner), 2); return DO; }
"domain"     { stepLen(yyget_lloc(yyscanner), 6); return DOMAIN; }
"else"       { stepLen(yyget_lloc(yyscanner), 4); return ELSE; }
"false"      { stepLen(yyget_lloc(yyscanner), 5); return FALSE_B; }
"for"        { stepLen(yyget_lloc(yyscanner), 3); return FOR; }
"if"         { stepLen(yyget_lloc(yyscanner), 2); return IF; }
"int"        { stepLen(yyget_lloc(yyscanner), 3); return INT; }
"uint"       { stepLen(yyget_lloc(yyscanner), 4); return UINT; }
"int8"       { stepLen(yyget_lloc(yyscanner), 4); return INT8; }
"uint8"      { stepLen(yyget_lloc(yyscanner), 5); return UINT8; }
"int16"      { stepLen(yyget_lloc(yyscanner), 5); return INT16; }
"uint16"     { stepLen(yyget_lloc(yyscanner), 6); return UINT16; }
"int32"      { stepLen(yyget_lloc(yyscanner), 5); return INT32; }
"uint32"     { stepLen(yyget_lloc(yyscanner), 6); return UINT32; }
"int64"      { stepLen(yyget_lloc(yyscanner), 5); return INT64; }
"uint64"     { stepLen(yyget_lloc(yyscanner), 6); return UINT64; }
"public"     { stepLen(yyget_lloc(yyscanner), 6); return PUBLIC; }
"return"     { stepLen(yyget_lloc(yyscanner), 6); return RETURN; }
"string"     { stepLen(yyget_lloc(yyscanner), 6); return STRING; }
"tostring"   { stepLen(yyget_lloc(yyscanner), 8); return TOSTRING; }
"true"       { stepLen(yyget_lloc(yyscanner), 4); return TRUE_B; }
"void"       { stepLen(yyget_lloc(yyscanner), 4); return VOID; }
"while"      { stepLen(yyget_lloc(yyscanner), 5); return WHILE; }
"size"       { stepLen(yyget_lloc(yyscanner), 4); return SIZE; }
"shape"      { stepLen(yyget_lloc(yyscanner), 5); return SHAPE; }
"reshape"    { stepLen(yyget_lloc(yyscanner), 7); return RESHAPE; }
"print"      { stepLen(yyget_lloc(yyscanner), 5); return PRINT; }
"kind"       { stepLen(yyget_lloc(yyscanner), 4); return KIND; }
"template"   { stepLen(yyget_lloc(yyscanner), 8); return TEMPLATE; }
"__syscall"  { stepLen(yyget_lloc(yyscanner), 9); return SYSCALL; }
"__push"     { stepLen(yyget_lloc(yyscanner), 6); return PUSH; }
"__pushref"  { stepLen(yyget_lloc(yyscanner), 9); return PUSHREF; }
"__pushcref" { stepLen(yyget_lloc(yyscanner), 10); return PUSHCREF; }
"__domainid" { stepLen(yyget_lloc(yyscanner), 10); return DOMAINID; }
"operator"   { stepLen(yyget_lloc(yyscanner), 8); return OPERATOR; }
"module"     { stepLen(yyget_lloc(yyscanner), 6); return MODULE; }
"import"     { stepLen(yyget_lloc(yyscanner), 6); return IMPORT; }
"xor_uint"   { stepLen(yyget_lloc(yyscanner), 8); return XOR_UINT; }
"xor_uint8"  { stepLen(yyget_lloc(yyscanner), 9); return XOR_UINT8; }
"xor_uint16" { stepLen(yyget_lloc(yyscanner), 10); return XOR_UINT16; }
"xor_uint32" { stepLen(yyget_lloc(yyscanner), 10); return XOR_UINT32; }
"xor_uint64" { stepLen(yyget_lloc(yyscanner), 10); return XOR_UINT64; }
"float"    {  stepLen(yyget_lloc(yyscanner), 6); return FLOAT; }
"float32"    {  stepLen(yyget_lloc(yyscanner), 7); return FLOAT32; }
"float64"    {  stepLen(yyget_lloc(yyscanner), 7); return FLOAT64; }
"__string_from_bytes" { stepLen(yyget_lloc(yyscanner), 19); return STRINGFROMBYTES; }
"__bytes_from_string" { stepLen(yyget_lloc(yyscanner), 19); return BYTESFROMSTRING; }

{NONDIGIT}({NONDIGIT}|{DIGIT})* { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); saveText(yyscanner); return IDENTIFIER; }

0b{BINARYDIGIT}+ { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); saveText(yyscanner); return BIN_LITERAL; }
0o{OCTALDIGIT}+  { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); saveText(yyscanner); return OCT_LITERAL; }
0x{HEXDIGIT}+ { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); saveText(yyscanner); return HEX_LITERAL; }
{DECIMAL}{EXPONENT} { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); saveText(yyscanner); return FLOAT_LITERAL; }
{DECIMAL}\.{DECIMAL}{EXPONENT}? { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); saveText(yyscanner); return FLOAT_LITERAL; }
{DECIMAL} { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); saveText(yyscanner); return DEC_LITERAL; }
\"(\\.|[^\\"])*\" {
    stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner));
    yyget_lval(yyscanner)->str = strndup(yyget_text(yyscanner) + 1, yyget_leng(yyscanner) - 2);
    return STRING_LITERAL;
 }

"+=" { stepLen(yyget_lloc(yyscanner), 2); return ADD_ASSIGN; }
"-=" { stepLen(yyget_lloc(yyscanner), 2); return SUB_ASSIGN; }
"*=" { stepLen(yyget_lloc(yyscanner), 2); return MUL_ASSIGN; }
"/=" { stepLen(yyget_lloc(yyscanner), 2); return DIV_ASSIGN; }
"%=" { stepLen(yyget_lloc(yyscanner), 2); return MOD_ASSIGN; }
"++" { stepLen(yyget_lloc(yyscanner), 2); return INC_OP; }
"--" { stepLen(yyget_lloc(yyscanner), 2); return DEC_OP; }
"&&" { stepLen(yyget_lloc(yyscanner), 2); return LAND_OP; }
"||" { stepLen(yyget_lloc(yyscanner), 2); return LOR_OP; }
"<=" { stepLen(yyget_lloc(yyscanner), 2); return LE_OP; }
">=" { stepLen(yyget_lloc(yyscanner), 2); return GE_OP; }
"==" { stepLen(yyget_lloc(yyscanner), 2); return EQ_OP; }
"!=" { stepLen(yyget_lloc(yyscanner), 2); return NE_OP; }
"::" { stepLen(yyget_lloc(yyscanner), 2); return TYPE_QUAL; }
":"  { stepChar(yyget_lloc(yyscanner)); return ':'; }
";"  { stepChar(yyget_lloc(yyscanner)); return ';'; }
"{"  { stepChar(yyget_lloc(yyscanner)); return '{'; }
"}"  { stepChar(yyget_lloc(yyscanner)); return '}'; }
","  { stepChar(yyget_lloc(yyscanner)); return ','; }
"="  { stepChar(yyget_lloc(yyscanner)); return '='; }
"("  { stepChar(yyget_lloc(yyscanner)); return '('; }
")"  { stepChar(yyget_lloc(yyscanner)); return ')'; }
"["  { stepChar(yyget_lloc(yyscanner)); return '['; }
"]"  { stepChar(yyget_lloc(yyscanner)); return ']'; }
"!"  { stepChar(yyget_lloc(yyscanner)); return '!'; }
"-"  { stepChar(yyget_lloc(yyscanner)); return '-'; }
"+"  { stepChar(yyget_lloc(yyscanner)); return '+'; }
"*"  { stepChar(yyget_lloc(yyscanner)); return '*'; }
"/"  { stepChar(yyget_lloc(yyscanner)); return '/'; }
"%"  { stepChar(yyget_lloc(yyscanner)); return '%'; }
"<"  { stepChar(yyget_lloc(yyscanner)); return '<'; }
">"  { stepChar(yyget_lloc(yyscanner)); return '>'; }
"?"  { stepChar(yyget_lloc(yyscanner)); return '?'; }
 /* "#"  { stepChar(yyget_lloc(yyscanner)); return '#'; } */

"\n\r" { newline(yyget_lloc(yyscanner)); }
"\r\n" { newline(yyget_lloc(yyscanner)); }
"\n" { newline(yyget_lloc(yyscanner)); }
"\r" { newline(yyget_lloc(yyscanner)); }
{WHITESPACE} { stepChar(yyget_lloc(yyscanner)); }

. { /* Ignore bad characters */ }
