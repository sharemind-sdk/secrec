#ifndef LOG_H
#define LOG_H

#include <cassert>
#include <deque>
#include <sstream>
#include "misc.h"
#include "types.h"

namespace SecreC {

struct CompileLogMessage {
        enum Type { Fatal, Error, Warning, Info, Debug } type;
        std::string message;

        inline CompileLogMessage(Type messageType, const std::string &msg)
            : type(messageType), message(msg) {}
};

class CompileLog;

class CompileLogStream {
    public: /* Methods: */
        inline CompileLogStream(const CompileLogStream &copy)
            : m_log(copy.m_log), m_type(copy.m_type)
        {
            assert(copy.m_os.str().empty());
        }
        inline CompileLogStream(CompileLog &log,
                                CompileLogMessage::Type messageType)
            : m_log(log), m_type(messageType) {}
        inline ~CompileLogStream();

        inline CompileLogStream &operator<<(bool v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(char v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(signed short v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(unsigned short v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(signed int v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(unsigned int v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(signed long v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(unsigned long v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(float v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(double v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(long double v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(const std::string &v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(const char *v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(const void *v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(const YYLTYPE &v) { m_os << v; return *this; }
        inline CompileLogStream &operator<<(const Type &v) { m_os << v; return *this; }

    CompileLog             &m_log;
    std::ostringstream      m_os;
    CompileLogMessage::Type m_type;
};

class CompileLog {
    public: /* Types: */

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
    typedef std::deque<CLM>::const_iterator MDCI;

    for (MDCI it(log.messages().begin()); it != log.messages().end(); it++) {
        switch ((*it).type) {
            case CLM::Fatal:   out << "[FATAL] "; break;
            case CLM::Error:   out << "[ERROR] "; break;
            case CLM::Warning: out << "[WARN ] "; break;
            case CLM::Info:    out << "[INFO ] "; break;
            case CLM::Debug:   out << "[DEBUG] "; break;
        }
        out << (*it).message << std::endl;
    }
    return out;
}

#endif // LOG_H
