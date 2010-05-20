#ifndef REACHINGDEFINITIONS_H
#define REACHINGDEFINITIONS_H

#include <cassert>
#include <map>
#include <ostream>
#include <set>

namespace SecreC {

class Block;
class Symbol;
class ICode;
class Imop;

class ReachingDefinitions {
    public: /* Types: */
        typedef std::set<const Imop*>         CJumps;
        typedef std::map<const Imop*, CJumps> Defs;
        typedef std::map<const Symbol*, Defs> SDefs;
        typedef std::map<const Block*, SDefs> BDM;

    public: /* Methods: */
        ReachingDefinitions(const ICode &code);
        void run();

        inline const ICode &icode() const { return m_code; }
        inline const SDefs &getReaching(const Block &b) const {
            assert(m_ins.find(&b) != m_ins.end());
            return (*m_ins.find(&b)).second;
        }

    private: /* Methods: */
        bool makeOuts(const Block &b, const SDefs &in, SDefs &out);

    private: /* Fields: */
        const ICode &m_code;
        BDM          m_ins;
};

} // namespace SecreC


std::ostream &operator<<(std::ostream &out, const SecreC::ReachingDefinitions &rd);

#endif // REACHINGDEFINITIONS_H
