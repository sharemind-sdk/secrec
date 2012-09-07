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
const YYLTYPE treenode_location(const TreeNode *node);
unsigned treenode_numChildren(const TreeNode *node);
TreeNode *treenode_childAt(const TreeNode *node, unsigned index);
void treenode_appendChild(TreeNode *parent, TreeNode *child);
void treenode_prependChild(TreeNode *parent, TreeNode *child);
void treenode_setLocation(TreeNode *node, YYLTYPE *loc);
void treenode_moveChildren(TreeNode* from, TreeNode* to);

TreeNode *treenode_init_bool(unsigned value, YYLTYPE *loc);
TreeNode *treenode_init_int(int value, YYLTYPE *loc);
TreeNode *treenode_init_uint(unsigned value, YYLTYPE *loc);
TreeNode *treenode_init_string(const char *value, YYLTYPE *loc);
TreeNode *treenode_init_float(const char *value, YYLTYPE *loc);
TreeNode *treenode_init_identifier(const char *value, YYLTYPE *loc);
TreeNode *treenode_init_publicSecTypeF(YYLTYPE *loc);
TreeNode *treenode_init_privateSecTypeF(YYLTYPE *loc);
TreeNode *treenode_init_dataTypeF(enum SecrecDataType dataType, YYLTYPE *loc);
TreeNode *treenode_init_dimTypeF(unsigned dimType, YYLTYPE *loc);
TreeNode *treenode_init_opdef(enum SecrecOperator op, YYLTYPE *loc);

#ifdef __cplusplus
} /* namespace SecreC */
#endif /* __cplusplus */

#endif /* SECREC_TREENODEC_H */
