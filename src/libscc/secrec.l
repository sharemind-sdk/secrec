%{
#include "secrec/parser.h"
#include "yacc_secrec.tab.h"

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

%}

%option bison-bridge bison-locations reentrant noyywrap nounput noinput

/* =============================================================================
  Character classes
============================================================================= */
DIGIT           [0-9]
UNDERSCORE      _
WHITESPACE      [ \n\r\t]
/* WHITESPACE      [[:space:]] */
UPPERCASE       [ABCDEFGHIJKLMNOPQRSTUVWXYZ]
LOWERCASE       [abcdefghijklmnopqrstuvwxyz]
LETTER          ({UPPERCASE}|{LOWERCASE})
NONDIGIT        ({UPPERCASE}|{LOWERCASE}|{UNDERSCORE})

%x COMMENT_C
%x COMMENT_CPP
%%

"/*"                      { stepLen(yylloc, 2); BEGIN(COMMENT_C); }
<COMMENT_C>[^*\n\r]*      { stepLen(yylloc, yyleng); }
<COMMENT_C>"*"+[^*/\n\r]* { stepLen(yylloc, yyleng); }
<COMMENT_C>\n\r           { newline(yylloc); }
<COMMENT_C>\r\n           { newline(yylloc); }
<COMMENT_C>\n             { newline(yylloc); }
<COMMENT_C>\r             { newline(yylloc); }
<COMMENT_C>"*"+"/"        { BEGIN(INITIAL); }

 /* Ignore C++-style comments: */
"//"               { stepLen(yylloc, 2); BEGIN(COMMENT_CPP); }
<COMMENT_CPP>[^\n] { stepChar(yylloc); }
<COMMENT_CPP>[\n]  { newline(yylloc); BEGIN(INITIAL); }

"bool"     { stepLen(yylloc, 4); return BOOL; }
"break"    { stepLen(yylloc, 5); return BREAK; }
"continue" { stepLen(yylloc, 8); return CONTINUE; }
"do"       { stepLen(yylloc, 2); return DO; }
 /* "double"   { stepLen(yylloc, 6); return DOUBLE; } */
"else"     { stepLen(yylloc, 4); return ELSE; }
"false"    { stepLen(yylloc, 5); return FALSE_B; }
"for"      { stepLen(yylloc, 3); return FOR; }
"if"       { stepLen(yylloc, 2); return IF; }
"int"      { stepLen(yylloc, 3); return INT; }
"private"  { stepLen(yylloc, 7); return PRIVATE; }
"public"   { stepLen(yylloc, 6); return PUBLIC; }
"return"   { stepLen(yylloc, 6); return RETURN; }
"signed"   { stepLen(yylloc, 6); return SIGNED; }
"string"   { stepLen(yylloc, 6); return STRING; }
"true"     { stepLen(yylloc, 4); return TRUE_B; }
"unsigned" { stepLen(yylloc, 8); return UNSIGNED; }
"void"     { stepLen(yylloc, 4); return VOID; }
"while"    { stepLen(yylloc, 5); return WHILE; }

{NONDIGIT}({NONDIGIT}|{DIGIT})* { stepLen(yylloc, yyleng); return IDENTIFIER; }

0|[1-9]{DIGIT}*   { stepLen(yylloc, yyleng); return DECIMAL_LITERAL; }
\"(\\.|[^\\"])*\" { stepLen(yylloc, yyleng); return STRING_LITERAL; }

"+=" { stepLen(yylloc, 2); return ADD_ASSIGN; }
"-=" { stepLen(yylloc, 2); return SUB_ASSIGN; }
"*=" { stepLen(yylloc, 2); return MUL_ASSIGN; }
"/=" { stepLen(yylloc, 2); return DIV_ASSIGN; }
"%=" { stepLen(yylloc, 2); return MOD_ASSIGN; }
 /* "++" { stepLen(yylloc, 2); return INC_OP; } */
 /* "--" { stepLen(yylloc, 2); return DEC_OP; } */
"&&" { stepLen(yylloc, 2); return LAND_OP; }
"||" { stepLen(yylloc, 2); return LOR_OP; }
"<=" { stepLen(yylloc, 2); return LE_OP; }
">=" { stepLen(yylloc, 2); return GE_OP; }
"==" { stepLen(yylloc, 2); return EQ_OP; }
"!=" { stepLen(yylloc, 2); return NE_OP; }
":"  { stepChar(yylloc); return ':'; }
";"  { stepChar(yylloc); return ';'; }
"{"  { stepChar(yylloc); return '{'; }
"}"  { stepChar(yylloc); return '}'; }
","  { stepChar(yylloc); return ','; }
"="  { stepChar(yylloc); return '='; }
"("  { stepChar(yylloc); return '('; }
")"  { stepChar(yylloc); return ')'; }
"["  { stepChar(yylloc); return '['; }
"]"  { stepChar(yylloc); return ']'; }
"!"  { stepChar(yylloc); return '!'; }
"-"  { stepChar(yylloc); return '-'; }
"+"  { stepChar(yylloc); return '+'; }
"*"  { stepChar(yylloc); return '*'; }
"/"  { stepChar(yylloc); return '/'; }
"%"  { stepChar(yylloc); return '%'; }
"<"  { stepChar(yylloc); return '<'; }
">"  { stepChar(yylloc); return '>'; }
"?"  { stepChar(yylloc); return '?'; }
"#"  { stepChar(yylloc); return '#'; }

"\n\r" { newline(yylloc); }
"\r\n" { newline(yylloc); }
"\n" { newline(yylloc); }
"\r" { newline(yylloc); }
{WHITESPACE} { stepChar(yylloc); }

. { /* Ignore bad characters */ }
