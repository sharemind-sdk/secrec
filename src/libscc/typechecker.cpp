#include "typechecker.h"

#include <boost/foreach.hpp>
#include <boost/range.hpp>

#include "typechecker/templates.h"

namespace {
using namespace SecreC;

void setContextType (TypeContext* e, TypeNonVoid* ty) {
    assert (e != 0 && ty != 0);
    e->setContextDataType (ty->secrecDataType ());
    e->setContextSecType (ty->secrecSecType ());
    e->setContextDimType (ty->secrecDimType ());
}

void setContextType (TypeContext* to, TypeContext* from) {
    assert (to != 0 && from != 0);
    to->setContextDataType (from->contextDataType ());
    to->setContextDimType (from->contextDimType ());
    to->setContextSecType (from->contextSecType ());
}

void setContextPublicIntScalar (TypeContext* e, Context& cxt) {
    assert (e != 0);
    e->setContextDataType (DATATYPE_INT64);
    e->setContextDimType (0);
    e->setContextSecType (PublicSecType::get (cxt));
}

bool isPublicIntScalar (SecreC::Type* ty) {
    if (! ty->isVoid ()) {
        return ty->secrecDataType () == DATATYPE_INT64 &&
               ty->secrecSecType ()->isPublic () &&
               ty->secrecDimType () == 0;
    }

    return false;
}

// declassify is function
SecrecDataType dtypeDeclassify (SecrecDataType dtype) {
    switch (dtype) {
    case DATATYPE_XOR_UINT8: return DATATYPE_UINT8;
    case DATATYPE_XOR_UINT16: return DATATYPE_UINT16;
    case DATATYPE_XOR_UINT32: return DATATYPE_UINT32;
    case DATATYPE_XOR_UINT64: return DATATYPE_UINT64;
    default: return dtype;
    }
}

} // anonymous namespace

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

bool TypeChecker::getForInstantiation (InstanceInfo& info) {
    return m_instantiator->getForInstantiation (info);
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
    SecreC::Type* destType = dest->secrecType();
    SecrecDimType destDim = destType->secrecDimType();
    assert(destType->isVoid() == false);

    // Check the slice:
    if (e->slice ()) {
        ICode::Status s = checkIndices (e->slice (), destDim);
        if (s != ICode::OK) {
            return s;
        }
    }


    // Calculate type of r-value:
    TreeNodeExpr* src = e->rightHandSide ();
    src->setContextDataType (destType->secrecDataType ());
    if (destType->secrecSecType ()->isPublic ())
        src->setContextSecType (destType->secrecSecType ());
    if (destDim == 0)
        src->setContextDimType (0);
    ICode::Status s = visitExpr (src);
    if (s != ICode::OK) return s;
    SecreC::Type* srcType = src->resultType();
    SecrecDimType srcDim = srcType->secrecDimType();

    // Check if destination and expected types match!
    if (!(srcDim == destDim || srcDim == 0)) {
        m_log.fatal() << "Incompatible dimensionalities in assignemnt at "
                      << e->location() << ". "
                      << "Expected " << destDim << " got " << srcDim << ".";
        return ICode::E_TYPE;
    }

    // Check types:
    if (checkAndLogIfVoid (src)) {
        return ICode::E_TYPE;
    }

    if (! latticeDataTypeLEQ (srcType->secrecDataType (), destType->secrecDataType ()) ||
        ! latticeSecTypeLEQ (srcType->secrecSecType (), destType->secrecSecType ())) {
        m_log.fatal() << "Invalid assignment from value of type " << *srcType
                      << " to variable of type " << *destType << " at "
                      << e->location() << ".";
        return ICode::E_TYPE;
    }

    // Add implicit classify node if needed:
    src = classifyIfNeeded (src, destType->secrecSecType ());
    srcType = src->resultType();
    if (destType->secrecSecType () != srcType->secrecSecType ()) {
        m_log.fatal () << "Internal compile error: security types don't match after classification.";
        m_log.fatal () << "Error at " << e->location () << ".";
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

    TreeNodeExpr* subExpr = root->expression ();
    SecrecDataType resultingDType = root->dataType ()->dataType ();
    subExpr->setContextDataType (resultingDType);
    ICode::Status status = visitExpr (subExpr);
    if (status != ICode::OK) {
        return status;
    }

    SecreC::Type* ty = subExpr->resultType ();
    SecrecDataType givenDType = ty->secrecDataType ();
    if (! latticeExplicitLEQ (givenDType, resultingDType)) {
        m_log.fatal () << "Unable to perform cast at "
                       << root->location () << ".";
        return ICode::E_TYPE;
    }


    root->setResultType (
        TypeNonVoid::get (getContext (),
            ty->secrecSecType (),
            resultingDType,
            ty->secrecDimType ()));

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
    e->setContextSecType (root->contextSecType ());
    e->setContextDataType (root->contextDataType ());
    e->setContextDimType (root->indices ()->children ().size ());
    ICode::Status s = visitExpr (e);
    if (s != ICode::OK) return s;
    if (checkAndLogIfVoid (e)) return ICode::E_TYPE;
    TNV* eType = static_cast<TNV*>(e->resultType());
    SecrecDimType k = 0;
    SecrecDimType n = eType->secrecDimType();

    if (root->indices ()->children ().size () != static_cast<size_t>(n)) {
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
        root->setResultType(TypeNonVoid::getIndexType (getContext ()));
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
        root->setResultType(TypeNonVoid::get (getContext (), DATATYPE_INT64, 1));
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
        e->setContextSecType (root->contextSecType ());
        e->setContextDataType (root->contextDataType ());
        if (root->contextDimType () >= 0) {
            e->setContextDimType (root->contextDimType () - 1);
        }

        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) return s;
        if (checkAndLogIfVoid (e)) return ICode::E_TYPE;

        e = classifyIfNeeded (e, root->contextSecType ());
        eTypes[i] = static_cast<TNV*>(e->resultType());
        if (eTypes[i]->isScalar()) {
            m_log.fatal() << "Concatenation of scalar values at "
                          << e->location() << ".";
            return ICode::E_TYPE;
        }
    }


    SecurityType* resSecType = upperSecType (eTypes[0]->secrecSecType(),
                                             eTypes[1]->secrecSecType());
    if (resSecType == 0) {
        m_log.fatal () << "Concatenating arrays of incompatable security types.";
        m_log.fatal () << "Error at " << root->location () << ".";
        return ICode::E_TYPE;
    }

    if (eTypes[0]->secrecDataType () != eTypes[1]->secrecDataType ()) {
        m_log.fatal() << "Data type mismatch at "
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
    setContextPublicIntScalar (e3, getContext ());
    ICode::Status s = visitExpr (e3);
    if (s != ICode::OK) return s;
    if (checkAndLogIfVoid (e3)) return ICode::E_TYPE;

    TNV* e3Type = static_cast<TNV*>(e3->resultType());
    if (! isPublicIntScalar (e3Type)) {
        m_log.fatal() << "Expected public scalar integer at "
                      << root->dimensionality ()->location()
                      << " got " << *e3Type << ".";
        return ICode::E_TYPE;
    }


    root->setResultType(TypeNonVoid::get (m_context,
        resSecType,
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

    SecrecDimType resDim = boost::size (root->dimensions ());
    TreeNodeExpr* e = root->reshapee ();
    e->setContextSecType (root->contextSecType ());
    e->setContextDataType (root->contextDataType ());
    e->setContextDimType (resDim);
    ICode::Status s = visitExpr (e);
    if (s != ICode::OK) return s;
    if (checkAndLogIfVoid (e)) return ICode::E_TYPE;

    TNV* eType = static_cast<TNV*>(e->resultType());
    BOOST_FOREACH (TreeNode* _dim, root->dimensions ()) {
        assert (dynamic_cast<TreeNodeExpr*>(_dim));
        TreeNodeExpr* dim = static_cast<TreeNodeExpr*>(_dim);
        setContextPublicIntScalar (dim, getContext ());
        s = visitExpr (dim);
        if (s != ICode::OK) return s;

        if (checkAndLogIfVoid (dim)) return ICode::E_TYPE;
        TNV* dimType = static_cast<TNV*>(dim->resultType());
        if (! isPublicIntScalar (dimType)) {
            m_log.fatal() << "Expected public integer scalar at "
                          << dim->location()
                          << " got " << dimType->toString() << ".";
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

ICode::Status TreeNodeExprToString::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprToString* root) {
    if (root->haveResultType()) {
        return ICode::OK;
    }

    TreeNodeExpr* e = root->expression ();
    e->setContextSecType (PublicSecType::get (getContext ()));
    e->setContextDimType (0);
    ICode::Status s = visitExpr (e);
    if (s != ICode::OK) return s;
    if (checkAndLogIfVoid (e)) return ICode::E_TYPE;
    TypeNonVoid* tnv = static_cast<TypeNonVoid*>(e->resultType ());
    if (tnv->secrecDimType () != 0
            || tnv->secrecSecType ()->isPrivate ()
            || tnv->secrecDataType () == DATATYPE_STRING) {
        m_log.fatal () << "Invalid argument passed to \"tostring\" expression.";
        m_log.fatal () << "Error at " << root->location () << ".";
        return ICode::E_TYPE;
    }

    root->setResultType (TypeNonVoid::get (getContext (), DATATYPE_STRING));

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
    SecreC::TypeNonVoid *eType1 = 0, *eType2 = 0;

    //set context data type
    switch (root->type()) {
    case NODE_EXPR_BINARY_ADD:
    case NODE_EXPR_BINARY_SUB:
    case NODE_EXPR_BINARY_MUL:
    case NODE_EXPR_BINARY_MOD:
    case NODE_EXPR_BINARY_DIV:
    case NODE_EXPR_BINARY_LAND:
    case NODE_EXPR_BINARY_LOR:
        setContextType (e1, root);
        setContextType (e2, root);
    default:
        /* relational and logical operators are polymorphic */
        break;
    }

    e1->setContextSecType (root->contextSecType ());
    e2->setContextSecType (root->contextSecType ());

    {
        ICode::Status s = visitExpr (e1);
        if (s != ICode::OK) return s;
        if (checkAndLogIfVoid (e1)) return ICode::E_TYPE;
        assert (dynamic_cast<TNV*>(e1->resultType()) != 0);
        eType1 = static_cast<TNV*>(e1->resultType());

        s = visitExpr (e2);
        if (s != ICode::OK) return s;
        if (checkAndLogIfVoid (e2)) return ICode::E_TYPE;
        assert(dynamic_cast<TNV*>(e2->resultType ()) != 0);
        eType2 = static_cast<TNV*>(e2->resultType ());
    }

    if (   static_cast<const TNV*>(eType1)->kind() == TNV::BASIC
        && static_cast<const TNV*>(eType2)->kind() == TNV::BASIC)
    {
        { // check if operator is overloaded
            SymbolProcedure* match = 0;
            std::vector<DataType*> argumentDataTypes;
            DataTypeProcedureVoid* argTypes = 0;
            argumentDataTypes.push_back (eType1->dataType ());
            argumentDataTypes.push_back (eType2->dataType ());
            argTypes = DataTypeProcedureVoid::get (getContext (),
                                                   argumentDataTypes);
            ICode::Status s = findBestMatchingProc (match, root->operatorName (),
                                                    *root, argTypes);
            if (s != ICode::OK) {
                m_log.fatal () << "Error at " << root->location () << ".";
                return s;
            }

            if (match != 0) { // overloaded operator
                SecreC::Type* resultType = 0;
                s = checkProcCall (match, argTypes, resultType);
                if (s != ICode::OK) {
                    return s;
                }

                assert (resultType != 0);
                root->setResultType (resultType);
                root->setProcSymbol (match);
                return ICode::OK;
            }
        }


        SecurityType* s1 = eType1->secrecSecType();
        SecurityType* s2 = eType2->secrecSecType();
        SecurityType* s0 = upperSecType(s1, s2);

        // No really needed, but oh well.
        e1->setContextSecType (s0);
        e2->setContextSecType (s0);

        // Add implicit classify nodes if needed:
        e1 = classifyIfNeeded (e1, s0);
        eType1 = static_cast<TNV*>(e1->resultType());
        e2 = classifyIfNeeded (e2, s0);
        eType2 = static_cast<TNV*>(e2->resultType());

        SecrecDataType d1 = eType1->secrecDataType();
        SecrecDataType d2 = eType2->secrecDataType();
        SecrecDimType n1 = eType1->secrecDimType();
        SecrecDimType n2 = eType2->secrecDimType();

        if (n1 == 0 || n2 == 0 || n1 == n2) {
            SecrecDimType n0 = upperDimType(n1, n2);
            switch (root->type()) {
                case NODE_EXPR_BINARY_ADD:
                    if (d1 != d2) break;
                    if (! isNumericDataType (d1) && d1 != DATATYPE_STRING)
                        break;

                    root->setResultType(TNV::get (m_context, s0, d1, n0));
                    return ICode::OK;
                case NODE_EXPR_BINARY_SUB:
                case NODE_EXPR_BINARY_MUL:
                case NODE_EXPR_BINARY_MOD:
                case NODE_EXPR_BINARY_DIV:
                    if (d1 != d2) break;
                    if (! isNumericDataType (d1)) break;
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
    setContextType (e, root);
    ICode::Status s = visitExpr (e);
    if (s != ICode::OK) return s;
    SecreC::Type* eType = e->resultType();

    if (!eType->isVoid()
        && (assert(dynamic_cast<TypeNonVoid*>(eType) != 0), true)
        && static_cast<TypeNonVoid*>(eType)->kind() == TypeNonVoid::BASIC)
    {
        TypeNonVoid* et = static_cast<TypeNonVoid*> (eType);
        assert (dynamic_cast<DTB*>(et->dataType()) != 0);
        DTB* bType = static_cast<DTB*>(et->dataType());

        { // check if operator is overloaded
            SymbolProcedure* match = 0;
            std::vector<DataType*> argumentDataTypes;
            DataTypeProcedureVoid* argTypes = 0;
            argumentDataTypes.push_back (bType);
            argTypes = DataTypeProcedureVoid::get (getContext (),
                                                   argumentDataTypes);
            ICode::Status s = findBestMatchingProc (match, root->operatorName (),
                                                    *root, argTypes);
            if (s != ICode::OK) {
                m_log.fatal () << "Error at " << root->location () << ".";
                return s;
            }

            if (match != 0) { // overloaded operator
                SecreC::Type* resultType = 0;
                s = checkProcCall (match, argTypes, resultType);
                if (s != ICode::OK) {
                    return s;
                }

                assert (resultType != 0);
                root->setResultType (resultType);
                root->setProcSymbol (match);
                return ICode::OK;
            }
        }

        if (root->type() == NODE_EXPR_UNEG && bType->dataType() == DATATYPE_BOOL) {
            root->setResultType(et);
            return ICode::OK;
        }
        else
        if (root->type() == NODE_EXPR_UMINUS) {
            if (isNumericDataType (bType->dataType())) {
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
        SecrecDataType dtype = DATATYPE_UNDEFINED;
        switch (e->contextDataType ()) {
        case DATATYPE_UNDEFINED:
            dtype = DATATYPE_INT64;
            break;
        case DATATYPE_UNIT:
        case DATATYPE_BOOL:
        case DATATYPE_STRING:
            m_log.fatal () << "Expected numeric type.";
            m_log.fatal () << "Error at " << e->location () << ".";
            return ICode::E_TYPE;
        default:
            dtype = dtypeDeclassify (e->contextDataType ());
            break;
        }

        e->setResultType (TypeNonVoid::get (getContext (), dtype));
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
        e->setContextSecType (PublicSecType::get (getContext ()));
        e->setContextDimType (root->contextDimType ());
        if (root->haveContextDataType ()) {
            e->setContextDataType (dtypeDeclassify (root->contextDataType ()));
        }

        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) return s;
        if (checkAndLogIfVoid(e)) return ICode::E_TYPE;
        assert (e->resultType()->secrecSecType()->isPublic ());

        switch (e->resultType()->secrecDataType ()) {
        case DATATYPE_XOR_UINT8:
        case DATATYPE_XOR_UINT16:
        case DATATYPE_XOR_UINT32:
        case DATATYPE_XOR_UINT64:
            m_log.fatal () << "Inferred XOR public type!";
            m_log.fatal () << "Error at " << e->location () << ".";
            return ICode::E_TYPE;
        default:
            break;
        }

        SecrecDataType destDType = e->resultType()->secrecDataType ();
        if (root->haveContextDataType ()) {
            destDType = root->contextDataType ();
        }

        root->setResultType(TypeNonVoid::get (getContext (),
                root->contextSecType (),
                destDType,
                e->resultType()->secrecDimType()));
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprDeclassify::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprDeclassify* root) {
    if (root->haveResultType()) {
        return ICode::OK;
    }

    TreeNodeExpr* e = root->expression ();
//    e->setContextDataType (root->contextDataType ());
    e->setContextDimType (root->contextDimType ());
    ICode::Status s = visitExpr (e);
    if (s != ICode::OK) {
        return s;
    }

    Type* childType = e->resultType ();
    if (checkAndLogIfVoid (e)) return ICode::E_TYPE;
    if (childType->secrecSecType ()->isPublic ()) {
        m_log.fatal() << "Argument of type " << *childType
                      << " passed to declassify operator at "
                      << root->location() << ".";
        return ICode::E_TYPE;
    }

    root->setResultType (TypeNonVoid::get (getContext (),
        dtypeDeclassify (childType->secrecDataType ()),
        childType->secrecDimType ()));
    return ICode::OK;
}

ICode::Status TreeNodeExprRVariable::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprRVariable* e) {
    if (e->haveResultType()) {
        return ICode::OK;
    }

    TreeNodeIdentifier *id = e->identifier ();
    SymbolSymbol *s = getSymbol (id);
    if (s == 0) {
        m_log.fatal () << "Undefined symbol at " << id->location () << ".";
        return ICode::E_OTHER;
    }

    assert(!s->secrecType()->isVoid());
    assert(dynamic_cast<TypeNonVoid*>(s->secrecType()) != 0);
    TypeNonVoid *type = static_cast<TypeNonVoid*>(s->secrecType());
    assert(type->dataType()->kind() == DataType::VAR);
    assert(dynamic_cast<DataTypeVar*>(type->dataType()) != 0);
    e->setResultType(TypeNonVoid::get (m_context,
        static_cast<DataTypeVar*>(type->dataType())->dataType()));
    return ICode::OK;
}

ICode::Status TreeNodeExprString::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprString* e) {
    if (!e->haveResultType()) {
        if (e->haveContextDataType () && e->contextDataType () != DATATYPE_STRING) {
            m_log.fatal () << "Expecting string, got " << e->contextDataType () << ".";
            m_log.fatal () << "Error at " << e->location () << ".";
            return ICode::E_TYPE;
        }

        e->setResultType (TypeNonVoid::get (m_context, DATATYPE_STRING));
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprFloat::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprFloat* e) {
    if (!e->haveResultType()) {
        if (e->haveContextDataType ()) {
            SecrecDataType dType = e->contextDataType ();
            switch (dType) {
            case DATATYPE_FLOAT32:
            case DATATYPE_FLOAT64:
                e->setResultType (TypeNonVoid::get (m_context, dType));
                return ICode::OK;
            default:
                m_log.fatal () << "Expecting floating point, got " << dType << ".";
                m_log.fatal () << "Error at " << e->location () << ".";
                return ICode::E_TYPE;
            }
        }

        // TODO: would it be rational to figure the type out based on
        e->setResultType (TypeNonVoid::get (m_context, DATATYPE_FLOAT32));
    }

    return ICode::OK;
}

ICode::Status TreeNodeExprTernary::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprTernary* root) {
    if (root->haveResultType()) {
        return ICode::OK;
    }

    TreeNodeExpr *e1 = root->conditional ();
    e1->setContextSecType (PublicSecType::get (getContext ()));
    e1->setContextDataType (DATATYPE_BOOL);
    ICode::Status s = visitExpr (e1);
    if (s != ICode::OK) return s;
    if (checkAndLogIfVoid (e1)) return ICode::E_TYPE;

    TreeNodeExpr *e2 = root->trueBranch ();
    setContextType (e2, root);
    s = visitExpr (e2);
    if (s != ICode::OK) return s;

    TreeNodeExpr *e3 = root->falseBranch ();
    setContextType (e3, root);
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
        SecurityType* s0 = upperSecType (e2->resultType ()->secrecSecType (), e3->resultType ()->secrecSecType ());
        if (s0 == 0) {
            m_log.fatal () << "Incompatible security types in ternary expression at " << e2->location () << " and " << e3->location () << ".";
            m_log.fatal () << "Unable to match " << e2->resultType ()->secrecSecType ()->toString () << " with "
                           << e3->resultType ()->secrecSecType ()->toString () << ".";
            return ICode::E_TYPE;
        }

        e2 = classifyIfNeeded (e2, s0);
        e3 = classifyIfNeeded (e3, s0);

        if (eType2->secrecDataType() != eType3->secrecDataType()) {
            m_log.fatal() << "Results of ternary expression  at "
                          << root->location()
                          << " have to be of same data types, got "
                          << *eType2 << " and " << *eType3 << ".";
            return ICode::E_TYPE;
        }

        SecrecDimType n1 = eType1->secrecDimType();
        SecrecDimType n2 = eType2->secrecDimType();
        SecrecDimType n3 = eType3->secrecDimType();

        if (n2 != n3) {
            m_log.fatal() << "Brances of ternary expression at "
                          << root->location()
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

    assert(e2->resultType() == e3->resultType());
    root->setResultType(e2->resultType());
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

ICode::Status TreeNodeExprQualified::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprQualified* e) {
    if (e->haveResultType ())
        return ICode::OK;

    ICode::Status status = ICode::OK;
    TreeNodeExpr* subExpr = e->expression ();
    BOOST_FOREACH (TreeNode* _node, e->types ()) {
        switch (_node->type ()) {
        case NODE_SECTYPE_F: {
            TreeNodeSecTypeF* secTy = static_cast<TreeNodeSecTypeF*>(_node);
            status = visit (secTy);
            if (status != ICode::OK) return status;
            subExpr->setContextSecType (secTy->cachedType ());
            }
            break;
        case NODE_DATATYPE_F:
            subExpr->setContextDataType (
                static_cast<TreeNodeDataTypeF*>(_node)->dataType ());
            break;
        case NODE_DIMTYPE_F:
            subExpr->setContextDimType (
                static_cast<TreeNodeDataTypeF*>(_node)->dataType ());
            break;
        default:
            assert (false && "ICE: expression qualified over non-type!");
            break;
        }
    }

    status = visitExpr (subExpr);
    if (status != ICode::OK) return status;

    /* Check that the actual type matches the qualified type: */

    if (subExpr->haveContextDataType ()) {
        if (subExpr->contextDataType () !=
                subExpr->resultType ()->secrecDataType ()) {
            m_log.fatal () << "Data type of the expression at "
                           << subExpr->location ()
                           << " does not match the qualified type.";
            return ICode::E_TYPE;
        }
    }

    if (subExpr->haveContextDimType ()) {
        if (subExpr->contextDimType () !=
                subExpr->resultType ()->secrecDimType ()) {
            m_log.fatal () << "Dimensionality type of the expression at "
                           << subExpr->location ()
                           << " does not match the qualified type.";
            return ICode::E_TYPE;
        }
    }

    if (subExpr->haveContextSecType ()) {
        if (subExpr->contextSecType () !=
                subExpr->resultType ()->secrecSecType ()) {
            m_log.fatal () << "Security type of the expression at "
                           << subExpr->location ()
                           << " does not match the qualified type.";
            return ICode::E_TYPE;
        }
    }

    e->setResultType (subExpr->resultType ());
    return ICode::OK;
}

ICode::Status TreeNodeExprStringFromBytes::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprStringFromBytes* e) {
    if (e->haveResultType ())
        return ICode::OK;

    TreeNodeExpr* subExpr = e->expression ();
    ICode::Status status = visitExpr (subExpr);
    if (status != ICode::OK) {
        return status;
    }

    Type* ty = subExpr->resultType ();
    if (ty->secrecSecType ()->isPrivate () ||
        ty->secrecDataType () != DATATYPE_UINT8 ||
        ty->secrecDimType () != 1)
    {
        m_log.fatal () << "Invalid argument. Expected public byte array, got "
            << ty->toString () << ".";
        m_log.fatal () << "Type error at " << e->location () << ".";
        return ICode::E_TYPE;
    }

    TypeNonVoid* resultType = TypeNonVoid::get(getContext (),
            PublicSecType::get (getContext ()), DATATYPE_STRING);
    e->setResultType (resultType);
    return ICode::OK;
}

ICode::Status TreeNodeExprBytesFromString::accept (TypeChecker& tyChecker) {
    return tyChecker.visit (this);
}

ICode::Status TypeChecker::visit (TreeNodeExprBytesFromString* e) {
    if (e->haveResultType ())
        return ICode::OK;

    TreeNodeExpr* subExpr = e->expression ();
    ICode::Status status = visitExpr (subExpr);
    if (status != ICode::OK) {
        return status;
    }

    Type* ty = subExpr->resultType ();
    if (ty->secrecSecType ()->isPrivate () ||
        ty->secrecDataType () != DATATYPE_STRING ||
        ty->secrecDimType () != 0)
    {
        m_log.fatal () << "Invalid argument. Expected public string, got "
            << ty->toString () << ".";
        m_log.fatal () << "Type error at " << e->location () << ".";
        return ICode::E_TYPE;
    }

    TypeNonVoid* resultType = TypeNonVoid::get(getContext (),
            PublicSecType::get (getContext ()), DATATYPE_UINT8, 1);
    e->setResultType (resultType);
    return ICode::OK;
}

ICode::Status TypeChecker::visit (TreeNodeSecTypeF* ty) {
    if (ty->cachedType () != 0)
        return ICode::OK;

    if (ty->isPublic ()) {
        ty->setCachedType (PublicSecType::get (getContext ()));
        return ICode::OK;
    }

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
    SecrecDimType destDim = eType->secrecDimType ();
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


ICode::Status TypeChecker::checkVarInit (TypeNonVoid* ty,
                                         TreeNodeVarInit* varInit)
{
    SecrecDimType n = 0;

    if (m_st->findFromCurrentScope (varInit->variableName ()) != 0) {
        m_log.fatal () << "Redeclaration of variable at " << varInit->location () << ".";
        return ICode::E_TYPE;
    }

    BOOST_FOREACH (TreeNode* node, varInit->shape ()->children ()) {
        assert (dynamic_cast<TreeNodeExpr*>(node) != 0);
        TreeNodeExpr* e = static_cast<TreeNodeExpr*>(node);
        setContextPublicIntScalar (e, getContext ());
        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) return s;
        if (checkAndLogIfVoid (e)) return ICode::E_TYPE;
        if (! isPublicIntScalar (e->resultType ())) {
            m_log.fatal() << "Expecting public unsigned integer scalar at "
                          << e->location() << ".";
            return ICode::E_TYPE;
        }

        ++ n;
    }

    if (n > 0 && n != ty->secrecDimType()) {
        m_log.fatal() << "Mismatching number of shape components in declaration at "
                      << varInit->location() << ".";
        return ICode::E_TYPE;
    }

    if (varInit->rightHandSide () != 0) {
        TreeNodeExpr *e = varInit->rightHandSide ();
        setContextType (e, ty);
        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) return s;
        if (checkAndLogIfVoid (e)) return ICode::E_TYPE;
        e = classifyIfNeeded (e, ty->secrecSecType ());
        if (! ty->canAssign (e->resultType ())) {
            m_log.fatal () << "Illegal assignment at " << varInit->location () << ".";
            m_log.fatal () << "Got " << *e->resultType () << " expected " << *ty << ".";
            return ICode::E_TYPE;
        }

    }

    return ICode::OK;
}

// Note that declarations are type checked very lazility, checks of
// individual variable initializations will be requested by the code
// generator (see CodeGen::cgVarInit).
ICode::Status TypeChecker::visit (TreeNodeStmtDecl* decl) {
    typedef DataTypeBasic DTB;
    typedef TypeNonVoid TNV;

    if (decl->m_type != 0) {
        return ICode::OK;
    }

    TreeNodeType *type = decl->varType ();
    ICode::Status s = visit (type);
    if (s != ICode::OK) return s;

    assert (!type->secrecType()->isVoid());
    assert (dynamic_cast<TNV*>(type->secrecType()) != 0);
    TNV* justType = static_cast<TNV*>(type->secrecType());

    assert(justType->kind() == TNV::BASIC);
    assert(dynamic_cast<DTB*>(justType->dataType()) != 0);
    DTB* dataType = static_cast<DTB*>(justType->dataType());

    decl->m_type = TypeNonVoid::get (getContext (),
        DataTypeVar::get (getContext (), dataType));

    if (decl->procParam ()) {
        // some sanity checks that parser did its work correctly.
        assert (boost::size (decl->initializers ()) == 1);
        assert (decl->initializer () != 0);
        assert (decl->shape ()->children ().empty ());
        assert (decl->initializer ()->rightHandSide () == 0);
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
        if (secType->isPublic ()) {
            switch (tyNode->dataType ()->dataType ()) {
            case DATATYPE_XOR_UINT8:
            case DATATYPE_XOR_UINT16:
            case DATATYPE_XOR_UINT32:
            case DATATYPE_XOR_UINT64:
                m_log.fatal () << "XOR types do not have public representation!";
                m_log.fatal () << "Type error at " << _ty->location () << ".";
                return ICode::E_TYPE;
            default:
                break;
            }
        }
        tyNode->m_cachedType = TypeNonVoid::get (m_context,
            secType, tyNode->dataType ()->dataType (),
                     tyNode->dimType ()->dimType());
    }
    else {
        assert (dynamic_cast<TreeNodeTypeVoid*>(_ty) != 0);
        _ty->m_cachedType = TypeVoid::get (getContext ());
    }

    return ICode::OK;
}

ICode::Status TypeChecker::visit (TreeNodeStmtPrint* stmt) {
    BOOST_FOREACH (TreeNode* node, stmt->expressions ()) {
        assert (dynamic_cast<TreeNodeExpr*>(node) != 0);
        TreeNodeExpr* e = static_cast<TreeNodeExpr*>(node);
        e->setContextSecType (PublicSecType::get (getContext ()));
        e->setContextDimType (0);

        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) {
            return s;
        }

        if (checkAndLogIfVoid (e)) {
            return ICode::E_TYPE;
        }

        bool isLegalType = true;
        if (  e->resultType()->secrecSecType()->isPrivate ()  ||
            ! e->resultType()->isScalar ()) {
            isLegalType = false;
        }

        SecrecDataType dType = e->resultType ()->secrecDataType ();
        if (  dType != DATATYPE_STRING && dType != DATATYPE_BOOL && ! isNumericDataType (dType)) {
            isLegalType = false;
        }

        if (! isLegalType) {
            m_log.fatal () << "Invalid argument to \"print\" statement."
                           << "Got " << *e->resultType() << " at " << stmt->location() << ".";
            return ICode::E_TYPE;
        }
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

        setContextType (e, procType);
        ICode::Status s = visitExpr (e);
        if (s != ICode::OK) return s;
        e = classifyIfNeeded (e, procType->secrecSecType ());
        if (!procType->canAssign (e->resultType ()) ||
             procType->secrecDimType () != e->resultType ()->secrecDimType ())
        {
            m_log.fatal () << "Cannot return value of type " << *e->resultType ()
                           << " from function with type "
                           << *procType << " at "
                           << stmt->location () << ".";
            return ICode::E_TYPE;
        }
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

TreeNodeExpr* TypeChecker::classifyIfNeeded (TreeNodeExpr* child, SecurityType* need) {
    if (need == 0) {
        return child;
    }

    SecurityType* haveSecType = child->resultType ()->secrecSecType ();
    if (need->isPublic () && haveSecType->isPrivate ()) assert (false);
    if (need->isPrivate () && haveSecType->isPrivate ()) assert (need == haveSecType);
    if (need->isPrivate () && haveSecType->isPublic ()) {
        TreeNode* node = child->parent ();
        SecrecDataType destDType = child->resultType()->secrecDataType ();
        SecrecDimType dimDType = child->resultType ()->secrecDimType ();
        TypeNonVoid* newTy = TypeNonVoid::get (getContext (), need, destDType, dimDType);
        TreeNodeExprClassify *ec = new TreeNodeExprClassify (need, child->location());
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
        child->setContextSecType (PublicSecType::get (getContext ()));
        ec->setContext (child);
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

ICode::Status TypeChecker::checkIndices (TreeNode* node, SecrecDimType& destDim) {
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
            setContextPublicIntScalar (e, getContext ());
            ICode::Status s = visitExpr (e);
            if (s != ICode::OK) {
                return s;
            }

            if (checkAndLogIfVoid (e)) {
                return ICode::E_TYPE;
            }

            TypeNonVoid* eTy = static_cast<TypeNonVoid*>(e->resultType());

            if (! isPublicIntScalar (eTy)) {
                m_log.fatal() << "Invalid type for index at " << e->location() << ". "
                              << "Expected public integer scalar, got " << *eTy << ".";
                return ICode::E_TYPE;
            }
        }
    }

    return ICode::OK;
}


} // namespace SecreC
