#ifndef REACHINGDEFINITIONS_H
#define REACHINGDEFINITIONS_H

#include <map>
#include <set>


namespace SecreC {

class Block;
class Symbol;
class ICode;
class Imop;

class ReachingDefinitions {
    public: /* Types: */
        typedef std::set<const Imop*>        Defs;
        typedef std::map<const Symbol*,Defs> SDefs;
        typedef std::map<const Block*,SDefs> BDM;

    public: /* Methods: */
        ReachingDefinitions(const ICode &code);
        void run();

    private: /* Methods: */
        bool makeOuts(const Block &i, const SDefs &in, SDefs &out);

    private: /* Fields: */
        const ICode &m_code;
        BDM          m_ins;
};

} // namespace SecreC


#endif // REACHINGDEFINITIONS_H
