#ifndef SECREC_LOG_H
#define SECREC_LOG_H

#include <boost/foreach.hpp>
#include <cassert>
#include <deque>
#include <sstream>
#include "misc.h"
#include "types.h"

/**
 * \todo Refactor this and ninja everything from sharemind logging facilities (low priority).
 */

namespace SecreC {

struct CompileLogMessage {
    enum Type { Fatal, Error, Warning, Info, Debug };

    inline CompileLogMessage(Type messageType, const std::string &msg)
        : type(messageType), message(msg) {}

    Type        const type;
    std::string const message;
};

class CompileLog;

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

} // namespace SecreC


inline std::ostream &operator<<(std::ostream &out,
                                const SecreC::CompileLog &log)
{
    typedef SecreC::CompileLogMessage CLM;
    BOOST_FOREACH (const CLM& msg, log.messages ()) {
        switch (msg.type) {
        case CLM::Fatal:   out << "[FATAL] "; break;
        case CLM::Error:   out << "[ERROR] "; break;
        case CLM::Warning: out << "[WARN ] "; break;
        case CLM::Info:    out << "[INFO ] "; break;
        case CLM::Debug:   out << "[DEBUG] "; break;
        }

        out << msg.message << std::endl;
    }
    return out;
}

#endif // LOG_H
