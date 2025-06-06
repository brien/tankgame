//
//  Singleton.h
//  tankgame
//
//

#pragma once

#include <cassert>

template <typename T>
class Singleton
{
    static T *m_singleton;

protected:
    Singleton()
    {
        assert(!m_singleton);
        m_singleton = static_cast<T *>(this);
    }

    ~Singleton()
    {
        assert(m_singleton);
        m_singleton = 0;
    }

public:
    static inline void Create()
    {
        if (m_singleton)
            return;
        new T();
    }
    static inline T &GetSingleton()
    {
        assert(m_singleton);
        return *m_singleton;
    }
    static inline T *GetSingletonPtr()
    {
        assert(m_singleton);
        return m_singleton;
    }
    static void Destroy()
    {
        if (m_singleton)
        {
            delete m_singleton;
        }
        m_singleton = 0;
    }
};

template <typename T>
T *Singleton<T>::m_singleton = 0;
