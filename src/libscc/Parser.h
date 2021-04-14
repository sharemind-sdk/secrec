/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#ifndef SECREC_PARSER_H
#define SECREC_PARSER_H

#ifdef __cplusplus
#include <sharemind/StringView.h>
#endif
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "ParserEnums.h"

#ifdef __cplusplus
namespace SecreC {
    class TreeNode;
    class TreeNodeModule;
    class StringTable;
} // namespace SecreC
#define TYPE_TREENODE        SecreC::TreeNode*
#define TYPE_TREENODEMODULE  SecreC::TreeNodeModule*
#define TYPE_STRINGTABLE     SecreC::StringTable*
#define TYPE_STRINGREF       sharemind::StringView const *
extern "C" {
#else /* #ifdef __cplusplus */
#define TYPE_TREENODE        void*
#define TYPE_TREENODEMODULE  void*
#define TYPE_STRINGTABLE     void*
#define TYPE_STRINGREF       const void*
#endif /* #ifdef __cplusplus */

union YYSTYPE;
struct YYLTYPE;

/**
    Parses SecreC from the standard input.
    \param result pointer where to store the resulting parse tree.
    \retval 0 Parsing was successful.
    \retval 1 Parsing failed due to syntax errors.
    \retval 2 Parsing failed due to memory exhaustion.
*/
extern int sccparse(TYPE_STRINGTABLE table, const char * filename, TYPE_TREENODEMODULE * result);

/**
    Parses SecreC from the given input.
    \param input pointer to the input stream.
    \param result pointer where to store the resulting parse tree.
    \retval 0 Parsing was successful.
    \retval 1 Parsing failed due to syntax errors.
    \retval 2 Parsing failed due to memory exhaustion.
*/
extern int sccparse_file(TYPE_STRINGTABLE table, const char * filename, FILE * input, TYPE_TREENODEMODULE * result);

/**
    Parses SecreC from the given memory region.
    \param buf pointer to the start of the region.
    \param size size of the region in bytes.
    \param result pointer where to store the resulting parse tree.
    \retval 0 Parsing was successful.
    \retval 1 Parsing failed due to syntax errors.
    \retval 2 Parsing failed due to memory exhaustion.
    \retval 3 Parsing failed because the input could not be read.
*/
extern int sccparse_mem(TYPE_STRINGTABLE table, const char * filename, const void * buf, size_t size, TYPE_TREENODEMODULE * result);

extern TYPE_STRINGREF add_string (TYPE_STRINGTABLE table, const char * str, size_t size);

union YYSTYPE {
    TYPE_TREENODE        treenode;
    void *               nothing;
    TYPE_STRINGREF       str;
    uint64_t             integer_literal;
    enum SecrecDataType  secrec_datatype;
    enum SecrecOperator  secrec_operator;
};
typedef union YYSTYPE YYSTYPE;

#define YYLTYPE YYLTYPE
typedef struct YYLTYPE {
    size_t first_line;
    size_t first_column;
    size_t last_line;
    size_t last_column;
    const char * filename;
} YYLTYPE;
#define YYLTYPE_IS_DECLARED 1

/* Define YYRHSLOC as a workaround for >=bison-2.6: */
#define YYRHSLOC(Rhs, K) ((Rhs)[K].yystate.yyloc)

#define YYLLOC_DEFAULT(Current, Rhs, N) \
    do { \
        if ((N)) { \
            (Current).first_line   = YYRHSLOC((Rhs), 1).first_line; \
            (Current).first_column = YYRHSLOC((Rhs), 1).first_column; \
            (Current).last_line    = YYRHSLOC((Rhs), (N)).last_line; \
            (Current).last_column  = YYRHSLOC((Rhs), (N)).last_column; \
        } else { \
            (Current).first_line = (Current).last_line = YYRHSLOC((Rhs), 0).last_line; \
            (Current).first_column = (Current).last_column = YYRHSLOC((Rhs), 0).last_column; \
        } \
        (Current).filename = fileName; \
    } while (0)

#ifdef __cplusplus
} /* extern "C" */
#endif /* #ifdef __cplusplus */

#endif /* #ifdef PARSER_H */

