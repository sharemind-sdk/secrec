%require "2.4"
%{
  #include <assert.h>
  #include <stdio.h>
  #include <stdint.h>

  #include "parser.h"
  #include "lex_secrec.h"
  #include "treenode_c.h"

  void yyerror(YYLTYPE *loc, yyscan_t yyscanner, TYPE_TREENODE *parseTree, const char *s);

  uint64_t char_to_digit(char c)
  {
      if ('0' <= c && c <= '9')
          return c - '0';
      if ('a' <= c && c <= 'f')
          return (c - 'a') + 10;
      if ('A' <= c && c <= 'F')
          return (c - 'A') + 10;
      assert(0 && "Invalid digit character!");
      return 0;
  }

  uint64_t convert_to_base(const char * input, uint64_t base)
  {
      assert(base == 2 || base == 8 || base == 10 || base == 16);
      uint64_t out = 0;
      size_t offset = 0;
      if (base != 10) {
          /* skip the headers: 0b, 0o, and 0x */
          assert(input[0] != '\0' && input[1] != '\0');
          if (input[0] == '\0') return 0;
          if (input[1] == '\0') return 0;
          offset += 2;
      }

      const char * ptr = 0;
      for (ptr = &input[offset]; *ptr != '\0'; ++ ptr) {
          uint64_t digit = char_to_digit(*ptr);
          assert(digit < base);
          uint64_t new_out = out*base + digit;
          assert(new_out >= out);
          out = new_out;
      }

      return out;
  }

  struct TreeNode * init_op(enum SecrecOperator op, YYLTYPE * loc,
                            struct TreeNode * ret,
                            struct TreeNode * params)
  {
      struct TreeNode * out = treenode_init_opdef(op, loc);
      treenode_appendChild(out, ret);
      treenode_moveChildren(params, out);
      treenode_free(params);
      return out;
  }

  struct TreeNode * init_binop(enum SecrecOperator op, YYLTYPE * loc,
                               struct TreeNode * ret, struct TreeNode * body,
                               struct TreeNode * arg1, struct TreeNode * arg2)
  {
      struct TreeNode * out = treenode_init_opdef(op, loc);
      /* indentifier is added automatically by treenode_init_opdef! */
      treenode_appendChild(out, ret);
      treenode_appendChild(out, body);
      treenode_appendChild(out, arg1);
      treenode_appendChild(out, arg2);
      return out;
  }

  struct TreeNode * init_unop(enum SecrecOperator op, YYLTYPE * loc,
                              struct TreeNode * ret, struct TreeNode * body,
                              struct TreeNode * arg1)
  {
      struct TreeNode * out = treenode_init_opdef(op, loc);
      /* indentifier is added automatically by treenode_init_opdef! */
      treenode_appendChild(out, ret);
      treenode_appendChild(out, body);
      treenode_appendChild(out, arg1);
      return out;
  }

  struct TreeNode * ensure_rValue(struct TreeNode * node)
  {
      struct TreeNode * t = 0;

      if (treenode_type(node) == NODE_IDENTIFIER) {
          t = treenode_init(NODE_EXPR_RVARIABLE, treenode_location(node));
          treenode_appendChild(t, treenode_childAt(node, 0));
          return t;
      } else {
          return node;
      }
  }

  struct TreeNode * add_vardecl(struct TreeNode * node1, struct TreeNode * node2, YYLTYPE * loc)
  {
      struct TreeNode * ret;
      if (treenode_type(node2) == NODE_STMT_COMPOUND) {
          if (treenode_numChildren(node2) > 0) {
              ret = node2;
              treenode_prependChild(ret, node1);
              treenode_setLocation(ret, loc);
          } else {
              treenode_free(node2);
              ret = treenode_init(NODE_STMT_COMPOUND, loc);
              treenode_appendChild(ret, node1);
          }
      } else {
          ret = treenode_init(NODE_STMT_COMPOUND, loc);
          treenode_appendChild(ret, node1);
          treenode_appendChild(ret, node2);
      }
      return ret;
  }

  struct TreeNode * add_stmt(struct TreeNode * node1, struct TreeNode * node2, YYLTYPE * loc)
  {
      struct TreeNode * ret;
      if (treenode_type(node2) == NODE_STMT_COMPOUND) {
          if (treenode_numChildren(node2) > 0) {
              ret = node2;
              treenode_prependChild(ret, node1);
              treenode_setLocation(ret, loc);
          } else {
              treenode_free(node2);
              ret = node1;
          }
      } else {
          if (treenode_type(node1) == NODE_STMT_COMPOUND && treenode_numChildren(node1) <= 0) {
              treenode_free(node1);
              ret = node2;
          } else {
              ret = treenode_init(NODE_STMT_COMPOUND, loc);
              treenode_appendChild(ret, node1);
              treenode_appendChild(ret, node2);
          }
      }
      return ret;
  }

%}

%define api.pure
%locations
%error-verbose
%glr-parser
%lex-param {yyscan_t yyscanner}
%parse-param {yyscan_t yyscanner}
%parse-param {TYPE_TREENODE *parseTree}

%destructor { treenode_free($$); } <treenode>

/* Identifiers: */
%token <str> IDENTIFIER

/* Keywords: */
%token ASSERT BOOL BREAK BYTESFROMSTRING CAT CONTINUE CREF DECLASSIFY DO DOMAIN
%token DOMAINID ELSE FALSE_B FLOAT FLOAT32 FLOAT64 FOR IF IMPORT INT INT16 INT32 INT64
%token INT8 KIND MODULE OPERATOR PRINT PRIVATE PUBLIC REF RESHAPE RETURN SHAPE SIZE
%token STRING STRINGFROMBYTES SYSCALL TEMPLATE TOSTRING TRUE_B UINT UINT16 UINT32
%token UINT64 UINT8 WHILE VOID XOR_UINT XOR_UINT16 XOR_UINT32 XOR_UINT64 XOR_UINT8

/* Literals: */
%token <str> BIN_LITERAL
%token <str> DEC_LITERAL
%token <str> FLOAT_LITERAL
%token <str> HEX_LITERAL
%token <str> OCT_LITERAL
%token <str> STRING_LITERAL

/* Operators from higher to lower precedence: */
%right '=' AND_ASSIGN OR_ASSIGN XOR_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%left TYPE_QUAL
%left '?' ':'
%left LOR_OP
%left LAND_OP
%left '|'
%left '^'
%left '&'
%nonassoc EQ_OP NE_OP
%nonassoc '<' '>' LE_OP GE_OP
%left '+' '-'
%left '*' '/' '%'
%nonassoc INC_OP
%nonassoc DEC_OP
%right UINV UNEG UMINUS


%type <treenode> additive_expression
%type <treenode> assert_statement
%type <treenode> assignment_expression
%type <treenode> binop_def_helper
%type <treenode> bitwise_and_expression
%type <treenode> bitwise_or_expression
%type <treenode> bitwise_xor_expression
%type <treenode> bool_literal
%type <treenode> cast_expression
%type <treenode> cat_expression
%type <treenode> compound_statement
%type <treenode> conditional_expression
%type <treenode> datatype_specifier
%type <treenode> dimension_list
%type <treenode> dimensions
%type <treenode> dimtype_specifier
%type <treenode> domain_declaration
%type <treenode> dowhile_statement
%type <treenode> equality_expression
%type <treenode> expression
%type <treenode> expression_list
%type <treenode> float_literal
%type <treenode> for_initializer
%type <treenode> for_statement
%type <treenode> global_declaration
%type <treenode> global_declarations
%type <treenode> identifier
%type <treenode> if_statement
%type <treenode> import_declaration
%type <treenode> import_declarations
%type <treenode> index
%type <treenode> indices
%type <treenode> int_literal
%type <treenode> kind_declaration
%type <treenode> literal
%type <treenode> logical_and_expression
%type <treenode> logical_or_expression
%type <treenode> lvalue
%type <treenode> maybe_dimensions
%type <treenode> maybe_expression
%type <treenode> multiplicative_expression
%type <treenode> operator_definition
%type <treenode> postfix_expression
%type <treenode> postfix_op
%type <treenode> prefix_op
%type <treenode> primary_expression
%type <treenode> print_statement
%type <treenode> procedure_definition
%type <treenode> procedure_parameter
%type <treenode> procedure_parameter_list
%type <treenode> program
%type <treenode> qualified_expression
%type <treenode> relational_expression
%type <treenode> return_type_specifier
%type <treenode> sectype_specifier
%type <treenode> statement
%type <treenode> statement_list
%type <treenode> string_literal
%type <treenode> subscript
%type <treenode> syscall_parameter
%type <treenode> syscall_parameters
%type <treenode> syscall_statement
%type <treenode> template_declaration
%type <treenode> template_quantifier
%type <treenode> template_quantifiers
%type <treenode> type_specifier
%type <treenode> unary_expression
%type <treenode> unop_def_helper
%type <treenode> variable_declaration
%type <treenode> variable_initialization
%type <treenode> variable_initializations
%type <treenode> while_statement

%type <integer_literal> int_literal_helper
%type <nothing> module

/* Starting nonterminal: */
%start module

%%

/*******************************************************************************
  Program and variable declarations:
*******************************************************************************/

module
 : MODULE identifier ';' program
   {
     $$ = 0;
     *parseTree = treenode_init (NODE_MODULE, &@$);
     treenode_appendChild (*parseTree, $4);
     treenode_appendChild (*parseTree, $2);
   }
 | program
   {
    $$ = 0;
    *parseTree = treenode_init (NODE_MODULE, &@$);
    treenode_appendChild (*parseTree, $1);
   }
 ;

program
 : import_declarations global_declarations
   {
     $$ = treenode_init (NODE_PROGRAM, &@$);
     treenode_moveChildren ($1, $$);
     treenode_moveChildren ($2, $$);
     treenode_free ($1);
     treenode_free ($2);
   }
 | global_declarations
   {
     $$ = $1;
   }
 ;

global_declarations
 : global_declarations global_declaration
   {
     $$ = $1;
     treenode_setLocation($$, &@$);
     treenode_appendChild($$, $2);
   }
 | global_declaration
   {
     $$ = treenode_init(NODE_PROGRAM, &@$);
     treenode_appendChild($$, $1);
   }
 ;

import_declarations
 : import_declarations import_declaration
   {
     $$ = $1;
     treenode_appendChild ($$, $2);
   }
 | import_declaration
   {
    $$ = treenode_init (NODE_INTERNAL_USE, &@$);
    treenode_appendChild ($$, $1);
   }
 ;

import_declaration
 : IMPORT identifier ';'
   {
     $$ = treenode_init (NODE_IMPORT, &@$);
     treenode_appendChild ($$, $2);
   }
 ;

global_declaration
 : variable_declaration ';'
 | domain_declaration ';'
 | kind_declaration ';'
 | procedure_definition
 | template_declaration
 ;

kind_declaration
 : KIND identifier
   {
     $$ = treenode_init (NODE_KIND, &@$);
     treenode_appendChild($$, $2);
   }
 ;

domain_declaration
 : DOMAIN identifier identifier
   {
     $$ = treenode_init (NODE_DOMAIN, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $3);
   }
 ;

maybe_dimensions
 : /* nothing */
   {
     $$ = (struct TreeNode *) treenode_init(NODE_DIMENSIONS, &@$);
   }
 | dimensions
   {
     $$ = $1;
   }
 ;

variable_initialization
 : identifier maybe_dimensions
   {
     $$ = (struct TreeNode *) treenode_init (NODE_VAR_INIT, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $2);
   }
 | identifier maybe_dimensions '=' expression
   {
     $$ = (struct TreeNode *) treenode_init (NODE_VAR_INIT, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, ensure_rValue ($4));
   }
 ;

variable_initializations
 : variable_initializations ',' variable_initialization
   {
     $$ = $1;
     treenode_setLocation($$, &@$);
     treenode_appendChild($$, $3);
   }
 | variable_initialization
   {
     $$ = treenode_init(NODE_INTERNAL_USE, &@$);
     treenode_appendChild($$, $1);
   }
 ;

variable_declaration
 : type_specifier variable_initializations
   {
     $$ = treenode_init(NODE_DECL, &@$);
     treenode_appendChild ($$, $1);
     treenode_moveChildren ($2, $$);
     treenode_free ($2);
   }
 ;

procedure_parameter
 : type_specifier identifier
   {
     TreeNode* var_init = treenode_init (NODE_VAR_INIT, &@$);
     treenode_appendChild (var_init, $2);
     treenode_appendChild (var_init, treenode_init (NODE_DIMENSIONS, &@$));

     $$ = treenode_init (NODE_DECL, &@$);
     treenode_appendChild ($$, $1);
     treenode_appendChild ($$, var_init);
   }
 ;

dimensions
 : '(' dimension_list ')'
   {
     $$ = $2;
   }
 ;

expression_list
 : expression_list ',' expression
   { $$ = $1;
     treenode_setLocation($$, &@$);
     treenode_appendChild($$, $3);
   }
 | expression
   {
     $$ = treenode_init(NODE_INTERNAL_USE, &@$);
     treenode_appendChild($$, $1);
   }
 ;

dimension_list
 : expression_list
   {
     $$ = treenode_init (NODE_DIMENSIONS, &@$);
     treenode_moveChildren ($1, $$);
     treenode_free ($1);
   }
 ;


/*******************************************************************************
  Types:
*******************************************************************************/

type_specifier
 : sectype_specifier datatype_specifier dimtype_specifier
   {
     $$ = (struct TreeNode *) treenode_init(NODE_TYPETYPE, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $3);
   }
 | sectype_specifier datatype_specifier
   {
     $$ = (struct TreeNode *) treenode_init(NODE_TYPETYPE, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, (struct TreeNode *) treenode_init_dimTypeF(0, &@$));
   }
 | datatype_specifier dimtype_specifier
   {
     $$ = (struct TreeNode *) treenode_init(NODE_TYPETYPE, &@$);
     treenode_appendChild($$, (struct TreeNode *) treenode_init_publicSecTypeF (&@$));
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $2);
   }
 | datatype_specifier
   {
     $$ = (struct TreeNode *) treenode_init(NODE_TYPETYPE, &@$);
     treenode_appendChild($$, (struct TreeNode *) treenode_init_publicSecTypeF (&@$));
     treenode_appendChild($$, $1);
     treenode_appendChild($$, (struct TreeNode *) treenode_init_dimTypeF(0, &@$));
   }
 ;

sectype_specifier
 : PUBLIC
   {
     $$ = (struct TreeNode *) treenode_init_publicSecTypeF (&@$);
   }
 | identifier
   {
     $$ = (struct TreeNode *) treenode_init_privateSecTypeF(&@$);
     treenode_appendChild($$, (struct TreeNode *) $1);
   }
 ;

datatype_specifier
 : BOOL        { $$ = treenode_init_dataTypeF(DATATYPE_BOOL,       &@$); }
 | INT         { $$ = treenode_init_dataTypeF(DATATYPE_INT64,      &@$); }
 | UINT        { $$ = treenode_init_dataTypeF(DATATYPE_UINT64,     &@$); }
 | INT8        { $$ = treenode_init_dataTypeF(DATATYPE_INT8,       &@$); }
 | UINT8       { $$ = treenode_init_dataTypeF(DATATYPE_UINT8,      &@$); }
 | INT16       { $$ = treenode_init_dataTypeF(DATATYPE_INT16,      &@$); }
 | UINT16      { $$ = treenode_init_dataTypeF(DATATYPE_UINT16,     &@$); }
 | INT32       { $$ = treenode_init_dataTypeF(DATATYPE_INT32,      &@$); }
 | UINT32      { $$ = treenode_init_dataTypeF(DATATYPE_UINT32,     &@$); }
 | INT64       { $$ = treenode_init_dataTypeF(DATATYPE_INT64,      &@$); }
 | UINT64      { $$ = treenode_init_dataTypeF(DATATYPE_UINT64,     &@$); }
 | STRING      { $$ = treenode_init_dataTypeF(DATATYPE_STRING,     &@$); }
 | XOR_UINT8   { $$ = treenode_init_dataTypeF(DATATYPE_XOR_UINT8,  &@$); }
 | XOR_UINT16  { $$ = treenode_init_dataTypeF(DATATYPE_XOR_UINT16, &@$); }
 | XOR_UINT32  { $$ = treenode_init_dataTypeF(DATATYPE_XOR_UINT32, &@$); }
 | XOR_UINT64  { $$ = treenode_init_dataTypeF(DATATYPE_XOR_UINT64, &@$); }
 | XOR_UINT    { $$ = treenode_init_dataTypeF(DATATYPE_XOR_UINT64, &@$); }
 | FLOAT       { $$ = treenode_init_dataTypeF(DATATYPE_FLOAT32,    &@$); }
 | FLOAT32     { $$ = treenode_init_dataTypeF(DATATYPE_FLOAT32,    &@$); }
 | FLOAT64     { $$ = treenode_init_dataTypeF(DATATYPE_FLOAT64,    &@$); }
 ;

dimtype_specifier
 : '[' '[' int_literal_helper ']' ']'
   {
      $$ = (struct TreeNode *) treenode_init_dimTypeF ($3, &@$);
   }
 ;

/*******************************************************************************
  Templates:
*******************************************************************************/

template_declaration
 : TEMPLATE '<' template_quantifiers '>' procedure_definition
   {
     $$ = (struct TreeNode*) treenode_init(NODE_TEMPLATE_DECL, &@$);
     treenode_appendChild($$, $3);
     treenode_appendChild($$, $5);
   }
 ;

template_quantifiers
 : template_quantifiers ',' template_quantifier
   {
     $$ = $1;
     treenode_appendChild($$, $3);
   }
 | template_quantifier
   {
     $$ = (struct TreeNode*) treenode_init(NODE_INTERNAL_USE, &@$);
     treenode_appendChild($$, $1);
   }
 ;

template_quantifier
 : DOMAIN identifier ':' identifier
  {
    $$ = (struct TreeNode*) treenode_init(NODE_TEMPLATE_QUANT, &@$);
    treenode_appendChild($$, $2);
    treenode_appendChild($$, $4);
  }
 | DOMAIN identifier
  {
    $$ = (struct TreeNode*) treenode_init(NODE_TEMPLATE_QUANT, &@$);
    treenode_appendChild($$, $2);
  }
 ;

/*******************************************************************************
  Procedures:
*******************************************************************************/

return_type_specifier
 : VOID
   {
     $$ = (struct TreeNode *) treenode_init(NODE_TYPEVOID, &@$);
   }
 | type_specifier
 ;

procedure_definition
 : operator_definition
 | return_type_specifier identifier '(' ')' compound_statement
   {
     $$ = treenode_init(NODE_PROCDEF, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $5);
   }
 | return_type_specifier identifier '(' procedure_parameter_list ')' compound_statement
   {
     $$ = treenode_init(NODE_PROCDEF, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $6);
     treenode_moveChildren ($4, $$);
     treenode_free($4);
   }
 ;

procedure_parameter_list
 : procedure_parameter_list ',' procedure_parameter
   { $$ = $1;
     treenode_setLocation($$, &@$);
     treenode_appendChild($$, $3);
   }
 | procedure_parameter
   {
     $$ = treenode_init(NODE_INTERNAL_USE, &@$);
     treenode_appendChild($$, $1);
   }
 ;

binop_def_helper
 : '(' procedure_parameter ',' procedure_parameter ')' compound_statement
   {
     $$ = treenode_init(NODE_INTERNAL_USE, &@$);
     treenode_appendChild($$, $6);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $4);
   }
 ;

unop_def_helper
 : '(' procedure_parameter ')' compound_statement
   {
     $$ = treenode_init(NODE_INTERNAL_USE, &@$);
     treenode_appendChild($$, $4);
     treenode_appendChild($$, $2);
   }
 ;

operator_definition
 :  return_type_specifier OPERATOR '+' binop_def_helper     { $$ = init_op(SCOP_BIN_ADD, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '&' binop_def_helper     { $$ = init_op(SCOP_BIN_BAND, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '|' binop_def_helper     { $$ = init_op(SCOP_BIN_BOR, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '/' binop_def_helper     { $$ = init_op(SCOP_BIN_DIV, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '>' binop_def_helper     { $$ = init_op(SCOP_BIN_GT, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '<' binop_def_helper     { $$ = init_op(SCOP_BIN_LT, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '%' binop_def_helper     { $$ = init_op(SCOP_BIN_MOD, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '*' binop_def_helper     { $$ = init_op(SCOP_BIN_MUL, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '-' binop_def_helper     { $$ = init_op(SCOP_BIN_SUB, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '^' binop_def_helper     { $$ = init_op(SCOP_BIN_XOR, &@$, $1, $4); }
 |  return_type_specifier OPERATOR EQ_OP binop_def_helper   { $$ = init_op(SCOP_BIN_EQ, &@$, $1, $4); }
 |  return_type_specifier OPERATOR GE_OP binop_def_helper   { $$ = init_op(SCOP_BIN_GE, &@$, $1, $4); }
 |  return_type_specifier OPERATOR LAND_OP binop_def_helper { $$ = init_op(SCOP_BIN_LAND, &@$, $1, $4); }
 |  return_type_specifier OPERATOR LE_OP binop_def_helper   { $$ = init_op(SCOP_BIN_LE, &@$, $1, $4); }
 |  return_type_specifier OPERATOR LOR_OP binop_def_helper  { $$ = init_op(SCOP_BIN_LOR, &@$, $1, $4); }
 |  return_type_specifier OPERATOR NE_OP binop_def_helper   { $$ = init_op(SCOP_BIN_NE, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '-' unop_def_helper      { $$ = init_op(SCOP_UN_MINUS, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '!' unop_def_helper      { $$ = init_op(SCOP_UN_NEG, &@$, $1, $4); }
 ;

/*******************************************************************************
  Statements:
*******************************************************************************/

compound_statement
 : '{' '}' { $$ = treenode_init(NODE_STMT_COMPOUND, &@$); }
 | '{' statement_list '}' { $$ = $2; treenode_setLocation($$, &@$); }
 ;

statement_list
 : variable_declaration ';' statement_list
   {
     $$ = add_vardecl($1, $3, &@$);
   }
 | statement statement_list
   {
     $$ = add_stmt($1, $2, &@$);
   }
 | variable_declaration ';'
 | statement
 ;

statement
 : compound_statement
 | if_statement
 | for_statement
 | while_statement
 | dowhile_statement
 | assert_statement
 | print_statement
 | syscall_statement
 | RETURN expression ';'
   {
     $$ = treenode_init(NODE_STMT_RETURN, &@$);
     treenode_appendChild($$, ensure_rValue($2));
   }
 | RETURN ';'
   {
     $$ = treenode_init(NODE_STMT_RETURN, &@$);
   }
 | CONTINUE ';'
   {
     $$ = treenode_init(NODE_STMT_CONTINUE, &@$);
   }
 | BREAK ';'
   {
     $$ = treenode_init(NODE_STMT_BREAK, &@$);
   }
 | ';'
   { /* We also use the compound statement construct for empty statements: */
     $$ = treenode_init(NODE_STMT_COMPOUND, &@$);
   }
 | expression ';'
   {
     $$ = treenode_init(NODE_STMT_EXPR, &@$);
     treenode_appendChild($$, $1);
   }
 ;

if_statement
 : IF '(' expression ')' statement ELSE statement
   {
     $$ = treenode_init(NODE_STMT_IF, &@$);
     treenode_appendChild($$, ensure_rValue($3));
     treenode_appendChild($$, $5);
     treenode_appendChild($$, $7);
   }
 | IF '(' expression ')' statement
   {
     $$ = treenode_init(NODE_STMT_IF, &@$);
     treenode_appendChild($$, ensure_rValue($3));
     treenode_appendChild($$, $5);
   }
 ;

for_initializer
 : maybe_expression
 | variable_declaration
 ;

for_statement
 : FOR '(' for_initializer  ';'
           maybe_expression ';'
           maybe_expression ')' statement
   {
     $$ = treenode_init(NODE_STMT_FOR, &@$);
     treenode_appendChild($$, ensure_rValue($3));
     treenode_appendChild($$, ensure_rValue($5));
     treenode_appendChild($$, ensure_rValue($7));
     treenode_appendChild($$, $9);
   }
 ;

maybe_expression
 : /* empty */ { $$ = treenode_init(NODE_EXPR_NONE, &@$); }
 | expression
 ;

while_statement
 : WHILE '(' expression ')' statement
   {
     $$ = treenode_init(NODE_STMT_WHILE, &@$);
     treenode_appendChild($$, ensure_rValue($3));
     treenode_appendChild($$, $5);
   }
 ;

print_statement
 : PRINT '(' expression_list ')' ';'
   {
     $$ = treenode_init(NODE_STMT_PRINT, &@$);
     treenode_appendChild($$, $3);
   }
 ;

dowhile_statement
 : DO statement WHILE '(' expression ')' ';'
   {
     $$ = treenode_init(NODE_STMT_DOWHILE, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, ensure_rValue($5));
   }
 ;

assert_statement
 : ASSERT '(' expression ')' ';'
   {
     $$ = treenode_init(NODE_STMT_ASSERT, &@$);
     treenode_appendChild($$, ensure_rValue($3));
   }
 ;

syscall_statement
  : SYSCALL '(' string_literal ')' ';'
    {
     $$ = treenode_init(NODE_STMT_SYSCALL, &@$);
     treenode_appendChild ($$, $3);
    }
  | SYSCALL '(' string_literal ',' syscall_parameters ')' ';'
    {
     $$ = treenode_init(NODE_STMT_SYSCALL, &@$);
     treenode_appendChild ($$, $3);
     treenode_moveChildren ($5, $$);
    }
  ;

syscall_parameters
  : syscall_parameters ',' syscall_parameter
    {
      $$ = $1;
      treenode_setLocation($$, &@$);
      treenode_appendChild($$, $3);
    }
  | syscall_parameter
    {
      $$ = treenode_init(NODE_INTERNAL_USE, &@$);
      treenode_appendChild($$, ensure_rValue($1));
    }
  ;

syscall_parameter
  : expression
    {
      $$ = treenode_init(NODE_PUSH, &@$);
      treenode_appendChild($$, ensure_rValue($1));
    }
  | REF identifier
    {
      $$ = treenode_init(NODE_PUSHREF, &@$);
      treenode_appendChild($$, ensure_rValue($2));
    }
  | CREF expression
    {
      $$ = treenode_init(NODE_PUSHCREF, &@$);
      treenode_appendChild($$, ensure_rValue($2));
    }
  ;

/*******************************************************************************
  Indices: not strictly expressions as they only appear in specific context
*******************************************************************************/

subscript
  : '[' indices ']'
    {
      $$ = $2;
      treenode_setLocation($$, &@$);
    }
  ;

indices
  : indices ',' index
    {
      $$ = $1;
      treenode_setLocation($$, &@$);
      treenode_appendChild($$, $3);
    }
  | index
    {
      $$ = treenode_init(NODE_SUBSCRIPT, &@$);
      treenode_appendChild($$, $1);
    }
  ;

/* Precedence of slicing operator? Right now it binds weakest as it can appear
 * in very specific context. However, if we ever wish for "foo : bar" evaluate
 * to value in some other context we need to figure out sane precedence.
 */
index
 : maybe_expression ':' maybe_expression
   {
     $$ = treenode_init(NODE_INDEX_SLICE, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | expression
   {
     $$ = treenode_init(NODE_INDEX_INT, &@$);
     treenode_appendChild($$, ensure_rValue($1));
   }
 ;

/*******************************************************************************
  Expressions:
*******************************************************************************/

lvalue
 : identifier
   {
     $$ = treenode_init(NODE_LVALUE, &@$);
     treenode_appendChild($$, $1);
   }
 | identifier subscript
   {
     $$ = treenode_init(NODE_LVALUE, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $2);
   }
 ;

expression
 : assignment_expression
 ;

assignment_expression /* WARNING: RIGHT RECURSION */
 : lvalue '=' assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_ASSIGN, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, ensure_rValue($3));
   }
 | lvalue MUL_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_ASSIGN_MUL, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, ensure_rValue($3));
   }
 | lvalue DIV_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_ASSIGN_DIV, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, ensure_rValue($3));
   }
 | lvalue MOD_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_ASSIGN_MOD, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, ensure_rValue($3));
   }
 | lvalue ADD_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_ASSIGN_ADD, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, ensure_rValue($3));
   }
 | lvalue SUB_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_ASSIGN_SUB, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, ensure_rValue($3));
   }
 | lvalue AND_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_ASSIGN_AND, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, ensure_rValue($3));
   }
 | lvalue OR_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_ASSIGN_OR, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, ensure_rValue($3));
   }
 | lvalue XOR_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_ASSIGN_XOR, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, ensure_rValue($3));
   }
 | qualified_expression
 ;

qualified_expression
 : qualified_expression TYPE_QUAL sectype_specifier
   {
     $$ = treenode_init(NODE_EXPR_TYPE_QUAL, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);

   }
 | qualified_expression TYPE_QUAL datatype_specifier
   {
     $$ = treenode_init(NODE_EXPR_TYPE_QUAL, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);

   }
 | qualified_expression TYPE_QUAL dimtype_specifier
   {
     $$ = treenode_init(NODE_EXPR_TYPE_QUAL, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | conditional_expression
 ;

conditional_expression
 : logical_or_expression '?' expression ':' expression
   {
     $$ = treenode_init(NODE_EXPR_TERNIF, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, $3);
     treenode_appendChild($$, $5);
   }
 | logical_or_expression
 ;

logical_or_expression
 : logical_or_expression LOR_OP logical_and_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_LOR, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | logical_and_expression
 ;

logical_and_expression
 : logical_and_expression LAND_OP bitwise_or_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_LAND, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | bitwise_or_expression
 ;

bitwise_or_expression
 : bitwise_or_expression '|' bitwise_xor_expression
   {
     $$ = treenode_init(NODE_EXPR_BITWISE_OR, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | bitwise_xor_expression
 ;

bitwise_xor_expression
 : bitwise_xor_expression '^' bitwise_and_expression
   {
     $$ = treenode_init(NODE_EXPR_BITWISE_XOR, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | bitwise_and_expression
 ;

bitwise_and_expression
 : bitwise_and_expression '&' equality_expression
   {
     $$ = treenode_init(NODE_EXPR_BITWISE_AND, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | equality_expression
 ;

equality_expression
 : equality_expression EQ_OP relational_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_EQ, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | equality_expression NE_OP relational_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_NE, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | relational_expression
 ;

relational_expression
 : relational_expression LE_OP additive_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_LE, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | relational_expression GE_OP additive_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_GE, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | relational_expression '<' additive_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_LT, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | relational_expression '>' additive_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_GT, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | additive_expression
 ;

additive_expression
 : additive_expression '+' multiplicative_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_ADD, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | additive_expression '-' multiplicative_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_SUB, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | multiplicative_expression
 ;

multiplicative_expression
 : multiplicative_expression '*' cast_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_MUL, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | multiplicative_expression '/' cast_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_DIV, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | multiplicative_expression '%' cast_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_MOD, &@$);
     treenode_appendChild($$, ensure_rValue($1));
     treenode_appendChild($$, ensure_rValue($3));
   }
 | cast_expression
 ;

cast_expression
 : '(' datatype_specifier ')' cast_expression
   {
     $$ = treenode_init(NODE_EXPR_CAST, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $4);
   }
 | prefix_op
 ;

prefix_op
 : INC_OP lvalue
   {
     $$ = treenode_init(NODE_EXPR_PREFIX_INC, &@$);
     treenode_appendChild($$, ensure_rValue($2));
   }
 | DEC_OP lvalue
   {
     $$ = treenode_init(NODE_EXPR_PREFIX_DEC, &@$);
     treenode_appendChild($$, ensure_rValue($2));
   }
 | postfix_op

postfix_op
 : lvalue INC_OP
   {
     $$ = treenode_init(NODE_EXPR_POSTFIX_INC, &@$);
     treenode_appendChild($$, ensure_rValue($1));
   }
 | lvalue DEC_OP
   {
     $$ = treenode_init(NODE_EXPR_POSTFIX_DEC, &@$);
     treenode_appendChild($$, ensure_rValue($1));
   }
 | unary_expression

unary_expression
 : '-' cast_expression %prec UMINUS
   {
     $$ = treenode_init(NODE_EXPR_UMINUS, &@$);
     treenode_appendChild($$, ensure_rValue($2));
   }
 | '!' cast_expression %prec UNEG
   {
     $$ = treenode_init(NODE_EXPR_UNEG, &@$);
     treenode_appendChild($$, ensure_rValue($2));
   }
  | '~' cast_expression %prec UINV
   {
     $$ = treenode_init(NODE_EXPR_UINV, &@$);
     treenode_appendChild($$, ensure_rValue($2));
   }
 | postfix_expression
 ;

cat_expression
 : CAT '(' expression ',' expression ',' int_literal ')'
   {
     $$ = treenode_init(NODE_EXPR_CAT, &@$);
     treenode_appendChild($$, $3);
     treenode_appendChild($$, $5);
     treenode_appendChild($$, $7);
   }
 | CAT '(' expression ',' expression ')'
   {
     $$ = treenode_init(NODE_EXPR_CAT, &@$);
     treenode_appendChild($$, $3);
     treenode_appendChild($$, $5);
   }
 ;

postfix_expression
: DECLASSIFY '(' expression ')'
  {
    $$ = treenode_init(NODE_EXPR_DECLASSIFY, &@$);
    treenode_appendChild($$, $3);
  }
 | SIZE '(' expression ')'
   {
     $$ = treenode_init(NODE_EXPR_SIZE, &@$);
     treenode_appendChild($$, $3);
   }
 | SHAPE '(' expression ')'
   {
     $$ = treenode_init(NODE_EXPR_SHAPE, &@$);
     treenode_appendChild($$, $3);
   }
 | cat_expression
 | DOMAINID '(' sectype_specifier ')'
   {
     $$ = treenode_init (NODE_EXPR_DOMAINID, &@$);
     treenode_appendChild($$, $3);
   }
 | RESHAPE '(' expression_list ')'
   {
     $$ = treenode_init(NODE_EXPR_RESHAPE, &@$);
     treenode_moveChildren ($3, $$);
     treenode_free($3);
   }
 | TOSTRING '(' expression ')'
   {
     $$ = treenode_init(NODE_EXPR_TOSTRING, &@$);
     treenode_appendChild($$, $3);
   }
 | STRINGFROMBYTES '(' expression ')'
   {
     $$ = treenode_init(NODE_EXPR_STRING_FROM_BYTES, &@$);
     treenode_appendChild($$, $3);
   }
 | BYTESFROMSTRING '(' expression ')'
   {
     $$ = treenode_init(NODE_EXPR_BYTES_FROM_STRING, &@$);
     treenode_appendChild($$, $3);
   }
 | identifier '(' ')'
   {
     $$ = treenode_init(NODE_EXPR_PROCCALL, &@$);
     treenode_appendChild($$, $1);
   }
 | identifier '(' expression_list ')'
   {
     $$ = treenode_init(NODE_EXPR_PROCCALL, &@$);
     treenode_appendChild($$, $1);
     treenode_moveChildren ($3, $$);
     treenode_free($3);
   }
 | postfix_expression subscript
   {
     $$ = treenode_init(NODE_EXPR_INDEX, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $2);
   }
 | primary_expression
 ;

primary_expression
 : '(' expression ')'
   {
     $$ = $2;
     treenode_setLocation($$, &@$);
   }
 | identifier
   {
     $$ = treenode_init(NODE_EXPR_RVARIABLE, &@$);
     treenode_appendChild($$, $1);
   }
 | literal
 ;

int_literal_helper
 : BIN_LITERAL { $$ = convert_to_base ($1,  2); free ($1); }
 | OCT_LITERAL { $$ = convert_to_base ($1,  8); free ($1); }
 | DEC_LITERAL { $$ = convert_to_base ($1, 10); free ($1); }
 | HEX_LITERAL { $$ = convert_to_base ($1, 16); free ($1); }
 ;

int_literal
 : int_literal_helper
   {
     $$ = treenode_init_int($1, &@$);
   }
 ;

float_literal
 : FLOAT_LITERAL
   {
     $$ = treenode_init_float ($1, &@$);
     free ($1);
   }
 ;

string_literal
 : STRING_LITERAL
   {
     $$ = treenode_init_string($1, &@$);
     free($1);
   }
 ;

bool_literal
 : TRUE_B   { $$ = treenode_init_bool(0 == 0, &@$); }
 | FALSE_B  { $$ = treenode_init_bool(1 != 1, &@$); }
 ;

literal
 : int_literal
 | string_literal
 | bool_literal
 | float_literal
 ;

identifier
 : IDENTIFIER
   {
     $$ = treenode_init_identifier($1, &@$);
     free($1);
   }
 ;

%%

void yyerror(YYLTYPE *loc, yyscan_t yyscanner, TYPE_TREENODE *parseTree,
             const char *s)
{
    (void) yyscanner;
    (void) parseTree;
    fprintf(stderr, "(%d,%d)-(%d,%d): %s\n",
            loc->first_line, loc->first_column,
            loc->last_line, loc->last_column,
            s);
}

int sccparse(TYPE_TREENODEMODULE *result) {
    yyscan_t scanner;
    int r;
    yylex_init(&scanner);
    r = yyparse(scanner, result);
    yylex_destroy(scanner);
    return r;
}

int sccparse_file(FILE *input, TYPE_TREENODEMODULE *result) {
    yyscan_t scanner;
    int r;
    yylex_init(&scanner);
    yyset_in(input, scanner);
    r = yyparse(scanner, result);
    yylex_destroy(scanner);
    return r;
}

int sccparse_mem(const void *buf, size_t size, TYPE_TREENODEMODULE *result) {
    FILE *memoryFile;
    yyscan_t scanner;
    int r;
#ifdef _GNU_SOURCE
    memoryFile = fmemopen((void*) buf, size, "r");
    if (memoryFile == NULL) return 3;
#else
    memoryFile = tmpfile();
    if (memoryFile == NULL) return 3;
    if (fwrite(buf, 1, size, memoryFile) != size) {
        fclose(memoryFile);
        return 3;
    }
    rewind(memoryFile);
#endif

    yylex_init(&scanner);
    yyset_in(memoryFile, scanner);
    r = yyparse(scanner, result);
    yylex_destroy(scanner);
    fclose(memoryFile);
    return r;
}
