#ifndef SCCPOINTER_H
#define SCCPOINTER_H

#include "sccobject.h"

template <class T>
class SccPointer {
    public:
        SccPointer()
            : m_obj(0) {}

        SccPointer(T *obj)
            : m_obj(obj)
        {
            obj->ref(&m_obj);
        }

        SccPointer(const SccPointer<T> &other)
            : m_obj(other.data())
        {
            m_obj->ref(&m_obj);
        }

        ~SccPointer() {
            if (m_obj != 0)
                m_obj->deref(&m_obj);
        }

        SccPointer &operator=(T *obj) {
            if (m_obj != obj) {
                if (m_obj != 0)
                    m_obj->deref(&m_obj);

                m_obj = obj;

                if (obj != 0)
                    obj->ref(&m_obj);
            }
            return *this;
        }

        inline SccPointer<T> &operator=(const SccPointer<T> &other) {
            return operator =(other.data());
        }

        inline bool isNull() const { return m_obj == 0; }
        inline T *data() const { return static_cast<T*>(m_obj); }
        inline operator T*() const { return static_cast<T*>(m_obj); }
        inline T &operator*() const { return *static_cast<T*>(m_obj); }
        inline T *operator->() const { return static_cast<T*>(m_obj); }
        inline bool operator==(T *o) const { return m_obj == o; }
        inline bool operator!=(T *o) const { return m_obj != o; }
        inline bool operator==(const T *o) const { return m_obj == o; }
        inline bool operator!=(const T *o) const { return m_obj != o; }
        inline bool operator==(const SccPointer<T> &p) const { return m_obj == p.m_obj; }
        inline bool operator!=(const SccPointer<T> &p) const { return m_obj != p.m_obj; }

    private:
        SccObject *m_obj;
};

template <class T>
inline bool operator==(T *o, const SccPointer<T> &p) { return o == p.data(); }

template <class T>
inline bool operator!=(T *o, const SccPointer<T> &p) { return o != p.data(); }

template <class T>
inline bool operator==(const T *o, const SccPointer<T> &p) { return o == p.data(); }

template <class T>
inline bool operator!=(const T *o, const SccPointer<T> &p) { return o != p.data(); }

#endif // SCCPOINTER_H
