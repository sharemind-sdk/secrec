/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "context.h"

#include "context_impl.h"

namespace SecreC {

Context::Context ()
    : m_pImpl (new ContextImpl ())
{ }

Context::~Context () {
    delete m_pImpl;
}

}
