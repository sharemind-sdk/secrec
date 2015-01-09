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

#ifndef SECREC_LOG_H
#define SECREC_LOG_H

#include <cassert>
#include <deque>
#include <sstream>
#include <utility>

/**
 * \todo Refactor this and ninja everything from sharemind logging facilities (low priority).
 */

namespace SecreC {

struct CompileLogMessage {
    enum Type { Fatal, Error, Warning, Info, Debug };

    inline CompileLogMessage(Type messageType, std::string msg)
        : type(messageType), message(std::move(msg)) {}

    Type        const type;
    std::string const message;
};

class CompileLog;
class TreeNode;

class CompileLogStream {
public: /* Methods: */

    inline CompileLogStream(const CompileLogStream &copy)
        : m_log (copy.m_log), m_type (copy.m_type)
    {
        assert(copy.m_os.str().empty());
    }

    inline CompileLogStream(CompileLog &log,
                            CompileLogMessage::Type messageType)
        : m_log(log)
        , m_type(messageType) {}

    inline ~CompileLogStream();

    template <typename T>
    inline CompileLogStream &operator<<(const T& v) { m_os << v; return *this; }

    CompileLog&                   m_log;
    std::ostringstream            m_os;
    CompileLogMessage::Type const m_type;
};

class CompileLog {
public: /* Methods: */
    inline CompileLogStream fatal() { return CompileLogStream(*this, CompileLogMessage::Fatal); }
    inline CompileLogStream error() { return CompileLogStream(*this, CompileLogMessage::Error); }
    inline CompileLogStream warning() { return CompileLogStream(*this, CompileLogMessage::Warning); }
    inline CompileLogStream info() { return CompileLogStream(*this, CompileLogMessage::Info); }
    inline CompileLogStream debug() { return CompileLogStream(*this, CompileLogMessage::Debug); }
    CompileLogStream fatalInProc(const TreeNode * n);
    CompileLogStream errorInProc(const TreeNode * n);
    CompileLogStream warningInProc(const TreeNode * n);
    CompileLogStream infoInProc(const TreeNode * n);
    CompileLogStream debugInProc(const TreeNode * n);

    inline void addMessage(const CompileLogMessage &msg) { m_messages.push_back(msg); }
    inline const std::deque<CompileLogMessage> &messages() const { return m_messages; }

private: /* Fields: */
    std::deque<CompileLogMessage> m_messages;
};

inline CompileLogStream::~CompileLogStream() {
    if (!m_os.str().empty()) {
        m_log.addMessage(CompileLogMessage(m_type, m_os.str()));
    }
}

inline std::ostream &operator<<(std::ostream &out, const CompileLog &log)
{
    using CLM = CompileLogMessage::Type;
    for (const auto & logMessage: log.messages ()) {
        switch (logMessage.type) {
        case CLM::Fatal:   out << "[FATAL] "; break;
        case CLM::Error:   out << "[ERROR] "; break;
        case CLM::Warning: out << "[WARN ] "; break;
        case CLM::Info:    out << "[INFO ] "; break;
        case CLM::Debug:   out << "[DEBUG] "; break;
        }

        out << logMessage.message << std::endl;
    }
    return out;
}

} // namespace SecreC

#endif // LOG_H
