%require "2.4"
%{
  struct TreeNode;
  #include <assert.h>
  #include <stdio.h>
  #include "parser.h"
  #include "lex_secrec.h"
  #include "treenode.h"

  void yyerror(YYLTYPE *loc, yyscan_t yyscanner, TYPE_TREENODE *parseTree, const char *s);

  struct TreeNode *ensure_rValue(struct TreeNode *node) {
     struct TreeNode *t = 0;

     if (treenode_type(node) == NODE_IDENTIFIER) {;
         t = treenode_init(NODE_EXPR_RVARIABLE, treenode_location(node));
         treenode_appendChild(t, treenode_childAt(node, 0));
         return t;
     } else {
         return node;
     }
  }

  struct TreeNode *add_vardecl(struct TreeNode *node1, struct TreeNode *node2, YYLTYPE *loc)
  {
    struct TreeNode *ret;
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

  struct TreeNode *add_stmt(struct TreeNode *node1, struct TreeNode *node2, YYLTYPE *loc) {
    struct TreeNode *ret;
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
%token BOOL BREAK CONTINUE DECLASSIFY DO ELSE FOR FALSE_B IF PRIVATE PUBLIC PRINT
%token INT UINT INT8 UINT8 INT16 UINT16 INT32 UINT32 INT64 UINT64
%token RETURN STRING TRUE_B VOID WHILE ASSERT SIZE SHAPE RESHAPE CAT
%token DOMAIN KIND TEMPLATE SYSCALL PUSH PUSHREF PUSHCREF

/* Literals: */
%token <str> STRING_LITERAL
%token <str> INT_LITERAL
%token <str> UINT_LITERAL

/* Operators from higher to lower precedence: */
%right '=' ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%left LOR_OP
%left LAND_OP
%nonassoc EQ_OP NE_OP
%nonassoc '<' '>' LE_OP GE_OP
%left '+' '-'
%left '*' '/' '%'
%nonassoc INC_OP
%nonassoc DEC_OP
%right UNEG UMINUS


%type <treenode> global_declarations
%type <treenode> global_declaration
%type <treenode> variable_declaration
%type <treenode> domain_declaration
%type <treenode> kind_declaration
%type <treenode> procedure_definition
%type <treenode> initializer
%type <treenode> dimensions
%type <treenode> dimension_list
%type <treenode> type_specifier
%type <treenode> procedure_type_specifier
%type <treenode> datatype_specifier
%type <treenode> sectype_specifier
%type <treenode> dimtype_specifier
%type <treenode> subscript
%type <treenode> indices
%type <treenode> index
%type <treenode> procedure_parameter_list
%type <treenode> procedure_parameter
%type <treenode> compound_statement
%type <treenode> statement_list
%type <treenode> statement
%type <treenode> if_statement
%type <treenode> for_initializer
%type <treenode> for_statement
%type <treenode> maybe_expression
%type <treenode> while_statement
%type <treenode> print_statement
%type <treenode> dowhile_statement
%type <treenode> assert_statement
%type <treenode> expression
%type <treenode> assignment_expression
%type <treenode> lvalue
%type <treenode> conditional_expression
%type <treenode> logical_or_expression
%type <treenode> logical_and_expression
%type <treenode> equality_expression
%type <treenode> relational_expression
%type <treenode> additive_expression
%type <treenode> multiplicative_expression
%type <treenode> cast_expression
%type <treenode> unary_expression
%type <treenode> postfix_expression
%type <treenode> postfix_op
%type <treenode> prefix_op
%type <treenode> cat_expression
%type <treenode> argument_list
%type <treenode> primary_expression
%type <treenode> constant
%type <treenode> identifier
%type <treenode> uint_literal
%type <treenode> int_literal
%type <treenode> string_literal
%type <treenode> bool_literal
%type <treenode> template_declaration
%type <treenode> template_quantifiers
%type <treenode> template_quantifier

%type <nothing> program

/* Starting nonterminal: */
%start program

%%

/*******************************************************************************
  Program and variable declarations:
*******************************************************************************/

program
 : global_declarations
   { $$ = 0;
     *parseTree = $1;
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

global_declaration
 : variable_declaration ';'
 | procedure_definition
 | template_declaration
 | domain_declaration ';'
 | kind_declaration ';'
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

variable_declaration
 : type_specifier identifier
   {
     $$ = treenode_init(NODE_DECL, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, (struct TreeNode *) treenode_init(NODE_DIMENSIONS, &@$));
   }
 | type_specifier identifier '=' initializer
   {
     $$ = treenode_init(NODE_DECL, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, (struct TreeNode *) treenode_init(NODE_DIMENSIONS, &@$));
     treenode_appendChild($$, ensure_rValue($4));
   }
 | type_specifier identifier dimensions
   {
     $$ = treenode_init(NODE_DECL, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | type_specifier identifier dimensions '=' initializer
   {
     $$ = treenode_init(NODE_DECL, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
     treenode_appendChild($$, ensure_rValue($5));
   }
 ;

initializer
 : expression
 ;

dimensions
 : '(' dimension_list ')'
   {
     $$ = $2
   }
 ;

dimension_list
 : dimension_list ',' expression
   {
     $$ = $1;
     treenode_setLocation($$, &@$);
     treenode_appendChild($$, $3);
   }
 | expression
   {
     $$ = treenode_init(NODE_DIMENSIONS, &@$);
     treenode_appendChild($$, $1);
   }


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
 : BOOL
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_BOOL, &@$);
   }
 | INT
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_INT, &@$);
   }
 | UINT
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_UINT, &@$);
   }
 | INT8
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_INT8, &@$);
   }
 | UINT8
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_UINT8, &@$);
   }
 | INT16
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_INT16, &@$);
   }
 | UINT16
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_UINT16, &@$);
   }
 | INT32
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_INT32, &@$);
   }
 | UINT32
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_UINT32, &@$);
   }
 | INT64
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_INT64, &@$);
   }
 | UINT64
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_UINT64, &@$);
   }
 | STRING
   {
     $$ = (struct TreeNode *) treenode_init_dataTypeF(DATATYPE_STRING, &@$);
   }
 ;

dimtype_specifier
  : '[' '[' INT_LITERAL ']' ']'
    {
        $$ = (struct TreeNode *) treenode_init_dimTypeF(atoi($3), &@$);
        free ($3);
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

procedure_type_specifier
 : VOID
   {
     $$ = (struct TreeNode *) treenode_init(NODE_TYPEVOID, &@$);
   }
 | type_specifier
 ;

procedure_definition
 : procedure_type_specifier identifier '(' ')' compound_statement
   {
     $$ = treenode_init(NODE_PROCDEF, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $5);
   }
 | procedure_type_specifier identifier '(' procedure_parameter_list ')' compound_statement
   {
     unsigned i;
     unsigned n;

     $$ = treenode_init(NODE_PROCDEF, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $6);

     n = treenode_numChildren($4);
     for (i = 0; i < n; i++) {
         treenode_appendChild($$, treenode_childAt($4, i));
     }
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

procedure_parameter
 : type_specifier identifier
   {
     $$ = treenode_init(NODE_DECL, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $1);
   }
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
 | SYSCALL '(' string_literal ')' ';'
   {
     $$ = treenode_init(NODE_STMT_SYSCALL, &@$);
     treenode_appendChild ($$, $3);
   }
 | PUSH '(' expression ')' ';'
   {
     $$ = treenode_init(NODE_STMT_PUSH, &@$);
     treenode_appendChild ($$, ensure_rValue ($3));
   }
 | PUSHREF '(' identifier ')' ';'
   {
     $$ = treenode_init(NODE_STMT_PUSHREF, &@$);
     treenode_appendChild ($$, $3);
   }
 | PUSHCREF '(' identifier ')' ';'
   {
     $$ = treenode_init(NODE_STMT_PUSHCREF, &@$);
     treenode_appendChild ($$, $3);
   }
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

maybe_expression /* Helper nonterminal for expression? */
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
 : PRINT '(' expression ')' ';'
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

/*******************************************************************************
  Expressions:
*******************************************************************************/

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
 | conditional_expression
 ;

lvalue
/* : unary_expression
   {
     $$ = treenode_init(NODE_EXPR_LVARIABLE, &@$);
     treenode_appendChild($$, $1);
   }*/
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
 : logical_and_expression LAND_OP equality_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_LAND, &@$);
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
 | '(' sectype_specifier ')' cast_expression
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
    /* treenode_appendChild($$, ensure_rValue($1)); */
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
 | RESHAPE '(' argument_list ')'
   {
     unsigned i;
     unsigned n;

     $$ = treenode_init(NODE_EXPR_RESHAPE, &@$);
     n = treenode_numChildren($3);
     for (i = 0; i < n; i++) {
         treenode_appendChild($$, ensure_rValue(treenode_childAt($3, i)));
     }
     treenode_free($3);
   }
/* : postfix_expression '(' ')'*/
 | identifier '(' ')'
   {
     $$ = treenode_init(NODE_EXPR_PROCCALL, &@$);
     /* treenode_appendChild($$, ensure_rValue($1)); */
     treenode_appendChild($$, $1);
   }
/* | postfix_expression '(' argument_list ')' */
 | identifier '(' argument_list ')'
   {
     unsigned i;
     unsigned n;

     $$ = treenode_init(NODE_EXPR_PROCCALL, &@$);
     /* treenode_appendChild($$, ensure_rValue($1));*/
     treenode_appendChild($$, $1);
     n = treenode_numChildren($3);
     for (i = 0; i < n; i++) {
         treenode_appendChild($$, ensure_rValue(treenode_childAt($3, i)));
     }
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

argument_list
 : argument_list ',' expression
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
 | constant
 ;

uint_literal
  : UINT_LITERAL
   {
     $$ = treenode_init_uint(atoi($1), &@$);
     free($1);
   }

int_literal
 : INT_LITERAL
  {
    $$ = treenode_init_int(atoi($1), &@$);
    free($1);
  }

string_literal
 : STRING_LITERAL
   {
     $$ = treenode_init_string($1, &@$);
     free($1);
   }

bool_literal
 : TRUE_B
   {
     $$ = treenode_init_bool(0 == 0, &@$);
   }
 | FALSE_B
   {
     $$ = treenode_init_bool(1 != 1, &@$);
   }
 ;

constant
 : int_literal
 | uint_literal
 | string_literal
 | bool_literal
 ;

identifier
 : IDENTIFIER
   {
     $$ = treenode_init_identifier($1, &@$);
     free($1);
   }

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

int sccparse(TYPE_TREENODEPROGRAM *result) {
    yyscan_t scanner;
    int r;
    yylex_init(&scanner);
    r = yyparse(scanner, result);
    yylex_destroy(scanner);
    return r;
}

int sccparse_file(FILE *input, TYPE_TREENODEPROGRAM *result) {
    yyscan_t scanner;
    int r;
    yylex_init(&scanner);
    yyset_in(input, scanner);
    r = yyparse(scanner, result);
    yylex_destroy(scanner);
    return r;
}

int sccparse_mem(const void *buf, size_t size, TYPE_TREENODEPROGRAM *result) {
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
