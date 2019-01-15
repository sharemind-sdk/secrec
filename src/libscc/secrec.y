%require "2.4"
%{
  #include <assert.h>
  #include <stdio.h>
  #include <stdint.h>

  #include "Parser.h"
  #include "lex_secrec.h"
  #include "TreeNodeC.h"

  void yyerror(YYLTYPE *loc, yyscan_t yyscanner, TYPE_TREENODE *parseTree, const char * fileName, TYPE_STRINGTABLE table, const char *s);

  struct TreeNode * init_op(TYPE_STRINGTABLE table,
                            enum SecrecOperator op,
                            YYLTYPE * loc,
                            struct TreeNode * ret,
                            struct TreeNode * params);
  struct TreeNode * init_op(TYPE_STRINGTABLE table,
                            enum SecrecOperator op,
                            YYLTYPE * loc,
                            struct TreeNode * ret,
                            struct TreeNode * params)
  {
      struct TreeNode * out = treenode_init_opdef(table, op, loc);
      treenode_appendChild(out, ret);
      treenode_moveChildren(params, out);
      treenode_free(params);
      return out;
  }

  struct TreeNode * treenode_init_compound(struct TreeNode * stmts,
                                           YYLTYPE * loc);
  struct TreeNode * treenode_init_compound(struct TreeNode * stmts,
                                           YYLTYPE * loc)
  {
        struct TreeNode * out = treenode_init (NODE_STMT_COMPOUND, loc);
        struct TreeNode * cur = stmts;

        while (treenode_numChildren (cur) == 1) {
            struct TreeNode * next = treenode_childAt (cur, 0);
            if (treenode_type (next) != NODE_STMT_COMPOUND && treenode_type (next) != NODE_INTERNAL_USE) {
                break;
            }

            cur = next;
        }

        treenode_moveChildren (cur, out);
        treenode_free (cur);
        return out;
  }

  void treenode_add_stmt(struct TreeNode * stmts, struct TreeNode * node);
  void treenode_add_stmt(struct TreeNode * stmts, struct TreeNode * node) {
      if (treenode_type(node) == NODE_STMT_COMPOUND && treenode_numChildren(node) <= 0)
          treenode_free(node);
      else
          treenode_appendChild(stmts, node);
  }

%}

%define api.pure
%locations
%error-verbose
%glr-parser
%lex-param {yyscan_t yyscanner}
%parse-param { yyscan_t yyscanner }
%parse-param { TYPE_TREENODE *parseTree }
%parse-param { char const * fileName }
%parse-param { TYPE_STRINGTABLE table }
%initial-action {
  @$.first_line = 1u;
  @$.first_column = 1u;
  @$.last_line = 1u;
  @$.last_column = 1u;
  @$.filename = fileName;
}
%destructor { treenode_free($$); } <treenode>

 /* Invalid character: */
%token INVALID_CHARACTER
%token INVALID_STRING

 /* Keywords: */
%token ASSERT BOOL BREAK BYTESFROMSTRING CAST CAT CONTINUE CREF DECLASSIFY DIMENSIONALITY
%token DO DOMAIN DOMAINID ELSE FALSE_B FLOAT FLOAT32 FLOAT64 FOR IF IMPORT INT INT16
%token INT32 INT64 INT8 KIND MODULE OPERATOR PRINT PUBLIC REF RESHAPE RETURN
%token SHAPE SIZE STRING STRINGFROMBYTES SYSCALL TEMPLATE TOSTRING TRUE_B UINT UINT16
%token UINT32 UINT64 UINT8 WHILE VOID SYSCALL_RETURN TYPE STRUCT STRLEN READONLY
%token GET_FPU_STATE SET_FPU_STATE

 /* Identifiers: */
%token <str> IDENTIFIER

 /* Literals: */
%token <str> BIN_LITERAL
%token <str> DEC_LITERAL
%token <str> FLOAT_LITERAL
%token <str> HEX_LITERAL
%token <str> OCT_LITERAL
%token <str> STR_FRAGMENT
%token <str> STR_IDENTIFIER

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
%left SHL_OP SHR_OP
%left '+' '-'
%left '*' '/' '%'
%nonassoc INC_OP
%nonassoc DEC_OP
%right UINV UNEG UMINUS
%left '.'

%type <treenode> additive_expression
%type <treenode> assert_statement
%type <treenode> assignment_expression
%type <treenode> binop_def_helper
%type <treenode> bitwise_and_expression
%type <treenode> bitwise_or_expression
%type <treenode> bitwise_xor_expression
%type <treenode> bool_literal
%type <treenode> cast_definition
%type <treenode> cast_expression
%type <treenode> cat_expression
%type <treenode> compound_statement
%type <treenode> conditional_expression
%type <treenode> datatype_specifier
%type <treenode> primitive_datatype_specifier
%type <treenode> variable_datatype_specifier
%type <treenode> dimension_list
%type <treenode> dimensions
%type <treenode> dimtype_specifier
%type <treenode> maybe_dimtype_specifier
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
%type <treenode> qualified_type qualified_types
%type <treenode> relational_expression
%type <treenode> shift_expression
%type <treenode> return_type_specifier
%type <treenode> sectype_specifier
%type <treenode> maybe_sectype_specifier
%type <treenode> public_sectype_specifier
%type <treenode> private_sectype_specifier
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
%type <treenode> string_part
%type <treenode> structure_declaration
%type <treenode> attribute_list
%type <treenode> attribute
%type <treenode> datatype_declaration_list
%type <treenode> datatype_declaration
%type <treenode> datatype_declaration_param_list
%type <treenode> datatype_declaration_param
%type <treenode> maybe_annotation

%type <treenode> template_struct_datatype_specifier
%type <treenode> type_arguments type_argument

%type <secrec_operator> binop
%type <secrec_datatype> primitive_datatype
%type <str> datatype_identifier
%type <nothing> module

 /* Starting nonterminal: */
%start module

%%

 /*******************************************************************************
  * Program and variable declarations:                                          *
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
 | kind_declaration
 | procedure_definition
 | structure_declaration
 | template_declaration
 ;

kind_declaration
 : KIND identifier '{' datatype_declaration_list '}'
   {
     $$ = treenode_init (NODE_KIND, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $4);
   }
 ;

datatype_declaration_list
 : datatype_declaration_list datatype_declaration
   {
     $$ = $1;
     treenode_setLocation ($$, &@$);
     treenode_appendChild ($$, $2);
   }
 | datatype_declaration
   {
     $$ = treenode_init (NODE_INTERNAL_USE, &@$);
     treenode_appendChild ($$, $1);
   }

datatype_declaration
 : TYPE datatype_identifier ';'
   {
     $$ = treenode_init_dataTypeDecl ($2, &@$);
     treenode_appendChild ($$, treenode_init (NODE_INTERNAL_USE, &@$));
   }
 | TYPE datatype_identifier '{' datatype_declaration_param_list '}' ';'
 {
     $$ = treenode_init_dataTypeDecl ($2, &@$);
     treenode_appendChild ($$, $4);
 }

/* We want to allow uint8 as well which will not parse as an identifier */
datatype_identifier
 : IDENTIFIER
 | primitive_datatype
   {
     $$ = secrec_fund_datatype_to_string(table, $1);
   }

datatype_declaration_param_list
 : datatype_declaration_param_list ',' datatype_declaration_param
   {
     $$ = $1;
     treenode_setLocation ($$, &@$);
     treenode_appendChild ($$, $3);
   }
 | datatype_declaration_param
  {
    $$ = treenode_init (NODE_INTERNAL_USE, &@$);
    treenode_appendChild ($$, $1);
  }

datatype_declaration_param
 : PUBLIC '=' primitive_datatype
   {
     $$ = treenode_init_dataTypeDeclParamPublic ($3, &@$);
   }
/* This will be used in the future for values whose memory is managed
 * by the virtual machine

 | SIZE '=' int_literal_helper
   {
     $$ = treenode_init_dataTypeDeclParamSize ($3, &@$);
   }
*/

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
     $$ = treenode_init(NODE_DIMENSIONS, &@$);
   }
 | dimensions
 ;

variable_initialization
 : identifier maybe_dimensions
   {
     $$ = treenode_init (NODE_VAR_INIT, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $2);
   }
 | identifier maybe_dimensions '=' expression
   {
     $$ = treenode_init (NODE_VAR_INIT, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $4);
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
  * Types:                                                                      *
  *******************************************************************************/

type_specifier
 : maybe_sectype_specifier datatype_specifier maybe_dimtype_specifier
   {
     $$ = treenode_init(NODE_TYPETYPE, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $3);
   }
 ;

maybe_sectype_specifier
 : /* nothing */
   {
     $$ = treenode_init_publicSecTypeF (&@$);
   }
 | sectype_specifier
 ;

maybe_dimtype_specifier
 : /* nothing */
   {
     $$ = treenode_init(NODE_DIMTYPE_ZERO_F, &@$);
   }
 | dimtype_specifier
 ;

sectype_specifier
 : public_sectype_specifier
 | private_sectype_specifier
 ;

public_sectype_specifier
 : PUBLIC
   {
     $$ = treenode_init_publicSecTypeF (&@$);
   }
 ;

private_sectype_specifier
 : identifier
   {
     $$ = treenode_init_privateSecTypeF(&@$);
     treenode_appendChild($$, $1);
   }
 ;

datatype_specifier
 : primitive_datatype_specifier
 | template_struct_datatype_specifier
 | variable_datatype_specifier
 ;

primitive_datatype
 : BOOL       { $$ = DATATYPE_BOOL;       }
 | INT        { $$ = DATATYPE_INT64;      }
 | UINT       { $$ = DATATYPE_UINT64;     }
 | INT8       { $$ = DATATYPE_INT8;       }
 | UINT8      { $$ = DATATYPE_UINT8;      }
 | INT16      { $$ = DATATYPE_INT16;      }
 | UINT16     { $$ = DATATYPE_UINT16;     }
 | INT32      { $$ = DATATYPE_INT32;      }
 | UINT32     { $$ = DATATYPE_UINT32;     }
 | INT64      { $$ = DATATYPE_INT64;      }
 | UINT64     { $$ = DATATYPE_UINT64;     }
 | STRING     { $$ = DATATYPE_STRING;     }
 | FLOAT      { $$ = DATATYPE_FLOAT32;    }
 | FLOAT32    { $$ = DATATYPE_FLOAT32;    }
 | FLOAT64    { $$ = DATATYPE_FLOAT64;    }
 ;

primitive_datatype_specifier
 : primitive_datatype
   {
      $$ = treenode_init_dataTypeConstF($1, &@$);
   }
 ;

variable_datatype_specifier
 : identifier
   {
     $$ = treenode_init_dataTypeVarF(&@$);
     treenode_appendChild($$, $1);
   }
 ;

template_struct_datatype_specifier
 : identifier '<' type_arguments '>'
   {
      $$ = treenode_init(NODE_DATATYPE_TEMPLATE_F, &@$);
      treenode_appendChild($$, $1);
      treenode_moveChildren($3, $$);
      treenode_free($3);
   }
 ;

type_argument
 : identifier
   {
     $$ = treenode_init(NODE_TYPE_ARG_VAR, &@$);
     treenode_appendChild($$, $1);
   }
 | identifier '<' type_arguments '>'
   {
     $$ = treenode_init(NODE_TYPE_ARG_TEMPLATE, &@$);
     treenode_appendChild($$, $1);
     treenode_moveChildren($3, $$);
     treenode_free($3);
   }
 | primitive_datatype
   {
     $$ = treenode_init_typeArgDataTypeConst($1, &@$);
   }
 | int_literal
   {
     $$ = treenode_init(NODE_TYPE_ARG_DIM_TYPE_CONST, &@$);
     treenode_appendChild($$, $1);
   }
 | PUBLIC
   {
     $$ = treenode_init(NODE_TYPE_ARG_PUBLIC, &@$);
   }
 ;

type_arguments
 : type_arguments ',' type_argument
   {
     $$ = $1;
     treenode_appendChild($$, $3);
   }
 | type_argument
   {
     $$ = treenode_init(NODE_INTERNAL_USE, &@$);
     treenode_appendChild($$, $1);
   }
 ;

dimtype_specifier
 : '[' '[' int_literal ']' ']'
   {
      $$ = treenode_init(NODE_DIMTYPE_CONST_F, &@$);
      treenode_appendChild($$, $3);
   }
 | '[' '[' identifier ']' ']'
   {
      $$ = treenode_init(NODE_DIMTYPE_VAR_F, &@$);
      treenode_appendChild($$, $3);
   }
 ;

 /*******************************************************************************
  * Templates:                                                                  *
  *******************************************************************************/

template_declaration
 : TEMPLATE '<' template_quantifiers '>' procedure_definition
   {
     $$ = treenode_init(NODE_TEMPLATE_DECL, &@$);
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
     $$ = treenode_init(NODE_INTERNAL_USE, &@$);
     treenode_appendChild($$, $1);
   }
 ;

template_quantifier
 : DOMAIN identifier ':' identifier
  {
    $$ = treenode_init(NODE_TEMPLATE_QUANTIFIER_DOMAIN, &@$);
    treenode_appendChild($$, $2);
    treenode_appendChild($$, $4);
  }
 | DOMAIN identifier
  {
    $$ = treenode_init(NODE_TEMPLATE_QUANTIFIER_DOMAIN, &@$);
    treenode_appendChild($$, $2);
  }
 | DIMENSIONALITY identifier
  {
    $$ = treenode_init(NODE_TEMPLATE_QUANTIFIER_DIM, &@$);
    treenode_appendChild($$, $2);
  }
 | TYPE identifier
  {
    $$ = treenode_init(NODE_TEMPLATE_QUANTIFIER_DATA, &@$);
    treenode_appendChild($$, $2);
  }
 ;

 /*******************************************************************************
  * Structures:                                                                 *
  *******************************************************************************/

structure_declaration
 : STRUCT identifier '{' attribute_list '}'
   {
      $$ = treenode_init(NODE_STRUCT_DECL, &@$);
      treenode_appendChild($$, treenode_init(NODE_INTERNAL_USE, &@$));
      treenode_appendChild($$, $2);
      treenode_appendChild($$, $4);
   }
 | TEMPLATE '<' template_quantifiers '>' STRUCT identifier '{' attribute_list '}'
   {
     $$ = treenode_init(NODE_STRUCT_DECL, &@$);
     treenode_appendChild($$, $3);
     treenode_appendChild($$, $6);
     treenode_appendChild($$, $8);
   }
 ;

attribute_list
  : attribute_list attribute
    {
       $$ = $1;
       treenode_setLocation($$, &@$);
       treenode_appendChild($$, $2);
    }
  | /* empty */
    {
      $$ = treenode_init(NODE_INTERNAL_USE, &@$);
    }
  ;

attribute
  : type_specifier identifier ';'
    {
      $$ = treenode_init(NODE_ATTRIBUTE, &@$);
      treenode_appendChild($$, $1);
      treenode_appendChild($$, $2);
    }
  ;

 /*******************************************************************************
  * Procedures:                                                                 *
  *******************************************************************************/

maybe_annotation
 : /* empty */
   {
     $$ = treenode_init(NODE_ANNOTATION, &@$);
   }
 | '@' identifier
   {
     $$ = treenode_init(NODE_ANNOTATION, &@$);
     treenode_appendChild($$, $2);
   }
 | '@' identifier '(' STR_FRAGMENT ')'
   {
     $$ = treenode_init(NODE_ANNOTATION, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, treenode_init_str_fragment($4, &@$));
  }
 ;

return_type_specifier
 : VOID
   {
     $$ = treenode_init(NODE_TYPEVOID, &@$);
   }
 | type_specifier
 ;

procedure_definition
 : operator_definition
 | cast_definition
 | maybe_annotation return_type_specifier identifier '(' ')' compound_statement
   {
     $$ = treenode_init(NODE_PROCDEF, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $6);
   }
 | maybe_annotation return_type_specifier identifier '(' procedure_parameter_list ')' compound_statement
   {
     $$ = treenode_init(NODE_PROCDEF, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $7);
     treenode_moveChildren ($5, $$);
     treenode_free($5);
   }
 ;

procedure_parameter_list
 : procedure_parameter_list ',' procedure_parameter
   {
     $$ = $1;
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

binop
 : '+'     { $$ = SCOP_BIN_ADD;  }
 | '&'     { $$ = SCOP_BIN_BAND; }
 | '|'     { $$ = SCOP_BIN_BOR;  }
 | '/'     { $$ = SCOP_BIN_DIV;  }
 | '>'     { $$ = SCOP_BIN_GT;   }
 | '<'     { $$ = SCOP_BIN_LT;   }
 | '%'     { $$ = SCOP_BIN_MOD;  }
 | '*'     { $$ = SCOP_BIN_MUL;  }
 | '-'     { $$ = SCOP_BIN_SUB;  }
 | '^'     { $$ = SCOP_BIN_XOR;  }
 | EQ_OP   { $$ = SCOP_BIN_EQ;   }
 | GE_OP   { $$ = SCOP_BIN_GE;   }
 | LAND_OP { $$ = SCOP_BIN_LAND; }
 | LE_OP   { $$ = SCOP_BIN_LE;   }
 | LOR_OP  { $$ = SCOP_BIN_LOR;  }
 | NE_OP   { $$ = SCOP_BIN_NE;   }
 | SHL_OP  { $$ = SCOP_BIN_SHL;  }
 | SHR_OP  { $$ = SCOP_BIN_SHR;  }
 ;

 /*
  * We can not split 'unop' to make this even nicer. We get reduce/reduce conflict for some reason.
  * I think this is, again, limitation of the parser generator.
  */
operator_definition
 :  return_type_specifier OPERATOR binop binop_def_helper { $$ = init_op(table, $3, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '-' unop_def_helper    { $$ = init_op(table, SCOP_UN_MINUS, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '!' unop_def_helper    { $$ = init_op(table, SCOP_UN_NEG, &@$, $1, $4); }
 |  return_type_specifier OPERATOR '~' unop_def_helper    { $$ = init_op(table, SCOP_UN_INV, &@$, $1, $4); }
 ;

cast_definition
 : return_type_specifier CAST unop_def_helper
   {
     $$ = treenode_init_castdef(table, &@$);
     treenode_appendChild($$, $1);
     treenode_moveChildren($3, $$);
     treenode_free($3);
   }

 /*******************************************************************************
  * Statements:                                                                 *
  *******************************************************************************/

compound_statement
 : '{' '}'
   {
     $$ = treenode_init(NODE_STMT_COMPOUND, &@$);
   }
 | '{' statement_list '}'
   {
     $$ = treenode_init_compound ($2, &@$);
   }
 ;

statement_list
 : statement_list statement
   {
     $$ = $1;
     treenode_add_stmt ($$, $2);
     treenode_setLocation($$, &@$);
   }
 | statement
   {
     $$ = treenode_init(NODE_INTERNAL_USE, &@$);
     treenode_appendChild($$, $1);
   }
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
 | variable_declaration ';'
 | RETURN expression ';'
   {
     $$ = treenode_init(NODE_STMT_RETURN, &@$);
     treenode_appendChild($$, $2);
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
     treenode_appendChild($$, $3);
     treenode_appendChild($$, $5);
     treenode_appendChild($$, $7);
   }
 | IF '(' expression ')' statement
   {
     $$ = treenode_init(NODE_STMT_IF, &@$);
     treenode_appendChild($$, $3);
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
     treenode_appendChild($$, $3);
     treenode_appendChild($$, $5);
     treenode_appendChild($$, $7);
     treenode_appendChild($$, $9);
   }
 ;

maybe_expression
 : /* empty */
   {
     $$ = treenode_init(NODE_EXPR_NONE, &@$);
   }
 | expression
 ;

while_statement
 : WHILE '(' expression ')' statement
   {
     $$ = treenode_init(NODE_STMT_WHILE, &@$);
     treenode_appendChild($$, $3);
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
     treenode_appendChild($$, $5);
   }
 ;

assert_statement
 : ASSERT '(' expression ')' ';'
   {
     $$ = treenode_init(NODE_STMT_ASSERT, &@$);
     treenode_appendChild($$, $3);
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
     treenode_free ($5);
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
      treenode_appendChild($$, $1);
    }
  ;

syscall_parameter
  : expression
    {
      $$ = treenode_init(NODE_PUSH, &@$);
      treenode_appendChild($$, $1);
    }
  | READONLY expression
    {
      $$ = treenode_init(NODE_READONLY, &@$);
      treenode_appendChild($$, $2);
    }
  | SYSCALL_RETURN identifier
    {
      const struct YYLTYPE loc = treenode_location ($2);
      TreeNode * var = treenode_init(NODE_EXPR_RVARIABLE, &loc);
      treenode_appendChild(var, $2);
      $$ = treenode_init(NODE_SYSCALL_RETURN, &@$);
      treenode_appendChild($$, var);
    }
  | REF identifier
    {
      const struct YYLTYPE loc = treenode_location ($2);
      TreeNode * var = treenode_init(NODE_EXPR_RVARIABLE, &loc);
      treenode_appendChild(var, $2);
      $$ = treenode_init(NODE_PUSHREF, &@$);
      treenode_appendChild($$, var);
    }
  | CREF expression
    {
      $$ = treenode_init(NODE_PUSHCREF, &@$);
      treenode_appendChild($$, $2);
    }
  ;

 /*******************************************************************************
  * Indices: not strictly expressions as they only appear in specific context   *
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
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | expression
   {
     $$ = treenode_init(NODE_INDEX_INT, &@$);
     treenode_appendChild($$, $1);
   }
 ;

 /*******************************************************************************
  * Expressions:                                                                *
  *******************************************************************************/

lvalue
 : postfix_expression
   {
     YYLTYPE loc = @$;
     if (($$ = treenode_init_lvalue($1, &loc)) == NULL) {
        yyerror(&loc, yyscanner, $1, loc.filename, table, "Invalid lvalue.");
        YYERROR;
     }
     else {
        treenode_free ($1);
     }
   }
 ;

expression
 : assignment_expression
 ;

assignment_expression /* WARNING: RIGHT RECURSION */
 : lvalue '=' assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_ASSIGN, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | lvalue MUL_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_ASSIGN_MUL, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | lvalue DIV_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_ASSIGN_DIV, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | lvalue MOD_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_ASSIGN_MOD, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | lvalue ADD_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_ASSIGN_ADD, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | lvalue SUB_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_ASSIGN_SUB, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | lvalue AND_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_ASSIGN_AND, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | lvalue OR_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_ASSIGN_OR, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | lvalue XOR_ASSIGN assignment_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_ASSIGN_XOR, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | qualified_expression
 ;

qualified_type
 : public_sectype_specifier
 | primitive_datatype_specifier
 | dimtype_specifier
 | identifier
   {
     $$ = treenode_init(NODE_TYPEVAR, &@$);
     treenode_appendChild($$, $1);
   }
 ;

qualified_types
 : qualified_types qualified_type
   {
     $$ = $1;
     treenode_appendChild($$, $2);
   }
 | qualified_type
   {
     $$ = treenode_init(NODE_INTERNAL_USE, &@$);
     treenode_appendChild($$, $1);
   }
 ;

qualified_expression
 : qualified_expression TYPE_QUAL qualified_types
   {
     $$ = treenode_init(NODE_EXPR_TYPE_QUAL, &@$);
     treenode_appendChild($$, $1);
     treenode_moveChildren($3, $$);
     treenode_free($3);
   }
 | conditional_expression
 ;

conditional_expression
 : logical_or_expression '?' expression ':' expression
   {
     $$ = treenode_init(NODE_EXPR_TERNIF, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
     treenode_appendChild($$, $5);
   }
 | logical_or_expression
 ;

logical_or_expression
 : logical_or_expression LOR_OP logical_and_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_LOR, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | logical_and_expression
 ;

logical_and_expression
 : logical_and_expression LAND_OP bitwise_or_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_LAND, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | bitwise_or_expression
 ;

bitwise_or_expression
 : bitwise_or_expression '|' bitwise_xor_expression
   {
     $$ = treenode_init(NODE_EXPR_BITWISE_OR, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | bitwise_xor_expression
 ;

bitwise_xor_expression
 : bitwise_xor_expression '^' bitwise_and_expression
   {
     $$ = treenode_init(NODE_EXPR_BITWISE_XOR, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | bitwise_and_expression
 ;

bitwise_and_expression
 : bitwise_and_expression '&' equality_expression
   {
     $$ = treenode_init(NODE_EXPR_BITWISE_AND, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | equality_expression
 ;

equality_expression
 : equality_expression EQ_OP relational_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_EQ, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | equality_expression NE_OP relational_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_NE, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | relational_expression
 ;

relational_expression
 : relational_expression LE_OP shift_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_LE, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | relational_expression GE_OP shift_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_GE, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | relational_expression '<' shift_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_LT, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | relational_expression '>' shift_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_GT, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | shift_expression
 ;

shift_expression
 : shift_expression SHL_OP additive_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_SHL, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | shift_expression SHR_OP additive_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_SHR, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | additive_expression
 ;

additive_expression
 : additive_expression '+' multiplicative_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_ADD, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | additive_expression '-' multiplicative_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_SUB, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | multiplicative_expression
 ;

multiplicative_expression
 : multiplicative_expression '*' cast_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_MUL, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | multiplicative_expression '/' cast_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_DIV, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | multiplicative_expression '%' cast_expression
   {
     $$ = treenode_init(NODE_EXPR_BINARY_MOD, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | cast_expression
 ;

 /**
  * I would use the following rule, but bison seems to think that
  * this would cause a reduce/reduce conflict. I don't quite
  * understand why, but right now I'll be using this workaround.
  * I think it's a LALR(1) specific limitation.
  *
  *  cast_expression
  *   : '(' datatype_specifier ')' cast_expression
  *     { ... }
  *   | prefix_op
  *   ;
  */

cast_expression
 : '(' primitive_datatype_specifier ')' cast_expression
   {
     $$ = treenode_init(NODE_EXPR_CAST, &@$);
     treenode_appendChild($$, $2);
     treenode_appendChild($$, $4);
   }
 | '(' identifier ')' cast_expression
   {
     $$ = treenode_init(NODE_EXPR_CAST, &@$);
     TreeNode* temp = treenode_init_dataTypeVarF(&@$);
     treenode_appendChild(temp, $2);
     treenode_appendChild($$, temp);
     treenode_appendChild($$, $4);
   }
 | prefix_op
 ;

prefix_op
 : INC_OP lvalue
   {
     $$ = treenode_init(NODE_EXPR_PREFIX_INC, &@$);
     treenode_appendChild($$, $2);
   }
 | DEC_OP lvalue
   {
     $$ = treenode_init(NODE_EXPR_PREFIX_DEC, &@$);
     treenode_appendChild($$, $2);
   }
 | postfix_op
 ;

postfix_op
 : lvalue INC_OP
   {
     $$ = treenode_init(NODE_EXPR_POSTFIX_INC, &@$);
     treenode_appendChild($$, $1);
   }
 | lvalue DEC_OP
   {
     $$ = treenode_init(NODE_EXPR_POSTFIX_DEC, &@$);
     treenode_appendChild($$, $1);
   }
 | unary_expression
 ;

unary_expression
 : '-' cast_expression %prec UMINUS
   {
     $$ = treenode_init(NODE_EXPR_UMINUS, &@$);
     treenode_appendChild($$, $2);
   }
 | '!' cast_expression %prec UNEG
   {
     $$ = treenode_init(NODE_EXPR_UNEG, &@$);
     treenode_appendChild($$, $2);
   }
  | '~' cast_expression %prec UINV
   {
     $$ = treenode_init(NODE_EXPR_UINV, &@$);
     treenode_appendChild($$, $2);
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
 | STRLEN '(' expression ')'
   {
     $$ = treenode_init(NODE_EXPR_STRLEN, &@$);
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
 | SET_FPU_STATE '(' expression ')'
   {
     $$ = treenode_init(NODE_EXPR_SET_FPU_STATE, &@$);
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
 | postfix_expression '.' identifier
   {
     $$ = treenode_init(NODE_EXPR_SELECTION, &@$);
     treenode_appendChild($$, $1);
     treenode_appendChild($$, $3);
   }
 | primary_expression
 ;

primary_expression
 : '(' expression ')'
   {
     $$ = $2;
     treenode_setLocation($$, &@$);
   }
 | '{' expression_list '}'
   {
     $$ = treenode_init (NODE_EXPR_ARRAY_CONSTRUCTOR, &@$);
     treenode_moveChildren ($2, $$);
     treenode_free ($2);
   }
 | identifier
   {
     $$ = treenode_init(NODE_EXPR_RVARIABLE, &@$);
     treenode_appendChild($$, $1);
   }
 | GET_FPU_STATE
   {
     $$ = treenode_init(NODE_EXPR_GET_FPU_STATE, &@$);
   }
 | literal
 ;

int_literal
 : BIN_LITERAL { $$ = treenode_init_int($1, &@$); }
 | OCT_LITERAL { $$ = treenode_init_int($1, &@$); }
 | DEC_LITERAL { $$ = treenode_init_int($1, &@$); }
 | HEX_LITERAL { $$ = treenode_init_int($1, &@$); }
 ;

float_literal
 : FLOAT_LITERAL
   {
     $$ = treenode_init_float ($1, &@$);
   }
 ;

string_literal
 : string_literal string_part
   {
     $$ = $1;
     treenode_appendChild ($$, $2);
   }
 | string_part
   {
     $$ = treenode_init (NODE_LITE_STRING, &@$);
     treenode_appendChild ($$, $1);
   }
 ;

string_part
 : STR_IDENTIFIER
   {
     $$ = treenode_init_str_ident ($1, &@$);
   }
 | STR_FRAGMENT
   {
     $$ = treenode_init_str_fragment ($1, &@$);
   }
 ;

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
   }
 ;

%%

void yyerror(YYLTYPE *loc, yyscan_t yyscanner, TYPE_TREENODE *parseTree,
             const char * fileName, TYPE_STRINGTABLE table, const char *s)
{
    (void) yyscanner;
    (void) parseTree;
    (void) table;
    fprintf(stderr, "%s:(%zu,%zu)-(%zu,%zu): %s\n",
            fileName,
            loc->first_line, loc->first_column,
            loc->last_line, loc->last_column,
            s);
}

int sccparse(TYPE_STRINGTABLE table, const char * filename, TYPE_TREENODEMODULE *result) {
    assert(filename);
    yyscan_t scanner;
    int r;
    yylex_init_extra(table, &scanner);
    r = yyparse(scanner, result, filename, table);
    yylex_destroy(scanner);
    return r;
}

int sccparse_file(TYPE_STRINGTABLE table, const char * filename, FILE *input, TYPE_TREENODEMODULE *result) {
    assert(filename);
    yyscan_t scanner;
    int r;
    yylex_init_extra(table, &scanner);
    yyset_in(input, scanner);
    r = yyparse(scanner, result, filename, table);
    yylex_destroy(scanner);
    return r;
}

int sccparse_mem(TYPE_STRINGTABLE table, const char * filename, const void *buf, size_t size, TYPE_TREENODEMODULE *result) {
    assert(filename);
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

    yylex_init_extra(table, &scanner);
    yyset_in(memoryFile, scanner);
    r = yyparse(scanner, result, filename, table);
    yylex_destroy(scanner);
    fclose(memoryFile);
    return r;
}
