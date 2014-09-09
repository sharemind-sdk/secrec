/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_TREENODEC_H
#define SECREC_TREENODEC_H

#include "parser.h"

/**
 * C interface for treenode.h. Use by parser.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct TreeNode TreeNode;

TreeNode *treenode_init(enum SecrecTreeNodeType type, const YYLTYPE *loc);
void treenode_free(TreeNode *node);
enum SecrecTreeNodeType treenode_type(TreeNode *node);
YYLTYPE treenode_location(const TreeNode *node);
unsigned treenode_numChildren(const TreeNode *node);
TreeNode *treenode_childAt(const TreeNode *node, unsigned index);
void treenode_appendChild(TreeNode *parent, TreeNode *child);
void treenode_setLocation(TreeNode *node, YYLTYPE *loc);
void treenode_moveChildren(TreeNode* from, TreeNode* to);

TreeNode *treenode_init_bool(unsigned value, YYLTYPE *loc);
TreeNode *treenode_init_int(uint64_t value, YYLTYPE *loc);

TreeNode *treenode_init_str_fragment(TYPE_STRINGREF value, YYLTYPE *loc);
TreeNode *treenode_init_str_ident(TYPE_STRINGREF value, YYLTYPE *loc);
TreeNode *treenode_init_float(TYPE_STRINGREF value, YYLTYPE *loc);
TreeNode *treenode_init_identifier(TYPE_STRINGREF value, YYLTYPE *loc);

TreeNode *treenode_init_publicSecTypeF(YYLTYPE *loc);
TreeNode *treenode_init_privateSecTypeF(YYLTYPE *loc);
TreeNode *treenode_init_dataTypeVarF (YYLTYPE * loc);
TreeNode *treenode_init_dataTypeConstF(enum SecrecDataType dataType, YYLTYPE * loc);
TreeNode *treenode_init_dataTypeF(enum SecrecDataType dataType, YYLTYPE *loc);
TreeNode *treenode_init_dimTypeConstF(unsigned dimType, YYLTYPE *loc);
TreeNode *treenode_init_opdef(TYPE_STRINGTABLE table, enum SecrecOperator op, YYLTYPE *loc);
TreeNode *treenode_init_lvalue (TreeNode* node, YYLTYPE *loc);

#ifdef __cplusplus
} /* namespace SecreC */
#endif /* __cplusplus */

#endif /* SECREC_TREENODEC_H */
