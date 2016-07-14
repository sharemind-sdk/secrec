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

#include "Imop.h"
#include "Intermediate.h"
#include "SecurityType.h"
#include "Symbol.h"
#include "TreeNode.h"
#include "Types.h"


#include<iostream>
using namespace std;


namespace {

static const unsigned inlineThreshold = 50;

} /* anonymous namespace */

namespace SecreC {

class Inliner {

public: /* Methods: */

    Inliner (std::vector<Imop*>& todo, Imop* call, ICode& code)
        : m_todo (todo)
        , m_call (call)
        , m_code (code)
        , m_paramIdx (0)
        , m_returnBlock (nullptr)
        , m_returnOp (nullptr)
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
        }

    void run () {
        removeRetClean ();
        removeCallEdges ();
        copyImops ();
        replaceReturnVars ();
        mergeCallSucc ();
        removeEmptyBlocks ();
        fixJumps ();

        // Fix CFG.
        m_code.program ().buildProcedureCFG (*m_call->block ()->proc ());

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
        else if (ty == Imop::ASSIGN) {
            if (imop.nArgs () > 2) {
                assert (false);
            }
            else {
                Symbol* oldDest = imop.dest ();
                Symbol* oldArg = imop.arg1 ();
                Symbol* newDest;

                if (m_symMap.count (oldDest) != 0) {
                    newDest = m_symMap[oldDest];
                }
                else {
                    newDest = symbols.appendTemporary (oldDest->secrecType ());
                    m_symMap.insert (std::make_pair (oldDest, newDest));
                }

                assert (oldArg != nullptr);
                Symbol* newArg = getSymbol (oldArg);
                i = newAssign (imop.creator (), newDest, newArg);
            }
        }
        else if (ty == Imop::RELEASE) {
            Symbol* arg = getSymbol (imop.arg1 ());
            if (! arg->isConstant ())
                i = new Imop (imop.creator (), Imop::RELEASE, nullptr, arg);
        }
        else if (ty == Imop::RETURN) {
            if (imop.nArgs() > 1) {
                m_returnOp = &imop;
            }
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
            m_symMap.insert (std::make_pair (imop.dest (), s));
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
            i = new Imop (imop.creator (), imop.type (), nullptr, getSymbol (imop.arg1 ()));
            m_jumps.push_back (std::make_pair (&imop, i));
        }
        else if (ty == Imop::JUMP) {
            i = new Imop (imop.creator (), Imop::JUMP, nullptr);
            m_jumps.push_back (std::make_pair (&imop, i));
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
        while (it != m_returnBlock->end () && it->type() != Imop::RETCLEAN)
            ++it;

        assert (it != m_returnBlock->end ());
        Imop* retClean = &*it;
        m_returnBlock->erase (it);
        delete retClean;
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

    void replaceReturnVars () {
        if (m_returnOp != nullptr) {
            std::map<const Symbol*, Symbol*> map;
            Imop::OperandConstRange range = m_call->defRange ();
            Imop::OperandConstIterator callIt = range.begin ();
            Imop::OperandConstIterator retIt = ++m_returnOp->operandsBegin ();

            while (retIt != m_returnOp->operandsEnd ()) {
                Symbol* callSym = *callIt;
                Symbol* retSym = *retIt;
                map.insert (std::make_pair (callSym, getSymbol (retSym)));
                ++retIt;
                ++callIt;
            }

            assert (callIt == range.end ());

            Procedure::iterator blockIt = procIterator (*m_call->block ());
            for (; blockIt != m_call->block ()->proc ()->end (); ++blockIt) {
                Block::iterator opIt = blockIt->begin ();
                for (; opIt != blockIt->end (); ++opIt) {
                    Imop& op = *opIt;
                    for (unsigned i = 0; i < op.nArgs (); ++i) {
                        Symbol* arg = op.arg (i);
                        if (arg != nullptr && map.count (arg) != 0) {
                            op.setArg (i, map[arg]);
                        }
                    }
                }
            }
        }
    }

    // Merge the block following the call, it doesn't have to be a
    // separate block anymore.
    void mergeCallSucc () {
        ImopList::iterator it = m_returnBlock->begin ();

        while (it != m_returnBlock->end ()) {
            Imop* op = &*it;
            it = m_returnBlock->erase (it);
            m_destBlock->push_back (*op);
        }

        // Copy return nodes since these will not be calculated when
        // reconstructing the CFG of the procedure.
        for (auto succ : m_returnBlock->successors ()) {
            if (succ.second == Edge::Ret)
                Block::addEdge (*m_destBlock, succ.second, *succ.first);
        }

        m_returnBlock->unlink ();
        delete m_returnBlock;
    }

    // Empty blocks can happen because we don't copy comments and
    // RETURN.
    void removeEmptyBlocks () {
        Procedure::iterator blockIt = procIterator (*m_call->block ());
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

                block->unlink ();
                delete block;
            }
        }
    }

    void fixJumps () {
        SymbolTable& symbols = m_code.symbols ();
        for (const auto& it : m_jumps) {
            const Imop* oldJ = it.first;
            Imop* newJ = it.second;
            SymbolLabel* l = oldJ->jumpDest ();

            assert (l->block () != nullptr);
            assert (m_blockMap.count (l->block ()) > 0);
            Imop* destImop = &(m_blockMap[l->block ()]->front ());

            SymbolLabel* newL = symbols.label (destImop);
            newL->setBlock (destImop->block ());
            newJ->setDest (newL);
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
            if (m_symMap.count (s) == 0) {
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
    ICode& m_code;
    std::map<const Symbol*, Symbol*> m_symMap;
    std::map<const Imop*, Imop*> m_imopMap;
    std::map<const Block*, Block*> m_blockMap;
    std::vector<std::pair<const Imop*, Imop*>> m_jumps;
    unsigned m_paramIdx;
    std::vector<Symbol*> m_suppliedArgs;
    Block* m_returnBlock;
    const Imop* m_returnOp;
    Procedure* m_proc; // the procedure we are inlining
    Block* m_destBlock; // the block we are copying ops to
    Block::const_iterator m_destBlockIt; // used to insert into m_destBlock

}; /* class Inliner { */

bool shouldInline (Imop* call) {
    if (static_cast<const SymbolProcedure*> (call->dest ())->procedureName () == "main")
        return false;

    unsigned cost = 0;

    for (const Symbol* arg : call->useRange ()) {
        (void) arg;
        ++cost;
    }

    const Imop* procImop = call->callDest ();
    assert (procImop != nullptr);

    for (const auto& block : *procImop->block ()->proc ()) {
        for (const auto& imop : block) {
            // Check if the call is recursive
            if (imop.type () == Imop::CALL &&
                procImop == imop.callDest ())
            {
                return false;
            }
            ++cost;
        }
    }

    if (cost < inlineThreshold)
        return true;

    return false;
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
    unsigned long index = 0;

    for (auto& proc : code.program ()) {
        for (auto& block : proc) {
            for (auto& imop : block) {
                imop.setIndex (index++);
            }
        }
    }
}

} /* namespace SecreC */
