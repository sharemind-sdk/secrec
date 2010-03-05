#ifndef SCCREFOBJ_H
#define SCCREFOBJ_H

#include <set>


class SccRefObjPointer;

class SccRefObj {
    friend class SccRefObjPointer;
    public:
        SccRefObj()
            : m_refCount(0) {}

        ~SccRefObj();

    private:
        void deref(SccRefObjPointer *pointer) const {
            m_refCount--;
            m_pointers.erase(pointer);

            if (m_refCount <= 0) delete this;
        }

        void ref(SccRefObjPointer *pointer) const {
            m_refCount++;
            m_pointers.insert(pointer);
        }

    private:
        mutable unsigned m_refCount;
        mutable std::set<SccRefObjPointer*> m_pointers;
};

class SccRefObjPointer: public SccRefObj {
    friend class SccRefObj;
    public:
        SccRefObjPointer(SccRefObj *obj)
            : m_obj(obj)
        {
            obj->ref(this);
        }

        SccRefObjPointer(SccRefObj &obj)
            : m_obj(&obj)
        {
            obj.ref(this);
        }

        ~SccRefObjPointer() {
            if (m_obj != 0) m_obj->deref(this);
        }


        SccRefObjPointer &operator=(SccRefObj *obj) {
            if (m_obj != obj) {
                if (m_obj != 0) m_obj->deref(this);
                m_obj = obj;
                obj->ref(this);
            }
            return *this;
        }

        inline SccRefObjPointer &operator=(const SccRefObjPointer &other) {
            return operator =(other.data());
        }

        inline SccRefObj *data() const { return m_obj; }
        inline operator SccRefObj*() const { return m_obj; }
        inline SccRefObj &operator*() const { return *m_obj; }
        inline SccRefObj *operator->() const { return m_obj; }

    private:
        SccRefObj *m_obj;
};

SccRefObj::~SccRefObj() {
    typedef std::set<SccRefObjPointer*>::iterator SOI;
    for (SOI it(m_pointers.begin()); it != m_pointers.end(); it++) {
        (*it)->m_obj = 0;
    }
}

#endif // SCCREFOBJ_H
