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

#include "Constant.h"
#include "Imop.h"
#include "Intermediate.h"
#include "SecurityType.h"
#include "Symbol.h"
#include "TreeNode.h"
#include "Types.h"

#include <boost/interprocess/containers/flat_set.hpp>

namespace {

static const unsigned inlineThreshold = 50;

} /* anonymous namespace */

namespace SecreC {

class Inliner {

public: /* Methods: */

    Inliner (std::vector<Imop*>& todo, Imop* call, ICode& code)
        : m_todo (todo)
        , m_call (call)
        , m_caller (call->block ()->proc ())
        , m_code (code)
        , m_paramIdx (0)
        , m_returnBlock (nullptr)
        , m_returnLabel (nullptr)
        , m_proc (m_call->callDest ()->block ()->proc ())
        , m_destBlock (m_call->block ())
        , m_destBlockIt (m_destBlock->erase (blockIterator (*m_call)))
        {
            for (Symbol* arg : m_call->useRange ()) {
                m_suppliedArgs.push_back (arg);
            }

            // Find RETCLEAN.
            for (auto succ : m_destBlock->successors ()) {
                if (succ.second == Edge::CallPass) {
                    m_returnBlock = succ.first;
                    break;
                }
            }

            assert (m_returnBlock != nullptr);

            // CALL was removed. It is possible that the CALL block is
            // empty and will be removed so we may have to reset the
            // destination of a jump preceding the CALL.
            m_blockMap.insert (std::make_pair (m_destBlock, m_destBlock));
        }

    void run () {
        removeRetClean ();
        removeCallEdges ();
        copyImops ();
        removeEmptyBlocks ();
        fixJumps ();

        // Fix CFG.
        m_code.program ().buildProcedureCFG (*m_caller);

        delete m_call;
    }

private: /* Methods: */

    void inlineImop (const Imop& imop) {
        SymbolTable& symbols = m_code.symbols ();
        Imop::Type ty = imop.type ();
        Imop* i = nullptr;

        if (isCopyable (imop)) {
            i = copyImop (imop);
        }
        else if (ty == Imop::RELEASE) {
            Symbol* arg = getSymbol (imop.arg1 ());
            if (! arg->isConstant ())
                i = new Imop (imop.creator (), Imop::RELEASE, nullptr, arg);
        }
        else if (ty == Imop::RETURN) {
            if (imop.nArgs() > 1) {
                Imop::OperandConstRange range = m_call->defRange ();
                Imop::OperandConstIterator callIt = range.begin ();
                Imop::OperandConstIterator retIt = ++imop.operandsBegin ();

                while (retIt != imop.operandsEnd ()) {
                    const TypeNonVoid* ty = (*retIt)->secrecType ();
                    Imop* copy;

                    if (ty->secrecDimType () > 0) {
                        Symbol* sizeSym = static_cast<SymbolSymbol*> (*retIt)->getSizeSym ();
                        assert(sizeSym != nullptr);
                        copy = new Imop (imop.creator (), Imop::COPY,
                                         *callIt, getSymbol (*retIt),
                                         getSymbol (sizeSym));
                    } else {
                        copy = new Imop (imop.creator (), Imop::ASSIGN,
                                         *callIt, getSymbol (*retIt));
                    }

                    m_destBlock->insert (m_destBlockIt, *copy);
                    copy->setBlock (m_destBlock);

                    if (ty->secrecDimType () > 0 || ty->secrecSecType ()->isPrivate ()) {
                        Imop* release = new Imop (imop.creator (), Imop::RELEASE,
                                                  nullptr, getSymbol (*retIt));
                        m_destBlock->insert (m_destBlockIt, *release);
                        release->setBlock (m_destBlock);
                    }

                    ++retIt;
                    ++callIt;
                }
            }

            i = new Imop (imop.creator (), Imop::JUMP, m_returnLabel);
        }
        else if (ty == Imop::ALLOC) {
            Symbol* newSym = symbols.appendTemporary (imop.dest ()->secrecType ());
            m_symMap.insert (std::make_pair (imop.dest (), newSym));

            if (imop.nArgs () == 3) {
                // has default value
                i = new Imop (imop.creator (), Imop::ALLOC, newSym,
                              getSymbol (imop.arg1 ()),
                              getSymbol (imop.arg2 ()));
            }
            else {
                i = new Imop (imop.creator (), Imop::ALLOC, newSym,
                              getSymbol (imop.arg1 ()));
            }
        }
        else if (ty == Imop::CALL) {
            i = copyImop (imop);
            m_todo.push_back (i);
        }
        else if (ty == Imop::PARAM) {
            Symbol* s = m_suppliedArgs[m_paramIdx++];
            if (s->secrecType ()->isScalar () &&
                ! s->secrecType ()->secrecSecType ()->isPrivate ())
            {
                Symbol* newSym = symbols.appendTemporary (s->secrecType ());
                Imop* i = new Imop (nullptr, Imop::DECLARE, newSym);
                i->setBlock (m_destBlock);
                m_destBlock->insert (m_destBlockIt, *i);
                i = new Imop (nullptr, Imop::ASSIGN, newSym, s);
                i->setBlock (m_destBlock);
                m_destBlock->insert (m_destBlockIt, *i);
                m_symMap.insert (std::make_pair (imop.dest (), newSym));
            }
            else {
                m_symMap.insert (std::make_pair (imop.dest (), s));
            }
        }
        else if (ty == Imop::DOMAINID) {
            Symbol* newSym = symbols.appendTemporary (imop.dest ()->secrecType ());
            m_symMap.insert (std::make_pair (imop.dest (), newSym));
            i = new Imop (imop.creator (), Imop::DOMAINID, newSym, imop.arg1 ());
        }
        else if (ty == Imop::COMMENT) {
            // nothing
        }
        else if (ty == Imop::JT || ty == Imop::JF) {
            i = new Imop (imop.creator (), imop.type (), imop.dest (), getSymbol (imop.arg1 ()));
        }
        else if (ty == Imop::JUMP) {
            i = new Imop (imop.creator (), Imop::JUMP, imop.dest ());
        }
        else {
            assert (false);
        }

        if (i != nullptr) {
            m_destBlock->insert (m_destBlockIt, *i);
            i->setBlock (m_destBlock);
            m_imopMap.insert (std::make_pair (&imop, i));
        }
    }

    void removeRetClean () {
        ImopList::iterator it = m_returnBlock->begin ();
        while (it != m_returnBlock->end () && it->type () != Imop::RETCLEAN)
            ++it;
        assert (it != m_returnBlock->end ());

        Imop* retClean = &*it;
        StringRef str ("Inlined procedure call landing pad");
        Imop* comment = new Imop (nullptr, Imop::COMMENT, nullptr,
                                  ConstantString::get (m_code.context (), str));
        retClean->replaceWith (*comment);
        m_returnLabel = m_code.symbols ().label (retClean);
        m_returnLabel->setBlock (m_returnBlock);
    }

    void removeCallEdges () {
        // Remove FromCall edge from m_called procedure.
        m_call->callDest ()->block ()->removePred (*m_call->block ());

        // Remove ToRet from m_called procedure exits
        // Remove FromRet from m_returnBlock.
        for (Block* exit : m_proc->exitBlocks ()) {
            exit->removeSucc (*m_returnBlock);
            m_returnBlock->removePred (*exit);
        }

        // Remove ToCall.
        m_call->block ()->removeSucc (*m_call->callDest ()->block ());
        // Remove PassTo.
        m_call->block ()->removeSucc (*m_returnBlock);

        // Remove call/return from Procedure.
        m_proc->removeCallFrom (*m_call->block ());
        m_proc->removeReturnTo (*m_returnBlock);

        // Remove PassFrom from m_returnBlock.
        m_returnBlock->removePred (*m_call->block ());
    }

    void copyImops () {
        Procedure::iterator blockInsertIt = ++procIterator (*m_destBlock);
        Procedure::const_iterator fromBlockIt = m_proc->begin ();
        while (fromBlockIt != m_proc->end () && fromBlockIt->reachable ()) {
            const Block& block = *fromBlockIt;
            m_blockMap.insert (std::make_pair (&block, m_destBlock));

            for (const auto& imop : block) {
                inlineImop (imop);
            }

            ++fromBlockIt;

            // Skip unreachable blocks.
            while (fromBlockIt != m_proc->end () && ! fromBlockIt->reachable ())
                ++fromBlockIt;

            // Create next block.
            if (fromBlockIt != m_proc->end ()) {
                Block* newBlock = new Block ();
                newBlock->setProc (m_call->block ()->proc ());
                newBlock->setReachable ();

                m_call->block ()->proc ()->insert (blockInsertIt, *newBlock);
                m_destBlock = newBlock;
                m_destBlockIt = newBlock->end ();
            }
        }
    }

    // Empty blocks can happen because we don't copy comments and
    // RETURN.
    void removeEmptyBlocks () {
        Procedure::iterator blockIt = procIterator (*m_call->block ());
        std::vector<Block*> todo;
        while (blockIt != m_call->block ()->proc ()->end ()) {
            Block* block = &*blockIt;
            ++blockIt;

            if (block->empty ()) {
                assert (blockIt != m_call->block ()->proc ()->end ());
                Block* nextBlock = &*blockIt;

                // Replace jump destination.
                for (auto pair : m_blockMap) {
                    if (pair.second == block)
                        m_blockMap[pair.first] = nextBlock;
                }

                todo.push_back (block);
            }
        }

        for (Block* block : todo) {
            block->unlink ();
            delete block;
        }
    }

    void fixJumps () {
        SymbolTable& symbols = m_code.symbols ();
        for (auto& block : *m_caller) {
            for (auto& imop : block) {
                if (imop.isJump ()) {
                    SymbolLabel* l = imop.jumpDest ();
                    assert (l->block () != nullptr);

                    if (m_blockMap.count (l->block ()) > 0) {
                        Imop* destImop = &(m_blockMap[l->block ()]->front ());
                        SymbolLabel* newL = symbols.label (destImop);
                        newL->setBlock (destImop->block ());
                        imop.setDest (newL);
                    }
                }
            }
        }
    }

    Symbol* getSymbol (Symbol* sym) {
        assert (sym != nullptr);

        // TODO: are there other symbol types that we need to consider?

        Symbol::Type ty = sym->symbolType ();
        if (ty == SYM_PROCEDURE || ty == SYM_CONSTANT || sym->isGlobal ()) {
            return sym;
        }
        else if (ty == SYM_SYMBOL) {
            if (m_symMap.count (sym) == 0)
                assert (false && "missing symbol");

            return m_symMap[sym];
        }
        else if (ty == SYM_LABEL) {
            SymbolLabel* l = static_cast<SymbolLabel*> (sym);

            // TODO: m_imopMap is only used for this case. This is
            // necessary for RETCLEAN whose dest is the corresponding
            // CALL. Neither the VirtualMachine or Compiler do
            // anything with RETCLEAN...
            assert (m_imopMap.count (l->target ())> 0);
            Imop* newTarget = m_imopMap[l->target ()];

            return m_code.symbols ().label (newTarget);
        }
        else {
            assert (false);
        }
    }

    bool isCopyable (const Imop& imop) {
        switch (imop.type ()) {
            case Imop::ASSIGN:
            case Imop::DECLARE:
            case Imop::CAST:
            case Imop::CLASSIFY:
            case Imop::DECLASSIFY:
            case Imop::UINV:
            case Imop::UNEG:
            case Imop::UMINUS:
            case Imop::TOSTRING:
            case Imop::STRLEN:
            case Imop::MUL:
            case Imop::DIV:
            case Imop::MOD:
            case Imop::ADD:
            case Imop::SUB:
            case Imop::EQ:
            case Imop::NE:
            case Imop::LE:
            case Imop::LT:
            case Imop::GE:
            case Imop::GT:
            case Imop::LAND:
            case Imop::LOR:
            case Imop::BAND:
            case Imop::BOR:
            case Imop::XOR:
            case Imop::SHL:
            case Imop::SHR:
            case Imop::STORE:
            case Imop::LOAD:
            case Imop::COPY:
            case Imop::PRINT:
            case Imop::SYSCALL:
            case Imop::PUSH:
            case Imop::PUSHREF:
            case Imop::PUSHCREF:
            case Imop::ERROR:
            case Imop::RETCLEAN:
                return true;

            default:
                return false;
        }
    }

    Imop* copyImop (const Imop& imop) {
        for (Symbol* s : imop.defRange ()) {
            if (m_symMap.count (s) == 0 && ! s->isGlobal ()) {
                Symbol* newSym = m_code.symbols ().appendTemporary (s->secrecType ());
                m_symMap.insert (std::make_pair (s, newSym));
            }
        }

        Imop::OperandList args;
        for (Symbol* s : imop.operands ()) {
            if (s == nullptr)
                args.push_back (s);
            else
                args.push_back (getSymbol (s));
        }

        return new Imop (imop.creator (), imop.type (), args);
    }

private: /* Fields: */

    std::vector<Imop*>& m_todo;
    Imop* m_call;
    Procedure* m_caller;
    ICode& m_code;
    std::map<const Symbol*, Symbol*> m_symMap;
    std::map<const Imop*, Imop*> m_imopMap;
    std::map<const Block*, Block*> m_blockMap;
    unsigned m_paramIdx;
    std::vector<Symbol*> m_suppliedArgs;
    Block* m_returnBlock;
    SymbolLabel* m_returnLabel;
    Procedure* m_proc; // the procedure we are inlining
    Block* m_destBlock; // the block we are copying ops to
    Block::const_iterator m_destBlockIt; // used to insert into m_destBlock

}; /* class Inliner { */

bool isRecursive (Imop* call) {
    boost::container::flat_set<Procedure*> visited;
    std::vector<Procedure*> todo;

    todo.push_back (call->callDest ()->block ()->proc ());

    while (todo.size () > 0) {
        Procedure* proc = &*todo.back ();
        todo.pop_back ();

        if (visited.count (proc) != 0)
            continue;

        visited.insert (proc);

        for (const auto& block : *proc) {
            if (!block.reachable ())
                continue;

            for (const auto& imop : block) {
                if (imop.type () == Imop::CALL) {
                    if (imop.dest () == call->dest ())
                        return true;
                    todo.push_back (imop.callDest ()->block ()->proc ());
                }
            }
        }
    }

    return false;
}

bool shouldInline (Imop* call) {
    if (static_cast<const SymbolProcedure*> (call->dest ())->procedureName () == "main")
        return false;

    if (isRecursive (call))
        return false;

    unsigned cost = 0;

    for (const Symbol* arg : call->useRange ()) {
        (void) arg;
        ++cost;
    }

    const Imop* procImop = call->callDest ();
    assert (procImop != nullptr);
    for (const auto& block : *procImop->block ()->proc ()) {
        if (!block.reachable ())
            continue;

        for (const auto& imop : block) {
            (void) imop;
            ++cost;
        }
    }

    return cost < inlineThreshold;
}

void inlineCall (std::vector<Imop*>& todo, ICode& code) {
    Imop* call = todo.back ();
    todo.pop_back ();

    if (! shouldInline (call))
        return;

    Inliner inliner (todo, call, code);
    inliner.run ();
}

void inlineCalls (ICode& code) {
    std::vector<Imop*> todo;

    for (auto& proc : code.program ()) {
        for (auto& block : proc) {
            if (!block.reachable ())
                continue;

            for (auto& imop : block) {
                if (imop.type () == Imop::CALL) {
                    todo.push_back (&imop);
                }
            }
        }
    }

    while (! todo.empty ()) {
        inlineCall (todo, code);
    }

    // Remove unused procedures.
    std::vector<Procedure*> remove;
    for (auto& proc : code.program ()) {
        // TODO: how to identify the START procedure? Currently we use
        // "name != NULL". Not sure if it's ideal.
        if (proc.name () != nullptr && proc.callFrom ().empty ())
            remove.push_back (&proc);
    }

    for (Procedure* proc : remove) {
        proc->unlink ();
        delete proc;
    }

    // Fix imop indexes.
    code.program ().numberInstructions ();
}

} /* namespace SecreC */
