#include "typechecker.h"

#include <boost/foreach.hpp>
#include <boost/range.hpp>
#include "log.h"
#include "symboltable.h"
#include "treenode.h"
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

TypeChecker::Status TypeChecker::visitExpr(TreeNodeExpr * e) {
    return e->accept(*this);
}

bool TypeChecker::getForInstantiation (InstanceInfo& info) {
    return m_instantiator->getForInstantiation (info);
}

TypeChecker::Status TypeChecker::visit(TreeNodeSecTypeF * ty) {
    if (ty->cachedType () != 0)
        return OK;

    if (ty->isPublic ()) {
        ty->setCachedType (PublicSecType::get (getContext ()));
        return OK;
    }

    TreeNodeIdentifier* id = ty->identifier ();
    Symbol* s = findIdentifier (id);
    if (s == 0)
        return E_TYPE;

    if (s->symbolType () != Symbol::PDOMAIN) {
        m_log.fatalInProc(ty) << "Expecting privacy domain at " << ty->location () << '.';
        return E_TYPE;
    }

    assert (dynamic_cast<SymbolDomain*>(s) != 0);
    ty->setCachedType (static_cast<SymbolDomain*>(s)->securityType ());
    return OK;
}


TypeChecker::Status TypeChecker::visit(TreeNodeType * _ty) {
    if (_ty->m_cachedType != 0)
        return OK;

    if (_ty->type () == NODE_TYPETYPE) {
        assert (dynamic_cast<TreeNodeTypeType*>(_ty) != 0);
        TreeNodeTypeType* tyNode = static_cast<TreeNodeTypeType*>(_ty);
        TreeNodeSecTypeF* secTyNode = tyNode->secType ();
        Status status = visit(secTyNode);
        if (status != OK)
            return status;

        SecurityType* secType = secTyNode->cachedType ();
        if (secType->isPublic ()) {
            switch (tyNode->dataType ()->dataType ()) {
            case DATATYPE_XOR_UINT8:
            case DATATYPE_XOR_UINT16:
            case DATATYPE_XOR_UINT32:
            case DATATYPE_XOR_UINT64:
                m_log.fatalInProc(_ty) << "XOR types do not have public representation at "
                                       << _ty->location () << '.';
                return E_TYPE;
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

    return OK;
}

Symbol* TypeChecker::findIdentifier (TreeNodeIdentifier* id) const {
    Symbol* s = m_st->find (id->value ());
    if (s == 0) {
        m_log.fatalInProc(id) << "Idenfier '" << id->value()
                              << "'' at " << id->location()
                              << " not in scope.";
    }

    return s;
}

SymbolSymbol* TypeChecker::getSymbol (TreeNodeIdentifier *id) {
    Symbol *s = m_st->find (id->value ());
    if (s == 0) {
        m_log.fatalInProc(id) << "Undeclared identifier '" << id->value ()
                              << "'' at " << id->location() << '.';
        return 0;
    }

    assert (s->symbolType() == Symbol::SYMBOL);
    assert (dynamic_cast<SymbolSymbol*>(s) != 0);
    return static_cast<SymbolSymbol*>(s);
}

// Potentially replaces the child in parent list. Does not invalidate iterators.
TreeNodeExpr * TypeChecker::classifyIfNeeded(TreeNodeExpr * child,
                                             SecurityType * need)
{
    if (need == 0)
        return child;

    SecurityType * const haveSecType = child->resultType()->secrecSecType();
    assert(!(need->isPrivate() && haveSecType->isPrivate()) || need == haveSecType);

    if (need->isPublic() || haveSecType->isPrivate())
        return child;

    TreeNode * const parent = child->parent();
    const SecrecDataType destDType = child->haveContextDataType()
                                   ? child->contextDataType()
                                   : child->resultType()->secrecDataType();

    const SecrecDimType dimDType = child->resultType()->secrecDimType();
    TypeNonVoid * const newTy = TypeNonVoid::get(getContext(), need, destDType, dimDType);
    TreeNodeExprClassify * const ec = new TreeNodeExprClassify(need, child->location());
    ec->appendChild(child);
    ec->resetParent(parent);
    ec->setResultType(newTy);
    BOOST_FOREACH (TreeNode *& n, parent->children()) {
        if (n == child) {
            n = ec;
            break;
        }
    }

    // patch up context types just in case
    ec->setContext(child);
    child->setContextSecType(PublicSecType::get(getContext()));
    child->setContextDataType(destDType);
    child = ec;
    return child;
}

bool TypeChecker::checkAndLogIfVoid (TreeNodeExpr* e) {
    assert (e->haveResultType());
    if (e->resultType()->isVoid()) {
        m_log.fatalInProc(e) << "Subexpression has type void at "
                             << e->location() << '.';
        return true;
    }

    return false;
}

TypeChecker::Status TypeChecker::checkPublicBooleanScalar (TreeNodeExpr * e) {
    assert (e != 0);
    if (! e->haveResultType ()) {
        e->setContextSecType (PublicSecType::get (getContext ()));
        e->setContextDataType (DATATYPE_BOOL);
        e->setContextDimType (0);

        if (visitExpr (e) != OK)
            return E_TYPE;

        if (!e->havePublicBoolType())
            return E_TYPE;
    }

    return OK;
}

TypeChecker::Status TypeChecker::checkIndices(TreeNode * node,
                                              SecrecDimType & destDim)
{
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
            return E_TYPE;
        }

        BOOST_FOREACH (TreeNode* j, tNode->children ()) {
            if (j->type() == NODE_EXPR_NONE) {
                continue;
            }

            assert (dynamic_cast<TreeNodeExpr*>(j) != 0);
            TreeNodeExpr* e = static_cast<TreeNodeExpr*>(j);
            e->setContextIndexType (getContext ());
            Status s = visitExpr(e);
            if (s != OK)
                return s;

            if (checkAndLogIfVoid(e))
                return E_TYPE;

            TypeNonVoid* eTy = static_cast<TypeNonVoid*>(e->resultType());

            if (! eTy->isPublicIntScalar ()) {
                m_log.fatalInProc(node) << "Invalid type for index at "
                                        << e->location()
                                        << ". Expected public integer scalar, got "
                                        << *eTy << '.';
                return E_TYPE;
            }
        }
    }

    return OK;
}


} // namespace SecreC
