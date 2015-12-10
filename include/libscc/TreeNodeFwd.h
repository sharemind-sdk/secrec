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
class TreeNodeOpDef;
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
