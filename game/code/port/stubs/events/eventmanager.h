// Minimal EventManager shim for the Linux PoC.
#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

class EventListener;

class EventManager
{
public:
    void RemoveAll(EventListener*)
    {
    }
};

inline EventManager* GetEventManager()
{
    static EventManager manager;
    return &manager;
}

#endif // EVENTMANAGER_H
