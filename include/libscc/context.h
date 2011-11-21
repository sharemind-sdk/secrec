/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef CONTEXT_H
#define CONTEXT_H

namespace SecreC {

class ContextImpl;

class Context {
private:

    Context (const Context&); // DO NOT IMPLEMENT
    void operator = (const Context&); // DO NOT IMPLEMENT

public: /* Fields: */

    Context ();
    ~Context ();

    inline ContextImpl* pImpl () {
        return m_pImpl;
    }

private: /* Fields: */

    ContextImpl* m_pImpl;
};

}

#endif // CONTEXT_H
