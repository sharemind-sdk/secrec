/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SECREC_CONTEXT_H
#define SECREC_CONTEXT_H

namespace SecreC {

class ContextImpl;


/**
 * Context class is responsible for allocating types and constant symbols.
 */
class Context {
private:

    Context (const Context&); // DO NOT IMPLEMENT
    void operator = (const Context&); // DO NOT IMPLEMENT

public: /* Fields: */

    Context ();
    ~Context ();

    inline ContextImpl* pImpl () const {
        return m_pImpl;
    }

private: /* Fields: */

    ContextImpl* m_pImpl;
};

}

#endif // CONTEXT_H
