#include "typechecker.h"

#include <boost/foreach.hpp>
#include <boost/range.hpp>

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

bool TypeChecker::getForInstantiation (InstanceInfo& info) {
    return m_instantiator->getForInstantiation (info);
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
            e->setContextPublicIntScalar (getContext ());
            ICode::Status s = visitExpr (e);
            if (s != ICode::OK) {
                return s;
            }

            if (checkAndLogIfVoid (e)) {
                return ICode::E_TYPE;
            }

            TypeNonVoid* eTy = static_cast<TypeNonVoid*>(e->resultType());

            if (! eTy->isPublicIntScalar ()) {
                m_log.fatal() << "Invalid type for index at " << e->location() << ". "
                              << "Expected public integer scalar, got " << *eTy << ".";
                return ICode::E_TYPE;
            }
        }
    }

    return ICode::OK;
}


} // namespace SecreC
