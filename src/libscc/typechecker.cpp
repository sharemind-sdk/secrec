#include "typechecker.h"

#include <boost/foreach.hpp>

#include "typechecker/templates.h"

namespace SecreC {

/*******************************************************************************
  TypeChecker
*******************************************************************************/

TypeChecker::TypeChecker (SymbolTable& st, CompileLog& log, Context& cxt)
    : m_st (&st)
    , m_log (log)
    , m_context (cxt)
    , m_instantiator (new TemplateInstantiator ())
{ }

TypeChecker::~TypeChecker () {
    delete m_instantiator;
}

bool TypeChecker::getForInstantiation (TreeNodeProcDef*& proc, SymbolTable*& st) {
    return m_instantiator->getForInstantiation (proc, st);
}

ICode::Status TreeNodeExprAssign::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprAssign* e) {
    typedef DataTypeVar DTV;
    typedef TypeNonVoid TNV;

    if (e->haveResultType()) {
        return ICode::OK;
    }

    // Get symbol for l-value:
    TreeNodeIdentifier* id = e->identifier ();
    SymbolSymbol* dest = getSymbol (id);
    if (dest == 0) return ICode::E_OTHER;

    // Calculate type of r-value:
    TreeNodeExpr* src = e->rightHandSide ();
    src->setContextType (dest->secrecType ()->secrecSecType ());
    ICode::Status s = visitExpr (src);
    if (s != ICode::OK) return s;

    // Get types for destination and source:
    SecreC::Type* destType = dest->secrecType();
    SecrecDimType destDim = destType->secrecDimType();
    assert(destType->isVoid() == false);
    SecreC::Type* srcType = src->resultType();
    SecrecDimType srcDim = srcType->secrecDimType();

    if (e->slice ()) {
        s = checkIndices (e->slice (), destDim);
        if (s != ICode::OK) {
            return ICode::E_TYPE;
        }
    }

    if (!(srcDim == destDim || srcDim == 0)) {
        m_log.fatal() << "Incompatible dimensionalities in assignemnt at "
                      << e->location() << ". "
                      << "Expected " << destDim << " got " << srcDim << ".";
        return ICode::E_TYPE;
    }

    // Check types:
    if (checkAndLogIfVoid (src)) return ICode::E_TYPE;
    if (!destType->canAssign (srcType)) {
        m_log.fatal() << "Invalid assignment from value of type " << *srcType
                      << " to variable of type " << *destType << " at "
                      << e->location() << ".";
        return ICode::E_TYPE;
    }

    // Add implicit classify node if needed:
    src = classifyIfNeeded (src);
    if (destType->secrecSecType () != src->resultType ()->secrecSecType ()) {
        return ICode::E_TYPE;
    }

    assert(dynamic_cast<TNV*>(destType) != 0);
    TNV* destTypeNV = static_cast<TNV*>(destType);
    assert(destTypeNV->dataType()->kind() == DataType::VAR);
    assert(dynamic_cast<DTV*>(destTypeNV->dataType()) != 0);
    DTV* ddtv = static_cast<DTV*>(destTypeNV->dataType());
    e->setResultType(TypeNonVoid::get (m_context, ddtv->dataType()));
    return ICode::OK;
}


ICode::Status TreeNodeExprCast::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprCast* root) {
    if (root->haveResultType ())
        return ICode::OK;

    if (! root->isSecTypeCast ()) {
        m_log.fatal () << "Only security type casts are allowed.";
        m_log.fatal () << "Error at " << root->location () << ".";
        return ICode::E_TYPE;
    }

    TreeNodeSecTypeF* secTypeNode = root->castType ();
    ICode::Status status = visit (secTypeNode);
    if (status != ICode::OK) return status;
    TreeNodeExpr* e = root->expression ();
    e->setContextType (secTypeNode->cachedType ());

    status = visitExpr (e);
    if (status != ICode::OK) {
        return status;
    }

    if (checkAndLogIfVoid (e)) {
        return ICode::E_TYPE;
    }

    TypeNonVoid* eType = static_cast<TypeNonVoid*>(e->resultType ());
    root->setResultType (eType);
    return ICode::OK;
}

ICode::Status TreeNodeExprIndex::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprIndex* root) {
    typedef TypeNonVoid TNV;

    if (root->haveResultType()) {
        return ICode::OK;
    }

    TreeNodeExpr* e = root->expression ();
    e->setContextType (root->contextSecType ());
    ICode::Status s = visitExpr (e);
    if (s != ICode::OK) return s;
    if (checkAndLogIfVoid (e)) return ICode::E_TYPE;
    TNV* eType = static_cast<TNV*>(e->resultType());
    SecrecDimType k = 0;
    SecrecDimType n = eType->secrecDimType();

    if (root->indices ()->children ().size () != n) {
        m_log.fatal() << "Incorrent number of indices at"
                      << e->location() << ".";
        return ICode::E_TYPE;
    }

    s = checkIndices (root->indices (), k);
    if (s != ICode::OK) {
        return s;
    }

    root->setResultType(TypeNonVoid::get (m_context,
        eType->secrecSecType(), eType->secrecDataType(), k));
    return ICode::OK;
}

ICode::Status TreeNodeExprSize::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprSize* root) {
    typedef TypeNonVoid TNV;

    if (! root->haveResultType()) {
        TreeNodeExpr* e = root->expression ();
        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) return s;
        if (checkAndLogIfVoid (e)) return ICode::E_TYPE;
        root->setResultType(TypeNonVoid::get (m_context, DATATYPE_INT));
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprShape::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprShape* root) {
    typedef TypeNonVoid TNV;

    if (! root->haveResultType ()) {
        TreeNodeExpr* e = root->expression ();
        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) return s;
        if (checkAndLogIfVoid (e)) return ICode::E_TYPE;
        root->setResultType(TypeNonVoid::get (getContext (), DATATYPE_INT, 1));
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprCat::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprCat* root) {
    typedef TypeNonVoid TNV;

    if (root->haveResultType()) {
        return ICode::OK;
    }

    // missing argument is interpreted as 0
    if (root->dimensionality () == 0) {
        TreeNode* e = new TreeNodeExprInt (0, root->location());
        root->appendChild(e);
    }

    TNV* eTypes[2];

    // check that first subexpressions 2 are arrays and of equal dimensionalities
    for (int i = 0; i < 2; ++ i) {
        assert (dynamic_cast<TreeNodeExpr*>(root->children().at(i)) != 0);
        TreeNodeExpr* e = static_cast<TreeNodeExpr*>(root->children().at(i));
        e->setContextType (root->contextSecType ());
        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) return s;
        if (checkAndLogIfVoid (e)) return ICode::E_TYPE;

        eTypes[i] = static_cast<TNV*>(e->resultType());
        if (eTypes[i]->isScalar()) {
            m_log.fatal() << "Concatenation of scalar values at "
                          << e->location() << ".";
            return ICode::E_TYPE;
        }
    }

    if (eTypes[0]->secrecDataType() != eTypes[1]->secrecDataType()) {
        m_log.fatal() << "Data types mismatch at "
                      << root->leftExpression ()->location() << " and "
                      << root->rightExpression ()->location() << ".";
        return ICode::E_TYPE;
    }

    if (eTypes[0]->secrecDimType() != eTypes[1]->secrecDimType()) {
        m_log.fatal() << "Dimensionalities mismatch at "
                      << root->leftExpression ()->location() << " and "
                      << root->rightExpression ()->location() << ".";
        return ICode::E_TYPE;
    }

    // type checker actually allows for aribtrary expression here
    // but right now parser expects integer literals, this is OK
    TreeNodeExpr* e3 = root->dimensionality ();
    ICode::Status s = visitExpr (e3);
    if (s != ICode::OK) return s;
    if (checkAndLogIfVoid (e3)) return ICode::E_TYPE;

    TNV* e3Type = static_cast<TNV*>(e3->resultType());
    if (!e3Type->isScalar() ||
        e3Type->secrecDataType() != DATATYPE_INT ||
        e3Type->secrecSecType()->isPrivate ())
    {
        m_log.fatal() << "Expected public scalar integer at "
                      << root->dimensionality ()->location()
                      << " got " << *e3Type << ".";
        return ICode::E_TYPE;
    }

    root->setResultType(TypeNonVoid::get (m_context,
        upperSecType(eTypes[0]->secrecSecType(), eTypes[1]->secrecSecType()),
        eTypes[0]->secrecDataType(),
        eTypes[0]->secrecDimType()));
    return ICode::OK;
}

ICode::Status TreeNodeExprReshape::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprReshape* root) {
    typedef TypeNonVoid TNV;

    if (root->haveResultType()) {
        return ICode::OK;
    }

    TreeNodeExpr* e = root->reshapee ();
    e->setContextType (root->contextSecType ());
    ICode::Status s = visitExpr (e);
    if (s != ICode::OK) return s;
    if (checkAndLogIfVoid (e)) return ICode::E_TYPE;

    TNV* eType = static_cast<TNV*>(e->resultType());
    SecrecDimType resDim = root->children ().size () - 1;
    for (unsigned i = 0; i < resDim; ++ i) {
        TreeNodeExpr* ei = root->dimensionality (i);
        ei->setContextType (PublicSecType::get (getContext ()));
        s = visitExpr (ei);
        if (s != ICode::OK) return s;

        if (checkAndLogIfVoid (ei)) return ICode::E_TYPE;
        TNV* eiType = static_cast<TNV*>(ei->resultType());
        if (eiType->secrecDataType() != DATATYPE_INT ||
            eiType->secrecSecType()->isPrivate () ||
            !eiType->isScalar())
        {
            m_log.fatal() << "Expected public integer scalar at "
                          << ei->location()
                          << " got " << eiType->toString() << ".";
            return ICode::E_TYPE;
        }
    }

    if (resDim == 0) {
        m_log.fatal() << "Conversion from non-scalar to scalar at "
                      << root->location() << ".";
        return ICode::E_TYPE;
    }

    root->setResultType (TypeNonVoid::get (getContext (),
        eType->secrecSecType(), eType->secrecDataType(), resDim));
    return ICode::OK;
}

ICode::Status TreeNodeExprBinary::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprBinary* root) {
    typedef TypeNonVoid TNV;

    if (root->haveResultType()) {
        return ICode::OK;
    }

    TreeNodeExpr *e1 = root->leftExpression ();
    TreeNodeExpr *e2 = root->rightExpression ();
    SecreC::Type *eType1, *eType2;

    if (root->contextSecType () != 0) {
        e1->setContextType (root->contextSecType ());
        e2->setContextType (root->contextSecType ());
    }

    {
        ICode::Status s = visitExpr (e1);
        if (s != ICode::OK) return s;
        eType1 = e1->resultType();

        s = visitExpr (e2);
        if (s != ICode::OK) return s;
        eType2 = e2->resultType();
    }

    SecurityType* s1 = eType1->secrecSecType();
    SecurityType* s2 = eType2->secrecSecType();
    SecurityType* s0 = upperSecType(s1, s2);

    if (root->contextSecType () == 0) {
        e1->setContextType (s0);
        e2->setContextType (s0);
    }

    if (!eType1->isVoid() && !eType2->isVoid()
#ifndef NDEBUG
        && (assert(dynamic_cast<const TNV*>(eType1) != 0), true)
        && (assert(dynamic_cast<const TNV*>(eType2) != 0), true)
#endif
        && static_cast<const TNV*>(eType1)->kind() == TNV::BASIC
        && static_cast<const TNV*>(eType2)->kind() == TNV::BASIC)
    {
        // Add implicit classify nodes if needed:
        e1 = classifyIfNeeded (e1);
        eType1 = e1->resultType();
        e2 = classifyIfNeeded (e2);
        eType2 = e2->resultType();

        SecrecDataType d1 = eType1->secrecDataType();
        SecrecDataType d2 = eType2->secrecDataType();
        SecrecDimType n1 = eType1->secrecDimType();
        SecrecDimType n2 = eType2->secrecDimType();

        if (n1 == 0 || n2 == 0 || n1 == n2) {
            SecrecDimType n0 = upperDimType(n1, n2);
            switch (root->type()) {
                case NODE_EXPR_BINARY_ADD:
                    if (d1 != d2) break;
                    if ((d1 & (DATATYPE_INT|DATATYPE_UINT|DATATYPE_STRING)) == 0x0)
                        break;

                    root->setResultType(TNV::get (m_context, s0, d1, n0));
                    return ICode::OK;
                case NODE_EXPR_BINARY_SUB:
                case NODE_EXPR_BINARY_MUL:
                case NODE_EXPR_BINARY_MOD:
                case NODE_EXPR_BINARY_DIV:
                    if (d1 != d2) break;
                    if ((d1 & (DATATYPE_INT|DATATYPE_UINT)) == 0x0) break;
                    root->setResultType(TNV::get (m_context, s0, d1, n0));
                    return ICode::OK;
                case NODE_EXPR_BINARY_EQ:
                case NODE_EXPR_BINARY_GE:
                case NODE_EXPR_BINARY_GT:
                case NODE_EXPR_BINARY_LE:
                case NODE_EXPR_BINARY_LT:
                case NODE_EXPR_BINARY_NE:
                    if (d1 != d2) break;
                    root->setResultType(TNV::get (m_context, s0, DATATYPE_BOOL, n0));
                    return ICode::OK;
                case NODE_EXPR_BINARY_LAND:
                case NODE_EXPR_BINARY_LOR:
                    if (d1 != DATATYPE_BOOL || d2 != DATATYPE_BOOL) break;
                    root->setResultType(TNV::get (m_context, s0, DATATYPE_BOOL, n0));
                    return ICode::OK;
                case NODE_EXPR_BINARY_MATRIXMUL:
                    m_log.fatal() << "Matrix multiplication not yet supported. At "
                                  << root->location() << ".";
                    return ICode::E_NOT_IMPLEMENTED;
                default:
                    assert(false);
                    return ICode::E_OTHER;
            }
        }
    }

    m_log.fatal() << "Invalid binary operation " << root->operatorString()
                  << " between operands of type " << *eType1 << " and " << *eType2
                  << " at " << root->location() << ".";
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprUnary::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprUnary* root) {
    typedef DataTypeBasic DTB;

    if (root->haveResultType()) {
        return ICode::OK;
    }

    assert(root->type() == NODE_EXPR_UMINUS ||
           root->type() == NODE_EXPR_UNEG);
    TreeNodeExpr *e = root->expression ();
    e->setContextType (root->contextSecType ());
    ICode::Status s = visitExpr (e);
    if (s != ICode::OK) return s;
    SecreC::Type* eType = e->resultType();

    /// \todo implement for matrixes also
    if (!eType->isVoid()
        && (assert(dynamic_cast<TypeNonVoid*>(eType) != 0), true)
        && static_cast<TypeNonVoid*>(eType)->kind() == TypeNonVoid::BASIC)
    {
        TypeNonVoid* et = static_cast<TypeNonVoid*>(eType);
        assert(dynamic_cast<DTB*>(et->dataType()) != 0);
        const DTB &bType = *static_cast<DTB*>(et->dataType());

        if (root->type() == NODE_EXPR_UNEG && bType.dataType() == DATATYPE_BOOL) {
            root->setResultType(et);
            return ICode::OK;
        }
        else
        if (root->type() == NODE_EXPR_UMINUS) {
            if (isSignedNumericDataType (bType.dataType())) {
                root->setResultType (et);
                return ICode::OK;
            }
        }
    }

    m_log.fatal() << "Invalid expression of type (" << *eType
                  << ") given to unary "
                  << (root->type() == NODE_EXPR_UNEG ? "negation" : "minus")
                  << " operator at " << root->location() << ".";
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprBool::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprBool* e) {
    if (! e->haveResultType ()) {
        e->setResultType (TypeNonVoid::get (m_context, DATATYPE_BOOL));
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprInt::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprInt* e) {
    if (! e->haveResultType()) {
        e->setResultType(TypeNonVoid::get (getContext (), DATATYPE_INT));
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprUInt::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprUInt* e) {
    if (! e->haveResultType()) {
        e->setResultType(TypeNonVoid::get (getContext (), DATATYPE_UINT));
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprClassify::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprClassify* root) {
    if (!root->haveResultType()) {
        if (root->contextSecType () == 0) {
            m_log.fatal () << "Unable to derive security type from context.";
            m_log.fatal () << "Error at " << root->location () << ".";
            return ICode::E_TYPE;
        }

        if (root->contextSecType ()->isPublic ()) {
            m_log.fatal () << "Classify to public security type.";
            m_log.fatal () << "Error at " << root->location () << ".";
            return ICode::E_TYPE;
        }

        TreeNodeExpr *e = root->expression ();
        e->setContextType (PublicSecType::get (getContext ()));
        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) return s;
        if (checkAndLogIfVoid(e)) return ICode::E_TYPE;
        assert (e->resultType()->secrecSecType()->isPublic ());
        root->setResultType(TypeNonVoid::get (getContext (),
                root->contextSecType (),
                e->resultType()->secrecDataType(),
                e->resultType()->secrecDimType()));
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprDeclassify::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprDeclassify* e) {
    if (e->haveResultType()) {
        return ICode::OK;
    }

    TreeNodeExpr* child = e->expression ();
    ICode::Status s = visitExpr (child);
    if (s != ICode::OK) {
        return s;
    }

    Type* childType = child->resultType ();
    if (!childType->isVoid ()) {
        if (childType->secrecSecType ()->isPrivate ()) {
            e->setResultType (TypeNonVoid::get (getContext (),
                childType->secrecDataType (),
                childType->secrecDimType ()));
            return ICode::OK;
        }
    }

    m_log.fatal() << "Argument of type " << *childType
                  << " passed to declassify operator at " << e->location() << ".";
    return ICode::E_TYPE;
}

ICode::Status TreeNodeExprRVariable::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprRVariable* e) {
    if (e->haveResultType()) {
        return ICode::OK;
    }

    typedef DataTypeVar DTV;
    typedef TypeNonVoid TNV;

    TreeNodeIdentifier *id = e->identifier ();
    SymbolSymbol *s = getSymbol (id);
    if (s == 0) {
        m_log.fatal () << "Undefined symbol at " << id->location () << ".";
        return ICode::E_OTHER;
    }

    assert(!s->secrecType()->isVoid());
    assert(dynamic_cast<TNV*>(s->secrecType()) != 0);
    TNV *type = static_cast<TNV*>(s->secrecType());
    assert(type->dataType()->kind() == DataType::VAR);
    assert(dynamic_cast<DTV*>(type->dataType()) != 0);
    e->setResultType(TypeNonVoid::get (m_context,
        static_cast<DTV*>(type->dataType())->dataType()));
    return ICode::OK;
}

ICode::Status TreeNodeExprString::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprString* e) {
    if (!e->haveResultType()) {
        e->setResultType (TypeNonVoid::get (m_context, DATATYPE_STRING));
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprTernary::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprTernary* e) {
    if (e->haveResultType()) {
        return ICode::OK;
    }

    TreeNodeExpr *e1 = e->conditional ();
    e1->setContextType (PublicSecType::get (getContext ()));
    ICode::Status s = visitExpr (e1);
    if (s != ICode::OK) return s;
    if (checkAndLogIfVoid (e1)) return ICode::E_TYPE;

    TreeNodeExpr *e2 = e->trueBranch ();
    s = visitExpr (e2);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e3 = e->falseBranch ();
    s = visitExpr (e3);
    if (s != ICode::OK) return s;

    SecreC::Type* eType1 = e1->resultType();
    SecreC::Type* eType2 = e2->resultType();
    SecreC::Type* eType3 = e3->resultType();

    assert(dynamic_cast<TypeNonVoid*>(eType1) != 0);
    TypeNonVoid* cType = static_cast<TypeNonVoid*>(eType1);

    // check if conditional expression is of public boolean type
    if (cType->kind() != TypeNonVoid::BASIC
        || cType->dataType()->kind() != DataType::BASIC
        || static_cast<DataTypeBasic*>(cType->dataType())->dataType()
            != DATATYPE_BOOL
        || static_cast<DataTypeBasic*>(cType->dataType())->secType()->isPrivate ())
    {
        m_log.fatal() << "Conditional subexpression at " << e1->location()
                      << " of ternary expression has to be public boolean, got "
                      << *cType << ".";
        return ICode::E_TYPE;
    }

    // check the types of results
    if (eType2->isVoid() != eType3->isVoid()) {
        m_log.fatal() << "Subxpression at " << e2->location() << " is "
                      << (eType2->isVoid() ? "" : "not")
                      << " void while subexpression at " << e3->location()
                      << (eType3->isVoid() ? " is" : " isn't");
        return ICode::E_TYPE;
    }

    if (!eType2->isVoid()) {
        if (eType2->secrecDataType() != eType3->secrecDataType()) {
            m_log.fatal() << "Results of ternary expression  at "
                          << e->location()
                          << " have to be of same data types, got "
                          << *eType2 << " and " << *eType3 << ".";
            return ICode::E_TYPE;
        }

        SecrecDimType n1 = eType1->secrecDimType();
        SecrecDimType n2 = eType2->secrecDimType();
        SecrecDimType n3 = eType3->secrecDimType();

        if (n2 != n3) {
            m_log.fatal() << "Brances of ternary expression at "
                          << e->location()
                          << " aren't of equal dimensionalities.";
            return ICode::E_TYPE;
        }

        if (n1 != 0 && n1 != n2) {
            m_log.fatal() << "Conditdional expression at "
                          << e1->location()
                          << " is non-scalar and doesn't match resulting subexpressions.";
            return ICode::E_TYPE;
        }
    }

    e2 = classifyIfNeeded (e2);
    e3 = classifyIfNeeded (e3);
    assert(e2->resultType() == e3->resultType());
    e->setResultType(e2->resultType());
    return ICode::OK;
}


ICode::Status TreeNodeExprDomainID::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprDomainID* e) {
    if (e->haveResultType ())
        return ICode::OK;

    if (e->securityType ()->isPublic ()) {
        m_log.fatal () << "Public security type does not have a domain ID.";
        m_log.fatal () << "Error at " << e->location () << ".";
        return ICode::E_TYPE;
    }

    ICode::Status status = visit (e->securityType ());
    if (status != ICode::OK) return status;
    e->setResultType (TypeNonVoid::get (getContext (), DATATYPE_UINT64));
    return ICode::OK;
}

ICode::Status TypeChecker::visit (TreeNodeSecTypeF* ty) {
    if (ty->cachedType () != 0)
        return ICode::OK;

    TreeNodeIdentifier* id = ty->identifier ();
    Symbol* s = findIdentifier (id);
    if (s == 0) {
        return ICode::E_TYPE;
    }

    if (s->symbolType () != Symbol::PDOMAIN) {
        m_log.fatal () << "Expecting privacy domain at " << ty->location () << ".";
        return ICode::E_TYPE;
    }

    assert (dynamic_cast<SymbolDomain*>(s) != 0);
    ty->setCachedType (static_cast<SymbolDomain*>(s)->securityType ());
    return ICode::OK;
}


ICode::Status TreeNodeExprPrefix::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprPrefix* root) {
    return checkPostfixPrefixIncDec (root, false, root->type() == NODE_EXPR_PREFIX_INC);
}

ICode::Status TreeNodeExprPostfix::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprPostfix* root) {
    return checkPostfixPrefixIncDec (root, true, root->type() == NODE_EXPR_POSTFIX_INC);
}

ICode::Status TypeChecker::checkPostfixPrefixIncDec (TreeNodeExpr* root,
                                                     bool isPrefix,
                                                     bool isInc)
{
    if (root->haveResultType()) {
        return ICode::OK;
    }

    assert(root->children().size() == 1);
    assert((root->children().at(0)->type() == NODE_LVALUE) != 0x0);
    TreeNode* lval = root->children().at(0);
    assert (lval->children ().size () <= 2);
    assert(dynamic_cast<TreeNodeIdentifier* >(lval->children ().at(0)) != 0);

    TreeNodeIdentifier *e =
            static_cast<TreeNodeIdentifier*>(lval->children ().at(0));
    SecreC::Type* eType = getSymbol (e)->secrecType ();
    unsigned destDim = eType->secrecDimType ();
    if (lval->children ().size () == 2) {
        ICode::Status s = checkIndices (lval->children ().at (1), destDim);
        if (s != ICode::OK) {
            return s;
        }
    }

    const char* m1 = isPrefix ? "Prefix " : "Postfix ";
    const char* m2 = isInc ? "increment" : "decrement";

    // check that argument is a variable
    if (e->type () == NODE_EXPR_RVARIABLE) {
        m_log.fatal() << m1 << m2
                      << " expects variable at " << root->location() << ".";
        return ICode::E_TYPE;
    }
    // increment or decrement of void
    if (eType->isVoid ()) {
        m_log.fatal() << m1 << m2 << " of void type expression at "
                      << root->location () << ".";
        return ICode::E_TYPE;
    }

    // check that we are operating on numeric types
    if (!isNumericDataType (eType->secrecDataType ())) {
        m_log.fatal() << m1 << m2
                      << " operator expects numeric type, given "
                      << *eType << " at " << root->location() << ".";
        return ICode::E_TYPE;
    }

    root->setResultType (TypeNonVoid::get (m_context,
        eType->secrecSecType (),
        eType->secrecDataType (),
        eType->secrecDimType ()));
    return ICode::OK;
}



ICode::Status TypeChecker::visit (TreeNodeStmtDecl* decl) {
    typedef DataTypeBasic DTB;
    typedef TreeNodeType TNT;
    typedef TypeNonVoid TNV;

    if (decl->m_type != 0) {
        return ICode::OK;
    }

    TreeNodeType *type = decl->varType ();
    ICode::Status s = visit (type);
    if (s != ICode::OK) return s;

    // First we create the new symbol, but we don't add it to the symbol table:
    assert (!type->secrecType()->isVoid());
    assert (dynamic_cast<TNV*>(type->secrecType()) != 0);
    TNV* justType = static_cast<TNV*>(type->secrecType());

    assert(justType->kind() == TNV::BASIC);
    assert(dynamic_cast<DTB*>(justType->dataType()) != 0);
    DTB* dataType = static_cast<DTB*>(justType->dataType());

    unsigned n = 0;
    if (decl->shape () != 0) {
        BOOST_FOREACH (TreeNode* node, decl->shape ()->children ()) {
            assert (dynamic_cast<TreeNodeExpr*>(node) != 0);
            TreeNodeExpr* e = static_cast<TreeNodeExpr*>(node);
            e->setContextType (PublicSecType::get (getContext ()));
            s = visitExpr (e);
            if (s != ICode::OK) return s;
            if (checkAndLogIfVoid (e)) return ICode::E_TYPE;
            if (   !e->resultType()->secrecDataType() == DATATYPE_INT
                || !e->resultType()->isScalar()
                ||  e->resultType()->secrecSecType()->isPrivate ())
            {
                m_log.fatal() << "Expecting public unsigned integer scalar at "
                              << e->location() << ".";
                return ICode::E_TYPE;
            }

            ++ n;
        }
    }

    if (n > 0 && n != justType->secrecDimType()) {
        m_log.fatal() << "Mismatching number of shape components in declaration at "
                      << decl->location() << ".";
        return ICode::E_TYPE;
    }

    decl->m_type = TypeNonVoid::get (getContext (),
        DataTypeVar::get (getContext (), dataType));

    if (decl->rightHandSide () != 0) {
        if (decl->procParam ()) {
            m_log.fatal () << "Declaration of procedure parameter may not have default value.";
            m_log.fatal () << "Error at " << decl->location () << ".";
            return ICode::E_TYPE;
        }

        TreeNodeExpr *e = decl->rightHandSide ();
        e->setContextType (dataType->secrecSecType ());
        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) return s;
        if (checkAndLogIfVoid (e)) return ICode::E_TYPE;
        if (! decl->m_type->canAssign (e->resultType ())) {
            m_log.fatal () << "Illegal assignment at " << decl->location () << ".";
            return ICode::E_TYPE;
        }

        e = classifyIfNeeded (e);
    }


    return ICode::OK;
}

ICode::Status TypeChecker::visit (TreeNodeType* _ty) {
    if (_ty->m_cachedType != 0) {
        return ICode::OK;
    }

    if (_ty->type () == NODE_TYPETYPE) {
        assert (dynamic_cast<TreeNodeTypeType*>(_ty) != 0);
        TreeNodeTypeType* tyNode = static_cast<TreeNodeTypeType*>(_ty);
        TreeNodeSecTypeF* secTyNode = tyNode->secType ();
        ICode::Status status = visit (secTyNode);
        if (status != ICode::OK) {
            return status;
        }

        SecurityType* secType = secTyNode->cachedType ();
        tyNode->m_cachedType = TypeNonVoid::get (m_context,
            secType, tyNode->dataType ()->dataType (), tyNode->dimType ()->dimType());
    }
    else {
        assert (dynamic_cast<TreeNodeTypeVoid*>(_ty) != 0);
        _ty->m_cachedType = TypeVoid::get (getContext ());
    }

    return ICode::OK;
}

ICode::Status TypeChecker::visit (TreeNodeStmtPrint* stmt) {
    TreeNodeExpr* e = stmt->expression ();
    e->setContextType (PublicSecType::get (getContext ()));
    ICode::Status s = visitExpr (e);
    if (s != ICode::OK) {
        return s;
    }

    if (e->resultType()->secrecDataType() != DATATYPE_STRING ||
        e->resultType()->secrecSecType()->isPrivate ()  ||
        !e->resultType()->isScalar())
    {
        m_log.fatal () << "Argument of \"print\" statement has to be public string scalar, got "
                       << *e->resultType() << " at " << stmt->location() << ".";
        return ICode::E_TYPE;
    }

    return ICode::OK;
}

ICode::Status TypeChecker::visit (TreeNodeStmtReturn* stmt) {
    SecreC::TypeNonVoid* procType = stmt->containingProcedure ()->procedureType ();
    TreeNodeExpr *e = stmt->expression ();
    if (e == 0) {
        if (procType->kind () == TypeNonVoid::PROCEDURE) {
            m_log.fatal() << "Cannot return from non-void function without value "
                             "at " << stmt->location() << ".";
            return ICode::E_TYPE;
        }
    }
    else {

        if (procType->kind () == TypeNonVoid::PROCEDUREVOID) {
            m_log.fatal () << "Cannot return value from void function at"
                           << stmt->location() << ".";
            return ICode::E_TYPE;
        }

        e->setContextType (stmt->returnSecurityType ());
        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) return s;
        if (!procType->canAssign(e->resultType ()) ||
             procType->secrecDimType () != e->resultType ()->secrecDimType ())
        {
            m_log.fatal () << "Cannot return value of type " << *e->resultType ()
                           << " from function with type "
                           << *procType << " at "
                           << stmt->location () << ".";
            return ICode::E_TYPE;
        }

        classifyIfNeeded (e);
    }

    return ICode::OK;
}

Symbol* TypeChecker::findIdentifier (TreeNodeIdentifier* id) const {
    Symbol* s = m_st->find (id->value ());
    if (s == 0) {
        m_log.fatal () << "Idenfier \"" << id->value () << "\" at " << id->location ()
                       << " not in scope.";
    }

    return s;
}

SymbolSymbol* TypeChecker::getSymbol (TreeNodeIdentifier *id) {
    Symbol *s = m_st->find (id->value ());
    if (s == 0) {
        m_log.fatal() << "Undeclared identifier \"" << id->value () << "\" at "
                      << id->location() << ".";
        return 0;
    }

    assert (s->symbolType() == Symbol::SYMBOL);
    assert (dynamic_cast<SymbolSymbol*>(s) != 0);
    return static_cast<SymbolSymbol*>(s);
}

TreeNodeExpr* TypeChecker::classifyIfNeeded (TreeNodeExpr* child) {
    if ((child->contextSecType () != 0) &&
        child->contextSecType ()->isPrivate () &&
        child->resultType()->secrecSecType()->isPublic ())
    {
        TreeNode* node = child->parent ();
        SecurityType* secTy (child->contextSecType ());
        TypeNonVoid* newTy = TypeNonVoid::get (getContext (), secTy,
            child->resultType ()->secrecDataType (),
            child->resultType ()->secrecDimType ());
        TreeNodeExprClassify *ec = new TreeNodeExprClassify (secTy, child->location());
        ec->appendChild (child);
        ec->resetParent (node);
        ec->setResultType (newTy);
        BOOST_FOREACH (TreeNode*& n, node->children ()) {
            if (n == child) {
                n = ec;
                break;
            }
        }

        // patch up context types just in case
        child->setContextType (PublicSecType::get (getContext ()));
        ec->setContextType (secTy);
        child = ec;
    }

    return child;
}

bool TypeChecker::checkAndLogIfVoid (TreeNodeExpr* e) {
    assert (e->haveResultType());
    if (e->resultType()->isVoid()) {
        m_log.fatal() << "Subexpression has type void at "
                      << e->location() << ".";
        return true;
    }

    return false;
}

ICode::Status TypeChecker::checkIndices (TreeNode* node, unsigned& destDim) {
    typedef TreeNode::ChildrenListConstIterator CLCI;
    assert (node->type() == NODE_SUBSCRIPT);
    destDim = 0;
    BOOST_FOREACH (TreeNode* tNode, node->children ()) {
        switch (tNode->type()) {
        case NODE_INDEX_SLICE:
            ++ destDim;
        case NODE_INDEX_INT:
            break;
        default:
            assert (false && "Reached an index that isn't int or a slice.");
            return ICode::E_TYPE;
        }

        BOOST_FOREACH (TreeNode* j, tNode->children ()) {
            if (j->type() == NODE_EXPR_NONE) {
                continue;
            }

            assert (dynamic_cast<TreeNodeExpr*>(j) != 0);
            TreeNodeExpr* e = static_cast<TreeNodeExpr*>(j);
            e->setContextType (PublicSecType::get (getContext ()));
            ICode::Status s = visitExpr (e);
            if (s != ICode::OK) {
                return s;
            }

            if (checkAndLogIfVoid (e)) {
                return ICode::E_TYPE;
            }

            TypeNonVoid* eTy = static_cast<TypeNonVoid*>(e->resultType());

            if (eTy->secrecSecType()->isPrivate () ||
                eTy->secrecDataType() != DATATYPE_INT ||
                eTy->secrecDimType() > 0)
            {
                m_log.fatal() << "Invalid type for index at " << e->location() << ". "
                              << "Expected public integer scalar, got " << *eTy << ".";
                return ICode::E_TYPE;
            }
        }
    }

    return ICode::OK;
}


} // namespace SecreC
