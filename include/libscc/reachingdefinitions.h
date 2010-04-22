#ifndef REACHINGDEFINITIONS_H
#define REACHINGDEFINITIONS_H

#include <set>
#include <vector>


namespace SecreC {

class Symbol;
class ICodeList;
class Imop;

class ReachingDefinitions {
    public: /* Methods: */
        ReachingDefinitions(const ICodeList &code);
        void run();

    private: /* Fields: */
        std::vector<std::set<Symbol*,Imop*> > m_defs;
        const ICodeList &m_code;
};

} // namespace SecreC


#endif // REACHINGDEFINITIONS_H
