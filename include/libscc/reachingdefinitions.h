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
        typedef std::set<const Imop*>           Defs;
        typedef std::set<const Imop*>           Jumps;
        typedef std::pair<Defs, Jumps>          SReach;
        typedef std::map<const Symbol*, SReach> SDefs;
        typedef std::map<const Block*, SDefs>   BDM;
        typedef std::map<const Block*, Jumps>   BJM;

    public: /* Methods: */
        ReachingDefinitions(const ICode &code);
        void run();

        inline const ICode &icode() const { return m_code; }
        inline const SDefs &getReaching(const Block &b) const {
            assert(m_ins.find(&b) != m_ins.end());
            return (*m_ins.find(&b)).second;
        }
        inline const BJM &getPosJumps() const { return m_inPos; }
        inline const BJM &getNegJumps() const { return m_inNeg; }

    private: /* Methods: */
        bool makeOuts(const Block &b, const SDefs &in, SDefs &out);

    private: /* Fields: */
        const ICode &m_code;
        BDM          m_ins;
        BJM          m_inPos;
        BJM          m_inNeg;
};

} // namespace SecreC


#endif // REACHINGDEFINITIONS_H
