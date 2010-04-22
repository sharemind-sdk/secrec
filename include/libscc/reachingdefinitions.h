#ifndef REACHINGDEFINITIONS_H
#define REACHINGDEFINITIONS_H

#include <map>


namespace SecreC {

class Symbol;
class ICodeList;
class Imop;

class ReachingDefinition {
    public: /* Types: */
        enum Type { DIRECT, JUMP, CONDJUMP };

    public: /* Methods: */
        inline ReachingDefinition(Type type) : m_type(type) {}

        inline Type type() const { return m_type; }

    private: /* Fields: */
        Type m_type;
};

class ReachingDefinitionDirect: public ReachingDefinition {
    public: /* Methods: */
        inline ReachingDefinitionDirect(const Imop *def)
            : ReachingDefinition(ReachingDefinition::DIRECT), m_def(def) {}

        inline const Imop *def() const { return m_def; }

    private: /* Fields: */
        const Imop *m_def;
};

class ReachingDefinitionJump: public ReachingDefinition {
    public: /* Methods: */
        inline ReachingDefinitionJump(ReachingDefinition *reaching)
            : ReachingDefinition(ReachingDefinition::JUMP), m_reaching(reaching) {}

        inline ReachingDefinition *reaching() const { return m_reaching; }

    private: /* Fields: */
        ReachingDefinition *m_reaching;
};


class ReachingDefinitionCondJump: public ReachingDefinition {
    public: /* Methods: */
        inline ReachingDefinitionCondJump(ReachingDefinition *reaching)
            : ReachingDefinition(ReachingDefinition::CONDJUMP), m_reaching(reaching) {}

        inline ReachingDefinition *reaching() const { return m_reaching; }

    private: /* Fields: */
        ReachingDefinition *m_reaching;
};

class ReachingDefinitions {
    public: /* Types: */
        typedef std::map<const Symbol*, const Imop*> RMapping;
        typedef std::map<const Imop*, RMapping> Mapping;

    public: /* Methods: */
        ReachingDefinitions(const ICodeList &code);
        void run();

    protected: /* Methods: */
        bool step(const Imop *i);

    private: /* Fields: */
        Mapping m_ins;
        const ICodeList &m_code;
};

} // namespace SecreC


#endif // REACHINGDEFINITIONS_H
