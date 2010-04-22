#ifndef REACHINGDEFINITIONS_H
#define REACHINGDEFINITIONS_H

#include <cassert>
#include <map>
#include <set>


namespace SecreC {

class Symbol;
class ICodeList;
class Imop;

class ReachingDefinition {
    public: /* Types: */
        enum Type { DIRECT, CONDJUMP };

    public: /* Methods: */
        inline ReachingDefinition(Type type) : m_type(type) {}

        inline Type type() const { return m_type; }
        virtual inline bool operator==(const ReachingDefinition &o)
                const
        {
            return m_type == o.m_type;
        }

    private: /* Fields: */
        Type m_type;
};

class ReachingDefinitionDirect: public ReachingDefinition {
    public: /* Methods: */
        inline ReachingDefinitionDirect(const Imop *def)
            : ReachingDefinition(ReachingDefinition::DIRECT), m_def(def) {}

        inline const Imop *def() const { return m_def; }
        inline bool operator==(const ReachingDefinition &o) const {
            typedef ReachingDefinitionDirect RDD;
            if (!ReachingDefinition::operator==(o)) return false;
            assert(dynamic_cast<const RDD*>(&o) != 0);
            return static_cast<const RDD&>(o).m_def == m_def;
        }

    private: /* Fields: */
        const Imop *m_def;
};

class ReachingDefinitionCondJump: public ReachingDefinition {
    public: /* Methods: */
        inline ReachingDefinitionCondJump(const ReachingDefinition &reaching,
                                          const Imop *condJump)
            : ReachingDefinition(ReachingDefinition::CONDJUMP),
              m_condJump(condJump), m_reaching(reaching) {}


        inline const ReachingDefinition &reaching() const {
            return m_reaching;
        }
        inline bool operator==(const ReachingDefinition &o) const {
            typedef ReachingDefinitionCondJump RDCJ;
            if (!ReachingDefinition::operator==(o)) return false;
            assert(dynamic_cast<const RDCJ*>(&o) != 0);
            return static_cast<const RDCJ&>(o).m_reaching == m_reaching;
        }

    private: /* Fields: */
        const Imop              *m_condJump;
        const ReachingDefinition m_reaching;
};

class ReachingDefinitions {
    public: /* Types: */
        typedef std::set<ReachingDefinition*> Defs;
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
