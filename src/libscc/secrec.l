%{
#include "Parser.h"
#include "yacc_secrec.h"

#define step(loc)\
    do {\
        (loc)->first_line = (loc)->last_line;\
        (loc)->first_column = (loc)->last_column;\
    } while (0)

#define newline(loc)\
    do {\
        step(loc);\
        (loc)->last_line++;\
        (loc)->last_column = 1;\
    } while (0)

#define stepLen(loc,len)\
    do {\
        step(loc);\
        (loc)->last_column += len;\
    } while (0)

#define stepChar(loc)\
    do {\
        step(loc);\
        (loc)->last_column++;\
    } while (0)

#define saveText(s)\
    do {\
        stepLen(yyget_lloc(s), (size_t) yyget_leng(s));\
        yyget_lval(s)->str = add_string (yyget_extra(s), yyget_text(s), (size_t) yyget_leng(s));\
    } while (0)

#define STRBUF_SIZE 2048
static char secrec_strbuf[STRBUF_SIZE];
static unsigned secrec_strbuflen;

int bufferChar(char c);
int bufferChar(char c) {
    if (STRBUF_SIZE == 0 || secrec_strbuflen >= STRBUF_SIZE - 1)
        return 0;

    secrec_strbuf[secrec_strbuflen ++] = c;
    return 1;
}

void clearBuffer(void);
void clearBuffer(void) { secrec_strbuflen = 0; }

#define saveString(s) \
    do { \
        yyget_lval(s)->str = add_string (yyget_extra(s), &secrec_strbuf[0], secrec_strbuflen); \
        clearBuffer (); \
    } while (0)


/* The following functions and macros are to work around compiler warnings for
 * Flex-generated code: */

void * yyalloc(yy_size_t  size, yyscan_t yyscanner)
{ (void) yyscanner; return malloc(size); }

void * yyrealloc(void * ptr, yy_size_t size, yyscan_t yyscanner)
{ (void) yyscanner; return realloc(ptr, size); }

void yyfree(void * ptr, yyscan_t yyscanner)
{ (void) yyscanner; free(ptr); }

#define YY_EXIT_FAILURE (((void) yyscanner), 2)

%}

%option extra-type="TYPE_STRINGTABLE"
%option bison-bridge bison-locations reentrant noyywrap nounput noinput
%option noyyalloc noyyfree noyyrealloc

 /* =============================================================================
    Character classes
    ============================================================================= */
BINARYDIGIT     [0-1]
OCTALDIGIT      [0-7]
DIGIT           [0-9]
HEXDIGIT        [0-9a-fA-F]
UNDERSCORE      _
WHITESPACE      [ \t]
UPPERCASE       [A-Z]
LOWERCASE       [a-z]
LETTER          ({UPPERCASE}|{LOWERCASE})
NONDIGIT        ({UPPERCASE}|{LOWERCASE}|{UNDERSCORE})
EXPONENT        (e|E)[+-]?{DECIMAL}
IDENTIFIER      {NONDIGIT}({NONDIGIT}|{DIGIT})*
FLOATSUFFIX     f(32|64)
INTSUFFIX       (u8|u16|u32|u64|i8|i16|i32|i64)
DECIMAL         (0|[1-9]{DIGIT}*)({INTSUFFIX}|{FLOATSUFFIX})?
BINARYLIT       0b{BINARYDIGIT}+{INTSUFFIX}?
OCTALLIT        0o{OCTALDIGIT}+{INTSUFFIX}?
HEXLIT          0x{HEXDIGIT}+{INTSUFFIX}?
SIMPLEFLOAT     {DECIMAL}{EXPONENT}{FLOATSUFFIX}?
SCIENTIFICFLOAT {DECIMAL}\.{DIGIT}*{EXPONENT}?{FLOATSUFFIX}?
NEWLINE         (\n\r|\r\n|\n|\r)
SIMPLEOP        [\.\-\,\;\:\!\?\/\^\~\(\)\[\]\{\}\*\&\%\+\<\=\>\|\@]


%x COMMENT_C
%x COMMENT_CPP
%x STATE_STRING
%x STATE_STRING_VARIABLE
%%

 /* C-style comments: */
"/*"                      { stepLen(yyget_lloc(yyscanner), 2); BEGIN(COMMENT_C); }
<COMMENT_C>[^*\n\r]*      { stepLen(yyget_lloc(yyscanner), (size_t) yyget_leng(yyscanner)); }
<COMMENT_C>"*"+[^*/\n\r]* { stepLen(yyget_lloc(yyscanner), (size_t) yyget_leng(yyscanner)); }
<COMMENT_C>{NEWLINE}      { newline(yyget_lloc(yyscanner)); }
<COMMENT_C>"*"+"/"        { BEGIN(INITIAL); }

 /* C++-style comments: */
"//"               { stepLen(yyget_lloc(yyscanner), 2); BEGIN(COMMENT_CPP); }
<COMMENT_CPP>[^\n] { stepChar(yyget_lloc(yyscanner)); }
<COMMENT_CPP>[\n]  { newline(yyget_lloc(yyscanner)); BEGIN(INITIAL); }

 /* String literals: */
\"                 { BEGIN(STATE_STRING); stepChar(yyget_lloc(yyscanner)); clearBuffer (); }
<STATE_STRING>\"   { BEGIN(INITIAL); stepChar(yyget_lloc(yyscanner)); saveString(yyscanner); return STR_FRAGMENT; }
<STATE_STRING>\$   { BEGIN(STATE_STRING_VARIABLE); stepChar(yyget_lloc(yyscanner)); saveString(yyscanner); return STR_FRAGMENT; }
<STATE_STRING>\\n  { stepLen(yyget_lloc(yyscanner), 2); if (! bufferChar ('\n')) return INVALID_STRING; }
<STATE_STRING>\\t  { stepLen(yyget_lloc(yyscanner), 2); if (! bufferChar ('\t')) return INVALID_STRING; }
<STATE_STRING>\\r  { stepLen(yyget_lloc(yyscanner), 2); if (! bufferChar ('\r')) return INVALID_STRING; }
<STATE_STRING>\\.  { stepLen(yyget_lloc(yyscanner), 2); if (! bufferChar (yyget_text(yyscanner)[1])) return INVALID_STRING; }
<STATE_STRING>.    { stepChar(yyget_lloc(yyscanner)); if (! bufferChar (yyget_text(yyscanner)[0])) return INVALID_STRING; }

 /* String identifier: */
<STATE_STRING_VARIABLE>{IDENTIFIER} { BEGIN(STATE_STRING); saveText(yyscanner); return STR_IDENTIFIER; }

 /* Keywords: */
"assert"     { stepLen(yyget_lloc(yyscanner), 6);  return ASSERT; }
"bool"       { stepLen(yyget_lloc(yyscanner), 4);  return BOOL; }
"break"      { stepLen(yyget_lloc(yyscanner), 5);  return BREAK; }
"cast"       { stepLen(yyget_lloc(yyscanner), 4);  return CAST; }
"cat"        { stepLen(yyget_lloc(yyscanner), 3);  return CAT; }
"continue"   { stepLen(yyget_lloc(yyscanner), 8);  return CONTINUE; }
"declassify" { stepLen(yyget_lloc(yyscanner), 8);  return DECLASSIFY; }
"dim"        { stepLen(yyget_lloc(yyscanner), 3);  return DIMENSIONALITY; }
"do"         { stepLen(yyget_lloc(yyscanner), 2);  return DO; }
"domain"     { stepLen(yyget_lloc(yyscanner), 6);  return DOMAIN; }
"else"       { stepLen(yyget_lloc(yyscanner), 4);  return ELSE; }
"false"      { stepLen(yyget_lloc(yyscanner), 5);  return FALSE_B; }
"float"      { stepLen(yyget_lloc(yyscanner), 6);  return FLOAT; }
"float32"    { stepLen(yyget_lloc(yyscanner), 7);  return FLOAT32; }
"float64"    { stepLen(yyget_lloc(yyscanner), 7);  return FLOAT64; }
"for"        { stepLen(yyget_lloc(yyscanner), 3);  return FOR; }
"if"         { stepLen(yyget_lloc(yyscanner), 2);  return IF; }
"import"     { stepLen(yyget_lloc(yyscanner), 6);  return IMPORT; }
"int"        { stepLen(yyget_lloc(yyscanner), 3);  return INT; }
"int16"      { stepLen(yyget_lloc(yyscanner), 5);  return INT16; }
"int32"      { stepLen(yyget_lloc(yyscanner), 5);  return INT32; }
"int64"      { stepLen(yyget_lloc(yyscanner), 5);  return INT64; }
"int8"       { stepLen(yyget_lloc(yyscanner), 4);  return INT8; }
"kind"       { stepLen(yyget_lloc(yyscanner), 4);  return KIND; }
"module"     { stepLen(yyget_lloc(yyscanner), 6);  return MODULE; }
"operator"   { stepLen(yyget_lloc(yyscanner), 8);  return OPERATOR; }
"print"      { stepLen(yyget_lloc(yyscanner), 5);  return PRINT; }
"public"     { stepLen(yyget_lloc(yyscanner), 6);  return PUBLIC; }
"reshape"    { stepLen(yyget_lloc(yyscanner), 7);  return RESHAPE; }
"return"     { stepLen(yyget_lloc(yyscanner), 6);  return RETURN; }
"shape"      { stepLen(yyget_lloc(yyscanner), 5);  return SHAPE; }
"size"       { stepLen(yyget_lloc(yyscanner), 4);  return SIZE; }
"string"     { stepLen(yyget_lloc(yyscanner), 6);  return STRING; }
"strlen"     { stepLen(yyget_lloc(yyscanner), 6);  return STRLEN; }
"struct"     { stepLen(yyget_lloc(yyscanner), 6);  return STRUCT; }
"template"   { stepLen(yyget_lloc(yyscanner), 8);  return TEMPLATE; }
"tostring"   { stepLen(yyget_lloc(yyscanner), 8);  return TOSTRING; }
"true"       { stepLen(yyget_lloc(yyscanner), 4);  return TRUE_B; }
"type"       { stepLen(yyget_lloc(yyscanner), 4);  return TYPE; }
"uint"       { stepLen(yyget_lloc(yyscanner), 4);  return UINT; }
"uint16"     { stepLen(yyget_lloc(yyscanner), 6);  return UINT16; }
"uint32"     { stepLen(yyget_lloc(yyscanner), 6);  return UINT32; }
"uint64"     { stepLen(yyget_lloc(yyscanner), 6);  return UINT64; }
"uint8"      { stepLen(yyget_lloc(yyscanner), 5);  return UINT8; }
"void"       { stepLen(yyget_lloc(yyscanner), 4);  return VOID; }
"while"      { stepLen(yyget_lloc(yyscanner), 5);  return WHILE; }

 /* built-in functions: */
"__bytes_from_string" { stepLen(yyget_lloc(yyscanner), 19); return BYTESFROMSTRING; }
"__cref"              { stepLen(yyget_lloc(yyscanner), 6);  return CREF; }
"__domainid"          { stepLen(yyget_lloc(yyscanner), 10); return DOMAINID; }
"__readonly"          { stepLen(yyget_lloc(yyscanner), 10); return READONLY; }
"__ref"               { stepLen(yyget_lloc(yyscanner), 5);  return REF; }
"__return"            { stepLen(yyget_lloc(yyscanner), 8);  return SYSCALL_RETURN; }
"__string_from_bytes" { stepLen(yyget_lloc(yyscanner), 19); return STRINGFROMBYTES; }
"__syscall"           { stepLen(yyget_lloc(yyscanner), 9);  return SYSCALL; }
"__fpu_state"         { stepLen(yyget_lloc(yyscanner), 15); return GET_FPU_STATE; }
"__set_fpu_state"     { stepLen(yyget_lloc(yyscanner), 15); return SET_FPU_STATE; }

{IDENTIFIER}       { saveText(yyscanner); return IDENTIFIER; }
{BINARYLIT}        { saveText(yyscanner); return BIN_LITERAL; }
{OCTALLIT}         { saveText(yyscanner); return OCT_LITERAL; }
{HEXLIT}           { saveText(yyscanner); return HEX_LITERAL; }
{SIMPLEFLOAT}      { saveText(yyscanner); return FLOAT_LITERAL; }
{SCIENTIFICFLOAT}  { saveText(yyscanner); return FLOAT_LITERAL; }
{DECIMAL}          { saveText(yyscanner); return DEC_LITERAL; }

"+=" { stepLen(yyget_lloc(yyscanner), 2); return ADD_ASSIGN; }
"&=" { stepLen(yyget_lloc(yyscanner), 2); return AND_ASSIGN; }
"--" { stepLen(yyget_lloc(yyscanner), 2); return DEC_OP; }
"/=" { stepLen(yyget_lloc(yyscanner), 2); return DIV_ASSIGN; }
"==" { stepLen(yyget_lloc(yyscanner), 2); return EQ_OP; }
">=" { stepLen(yyget_lloc(yyscanner), 2); return GE_OP; }
"++" { stepLen(yyget_lloc(yyscanner), 2); return INC_OP; }
"&&" { stepLen(yyget_lloc(yyscanner), 2); return LAND_OP; }
"<=" { stepLen(yyget_lloc(yyscanner), 2); return LE_OP; }
"||" { stepLen(yyget_lloc(yyscanner), 2); return LOR_OP; }
">>" { stepLen(yyget_lloc(yyscanner), 2); return SHR_OP; }
"<<" { stepLen(yyget_lloc(yyscanner), 2); return SHL_OP; }
"%=" { stepLen(yyget_lloc(yyscanner), 2); return MOD_ASSIGN; }
"*=" { stepLen(yyget_lloc(yyscanner), 2); return MUL_ASSIGN; }
"!=" { stepLen(yyget_lloc(yyscanner), 2); return NE_OP; }
"|=" { stepLen(yyget_lloc(yyscanner), 2); return OR_ASSIGN; }
"-=" { stepLen(yyget_lloc(yyscanner), 2); return SUB_ASSIGN; }
"::" { stepLen(yyget_lloc(yyscanner), 2); return TYPE_QUAL; }
"^=" { stepLen(yyget_lloc(yyscanner), 2); return XOR_ASSIGN; }

{SIMPLEOP}   { stepChar(yyget_lloc(yyscanner)); return yyget_text(yyscanner)[0]; }
{NEWLINE}    { newline(yyget_lloc(yyscanner)); }
{WHITESPACE} { stepChar(yyget_lloc(yyscanner)); }

. { return INVALID_CHARACTER; }
