#ifndef KSEVENTCENTRE_H
#define KSEVENTCENTRE_H

#include <functional>
#include <cstdint>


typedef std::function<void(int, void*)>  KSEventFunc;

struct KSEventSubscriber {
    uint64_t id;
    KSEventFunc func;

    void initNew();
};

class KSEventCentre
{
public:
    virtual void bind( int event, KSEventSubscriber& subs )=0;
    virtual void unbind( KSEventSubscriber* subs, int event = -1 )=0;
    virtual void post( int event, void* pcontext )=0;
};

KSEventCentre& ECentre();

#endif // KSEVENTCENTRE_H
