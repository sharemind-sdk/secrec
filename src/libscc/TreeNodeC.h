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

#ifndef SECREC_TREENODEC_H
#define SECREC_TREENODEC_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif
#include "Parser.h"

/**
 * C interface for TreeNode.h. Use by parser.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct TreeNode TreeNode;

TreeNode *treenode_init(enum SecrecTreeNodeType type, const YYLTYPE *loc);
void treenode_free(TreeNode *node);
enum SecrecTreeNodeType treenode_type(TreeNode *node);
YYLTYPE treenode_location(const TreeNode *node);
#ifdef __cplusplus
std::
#endif
size_t treenode_numChildren(const TreeNode *node);
TreeNode *treenode_childAt(const TreeNode *node, unsigned index);
void treenode_appendChild(TreeNode *parent, TreeNode *child);
void treenode_setLocation(TreeNode *node, YYLTYPE *loc);
void treenode_moveChildren(TreeNode* from, TreeNode* to);

TreeNode *treenode_init_bool(unsigned value, YYLTYPE *loc);
TreeNode *treenode_init_int(TYPE_STRINGREF value, YYLTYPE *loc);

TreeNode *treenode_init_str_fragment(TYPE_STRINGREF value, YYLTYPE *loc);
TreeNode *treenode_init_str_ident(TYPE_STRINGREF value, YYLTYPE *loc);
TreeNode *treenode_init_float(TYPE_STRINGREF value, YYLTYPE *loc);
TreeNode *treenode_init_identifier(TYPE_STRINGREF value, YYLTYPE *loc);

TreeNode *treenode_init_publicSecTypeF(YYLTYPE *loc);
TreeNode *treenode_init_privateSecTypeF(YYLTYPE *loc);
TreeNode *treenode_init_dataTypeVarF (YYLTYPE * loc);
TreeNode *treenode_init_dataTypeConstF(enum SecrecDataType dataType, YYLTYPE * loc);
TreeNode *treenode_init_dataTypeF(enum SecrecDataType dataType, YYLTYPE *loc);
TreeNode *treenode_init_opdef(TYPE_STRINGTABLE table, enum SecrecOperator op, YYLTYPE *loc);
TreeNode *treenode_init_castdef(TYPE_STRINGTABLE table, YYLTYPE *loc);
TreeNode *treenode_init_lvalue (TreeNode* node, YYLTYPE *loc);

TreeNode *treenode_init_typeArgDataTypeConst (enum SecrecDataType dataType, YYLTYPE * loc);

TreeNode *treenode_init_dataTypeDecl (TYPE_STRINGREF name, YYLTYPE *loc);
TreeNode *treenode_init_dataTypeDeclParamPublic (enum SecrecDataType dataType, YYLTYPE * loc);
TreeNode *treenode_init_dataTypeDeclParamSize (uint64_t size, YYLTYPE * loc);

TYPE_STRINGREF secrec_fund_datatype_to_string (TYPE_STRINGTABLE table, enum SecrecDataType ty);

#ifdef __cplusplus
} /* extern "C" { */
#endif /* __cplusplus */

#endif /* SECREC_TREENODEC_H */
