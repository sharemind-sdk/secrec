/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_TREENODEFWD_H
#define SECREC_TREENODEFWD_H

namespace SecreC {

class TreeNode;
class TreeNodeAttribute;
class TreeNodeDataTypeConstF;
class TreeNodeDataTypeF;
class TreeNodeDataTypeTemplateF;
class TreeNodeDataTypeVarF;
class TreeNodeDimTypeConstF;
class TreeNodeDimTypeF;
class TreeNodeDimTypeVarF;
class TreeNodeDomain;
class TreeNodeExpr;
class TreeNodeExprArrayConstructor;
class TreeNodeExprAssign;
class TreeNodeExprBinary;
class TreeNodeExprBool;
class TreeNodeExprBytesFromString;
class TreeNodeExprCast;
class TreeNodeExprCat;
class TreeNodeExprClassify;
class TreeNodeExprDeclassify;
class TreeNodeExprDomainID;
class TreeNodeExprFloat;
class TreeNodeExprIndex;
class TreeNodeExprInt;
class TreeNodeExprNone;
class TreeNodeExprPostfix;
class TreeNodeExprPrefix;
class TreeNodeExprProcCall;
class TreeNodeExprQualified;
class TreeNodeExprRVariable;
class TreeNodeExprReshape;
class TreeNodeExprSelection;
class TreeNodeExprShape;
class TreeNodeExprSize;
class TreeNodeExprString;
class TreeNodeExprStringFromBytes;
class TreeNodeExprStrlen;
class TreeNodeExprTernary;
class TreeNodeExprToString;
class TreeNodeExprUnary;
class TreeNodeIdentifier;
class TreeNodeImport;
class TreeNodeKind;
class TreeNodeLIndex;
class TreeNodeLSelect;
class TreeNodeLValue;
class TreeNodeLVariable;
class TreeNodeModule;
class TreeNodeProcDef;
class TreeNodeProgram;
class TreeNodeQuantifier;
class TreeNodeQuantifierData;
class TreeNodeQuantifierDim;
class TreeNodeQuantifierDomain;
class TreeNodeSecTypeF;
class TreeNodeStmt;
class TreeNodeStmtAssert;
class TreeNodeStmtBreak;
class TreeNodeStmtCompound;
class TreeNodeStmtContinue;
class TreeNodeStmtDecl;
class TreeNodeStmtDoWhile;
class TreeNodeStmtExpr;
class TreeNodeStmtFor;
class TreeNodeStmtIf;
class TreeNodeStmtPrint;
class TreeNodeStmtReturn;
class TreeNodeStmtSyscall;
class TreeNodeStmtWhile;
class TreeNodeStringPart;
class TreeNodeStringPartFragment;
class TreeNodeStringPartIdentifier;
class TreeNodeStructDecl;
class TreeNodeSubscript;
class TreeNodeSyscallParam;
class TreeNodeTemplate;
class TreeNodeTemplateStruct;
class TreeNodeType;
class TreeNodeTypeArg;
class TreeNodeTypeArgDataTypeConst;
class TreeNodeTypeArgDimTypeConst;
class TreeNodeTypeArgPublic;
class TreeNodeTypeArgTemplate;
class TreeNodeTypeArgVar;
class TreeNodeTypeF;
class TreeNodeTypeVarF;
class TreeNodeVarInit;

template <class SubClass>
class TreeNodeSeqView;

} // namespace SecreC

#endif // SECREC_TYPECONTEXT_H
