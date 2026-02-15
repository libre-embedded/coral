#pragma once

namespace Coral
{

template <class T> class ContextLock
{
  public:
    ContextLock()
    {
        static_cast<T *>(this)->lock();
    }

    ~ContextLock()
    {
        static_cast<T *>(this)->unlock();
    }
};

class NoopLock : public ContextLock<NoopLock>
{
  public:
    inline void lock(void)
    {
        ;
    }

    inline void unlock(void)
    {
        ;
    }
};

} // namespace Coral
