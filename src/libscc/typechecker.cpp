#include "typechecker.h"

#include <boost/foreach.hpp>
#include <boost/range.hpp>
#include "log.h"
#include "symbol.h"
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

Symbol* TypeChecker::findIdentifier (SymbolCategory type, const TreeNodeIdentifier* id) const {
    Symbol* s = m_st->find (type, id->value ());
    if (s == 0) {
        m_log.fatalInProc(id) << "Identifier '" << id->value()
                              << "' at " << id->location()
                              << " not in scope.";
    }

    return s;
}

SymbolSymbol* TypeChecker::getSymbol (TreeNodeIdentifier *id) {
    SymbolSymbol *s = m_st->find<SYM_SYMBOL>(id->value ());
    if (s == 0) {
        m_log.fatalInProc(id) << "Undeclared identifier '" << id->value ()
                              << "' at " << id->location() << '.';
        return 0;
    }

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
    DataType* destDType = child->resultType()->secrecDataType();
    if (child->haveContextDataType()) {
        if (dtypeDeclassify (getContext (), child->contextDataType()) == destDType)
            destDType = child->contextDataType();
    }

    const SecrecDimType dimDType = child->resultType()->secrecDimType();
    TypeBasic * const newTy = TypeBasic::get(getContext(), need, destDType, dimDType);
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
        e->setContextDataType (DataTypePrimitive::get (getContext (), DATATYPE_BOOL));
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
            TCGUARD (visitExpr(e));

            if (checkAndLogIfVoid(e))
                return E_TYPE;

            TypeNonVoid* eTy = static_cast<TypeNonVoid*>(e->resultType());

            if (! eTy->isPublicUIntScalar ()) {
                m_log.fatalInProc(node) << "Invalid type for index at "
                                        << e->location()
                                        << ". Expecting 'uint', got "
                                        << *eTy << '.';
                return E_TYPE;
            }
        }
    }

    return OK;
}

bool TypeChecker::canPrintValue (Type* ty) {
    if (ty->isVoid ())
        return false;

    if (ty->secrecSecType()->isPrivate ())
        return false;

    if (! ty->isScalar ())
        return false;

    DataType* dType = ty->secrecDataType ();
    if (! dType->isString () &&
        ! dType->isBool () &&
        ! isNumericDataType (dType))
    {
        return false;
    }

    return true;
}

/*******************************************************************************
  TreeNodeStructDecl
*******************************************************************************/

// TODO: move to different file
TypeChecker::Status TypeChecker::visit(TreeNodeStructDecl* decl) {
    TreeNodeIdentifier* id = decl->identifier ();
    BOOST_FOREACH (Symbol* s, m_st->findAll (SYM_TYPE, id->value ())) {
        m_log.fatal () << "Redeclaration of type \'" << id->value () << "\' at " << decl->location () << ".";
        if (s->location ()) {
            m_log.fatal () << "Previous declaration at " << *s->location () << ".";
        }

        return E_TYPE;
    }

    assert (false && "TODO: construct appropriate symbol!");
    // SymbolDataType* symDataType = new SymbolDataType(id->value (), DATATYPE_INT64);
    // m_st->appendSymbol (symDataType);
    return OK;
}

} // namespace SecreC
