%{
#include "parser.h"
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

#define saveText(s) yyget_lval(s)->str = strndup(yyget_text(s), yyget_leng(s));

%}

%option bison-bridge bison-locations reentrant noyywrap nounput noinput

/* =============================================================================
  Character classes
============================================================================= */
DIGIT           [0-9]
UNDERSCORE      _
WHITESPACE      [ \t]
/* WHITESPACE      [[:space:]] */
UPPERCASE       [ABCDEFGHIJKLMNOPQRSTUVWXYZ]
LOWERCASE       [abcdefghijklmnopqrstuvwxyz]
LETTER          ({UPPERCASE}|{LOWERCASE})
NONDIGIT        ({UPPERCASE}|{LOWERCASE}|{UNDERSCORE})

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

"bool"       { stepLen(yyget_lloc(yyscanner), 4); return BOOL; }
"break"      { stepLen(yyget_lloc(yyscanner), 5); return BREAK; }
"continue"   { stepLen(yyget_lloc(yyscanner), 8); return CONTINUE; }
"declassify" { stepLen(yyget_lloc(yyscanner), 8); return DECLASSIFY; }
"do"         { stepLen(yyget_lloc(yyscanner), 2); return DO; }
 /* "double"   { stepLen(yyget_lloc(yyscanner), 6); return DOUBLE; } */
"else"       { stepLen(yyget_lloc(yyscanner), 4); return ELSE; }
"false"      { stepLen(yyget_lloc(yyscanner), 5); return FALSE_B; }
"for"        { stepLen(yyget_lloc(yyscanner), 3); return FOR; }
"if"         { stepLen(yyget_lloc(yyscanner), 2); return IF; }
"int"        { stepLen(yyget_lloc(yyscanner), 3); return INT; }
"private"    { stepLen(yyget_lloc(yyscanner), 7); return PRIVATE; }
"public"     { stepLen(yyget_lloc(yyscanner), 6); return PUBLIC; }
"return"     { stepLen(yyget_lloc(yyscanner), 6); return RETURN; }
"signed"     { stepLen(yyget_lloc(yyscanner), 6); return SIGNED; }
"string"     { stepLen(yyget_lloc(yyscanner), 6); return STRING; }
"true"       { stepLen(yyget_lloc(yyscanner), 4); return TRUE_B; }
"unsigned"   { stepLen(yyget_lloc(yyscanner), 8); return UNSIGNED; }
"void"       { stepLen(yyget_lloc(yyscanner), 4); return VOID; }
"while"      { stepLen(yyget_lloc(yyscanner), 5); return WHILE; }
"assert"     { stepLen(yyget_lloc(yyscanner), 6); return ASSERT; }
"size"       { stepLen(yyget_lloc(yyscanner), 4); return SIZE; }
"shape"      { stepLen(yyget_lloc(yyscanner), 5); return SHAPE; }
"reshape"    { stepLen(yyget_lloc(yyscanner), 7); return RESHAPE; }
"cat"        { stepLen(yyget_lloc(yyscanner), 3); return CAT; }
"print"      { stepLen(yyget_lloc(yyscanner), 5); return PRINT; }
"fread"      { stepLen(yyget_lloc(yyscanner), 5); return FREAD; }

{NONDIGIT}({NONDIGIT}|{DIGIT})* { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); saveText(yyscanner); return IDENTIFIER; }

0|[1-9]{DIGIT}* { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); saveText(yyscanner); return INT_LITERAL; }
0(0|[1-9]{DIGIT}*) { stepLen(yyget_lloc(yyscanner), yyget_leng(yyscanner)); saveText(yyscanner); return UINT_LITERAL; }
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
 /* "++" { stepLen(yyget_lloc(yyscanner), 2); return INC_OP; } */
 /* "--" { stepLen(yyget_lloc(yyscanner), 2); return DEC_OP; } */
"&&" { stepLen(yyget_lloc(yyscanner), 2); return LAND_OP; }
"||" { stepLen(yyget_lloc(yyscanner), 2); return LOR_OP; }
"<=" { stepLen(yyget_lloc(yyscanner), 2); return LE_OP; }
">=" { stepLen(yyget_lloc(yyscanner), 2); return GE_OP; }
"==" { stepLen(yyget_lloc(yyscanner), 2); return EQ_OP; }
"!=" { stepLen(yyget_lloc(yyscanner), 2); return NE_OP; }
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
"#"  { stepChar(yyget_lloc(yyscanner)); return '#'; }

"\n\r" { newline(yyget_lloc(yyscanner)); }
"\r\n" { newline(yyget_lloc(yyscanner)); }
"\n" { newline(yyget_lloc(yyscanner)); }
"\r" { newline(yyget_lloc(yyscanner)); }
{WHITESPACE} { stepChar(yyget_lloc(yyscanner)); }

. { /* Ignore bad characters */ }
