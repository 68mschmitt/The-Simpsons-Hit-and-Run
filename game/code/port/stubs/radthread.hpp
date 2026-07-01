// Minimal RAD thread shim for the Linux PoC.
#ifndef RADTHREAD_HPP
#define RADTHREAD_HPP

struct IRadThreadLocalStorage
{
    virtual void Release() { delete this; }

protected:
    virtual ~IRadThreadLocalStorage() = default;
};

#endif // RADTHREAD_HPP
