#ifndef SCCREFOBJ_H
#define SCCREFOBJ_H

#include <set>

class SccObject {
    template <class T>
    friend class SccPointer;

    public:
        SccObject()
            : m_refCount(0), m_garbageCollect(false) {}

        virtual ~SccObject() {
            typedef std::set<SccObject**>::iterator SOI;
            for (SOI it(m_pointers.begin()); it != m_pointers.end(); it++)
                (**it) = 0;
        }

        inline bool autoGarbageCollect() const { return m_garbageCollect; }
        inline void setAutoGarbageCollect(bool gc) { m_garbageCollect = gc; }

    private:
        void deref(SccObject **pointer) const {
            m_refCount--;
            m_pointers.erase(pointer);

            if (m_refCount <= 0 && m_garbageCollect) {
                delete this;
            }
        }

        void ref(SccObject **pointer) const {
            m_refCount++;
            m_pointers.insert(pointer);
        }

    private:
        mutable unsigned m_refCount;
        mutable std::set<SccObject**> m_pointers;
        bool m_garbageCollect;
};

#endif // SCCREFOBJ_H
