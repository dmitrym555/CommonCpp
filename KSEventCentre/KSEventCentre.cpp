//
// Data access server project
// Author: Dmitry Melnik
//

#include "KSEventCentre.h"

#include <mutex>

#include <list>
#include <unordered_map>
#include <memory>
#include <utility>

#include "../../CommonCpp/KSEventLog/KSEventLog.h"
#include <format>

static std::recursive_mutex s_ecmutex;

static uint64_t s_esubsId = 1;

void KSEventSubscriber::initNew() {
    std::lock_guard<std::recursive_mutex> lg( s_ecmutex );
    id = s_esubsId++;
}


class KSEventCentreImpl : public KSEventCentre {

    std::unordered_map<int, std::list<KSEventSubscriber> > m_pool;
    void unbindInt( int event, KSEventSubscriber* psubs=nullptr );
public:
    virtual void bind( int event, KSEventSubscriber& subs ) override;
    virtual void unbind( KSEventSubscriber* psubs, int event = -1 ) override;
    virtual void post( int event, void* pcontext ) override;
};


void KSEventCentreImpl::bind( int event, KSEventSubscriber& subs ) {
    std::lock_guard<std::recursive_mutex> lg( s_ecmutex );
    m_pool[event].push_back( subs );
}


void KSEventCentreImpl::unbindInt( int event, KSEventSubscriber* psubs ) {
    if ( m_pool.count( event ) ) {
        if ( !psubs ) {
            m_pool.erase( event );
        }
        else {
            std::list<KSEventSubscriber>& subscribers = m_pool[event];
            for ( std::list<KSEventSubscriber>::iterator iii = subscribers.begin(); iii != subscribers.end(); ++iii ) {
                if ( iii->id == psubs->id ) {
                    subscribers.erase( iii );
                    break;
                }
            }
        }
    }
}

void KSEventCentreImpl::unbind( KSEventSubscriber* psub, int event ) {
    std::lock_guard<std::recursive_mutex> lg( s_ecmutex );
    if ( event > -1 ) {
        unbindInt( event, psub );
    }
    else {
        for ( auto& [evtId, Subs] : m_pool ) {
            unbindInt(evtId, psub );
        }
    }
}

void KSEventCentreImpl::post( int event, void* pcontext ) {
    std::lock_guard<std::recursive_mutex> lg( s_ecmutex );
    if ( m_pool.count(event) == 0 )
        return;
    auto subscribers = m_pool[event];
//    if ( 1 == event ) { // KSEVENT_PARAMCHANGED 1
//        int sz = subscribers.size();
//        Log().I( std::format( "KSEVENT_PARAMCHANGED subs count: {}", sz ) );
//    }
    for ( KSEventSubscriber subs : subscribers ) {
        subs.func(event, pcontext);
    }
}

KSEventCentre& ECentre() {
    std::lock_guard<std::recursive_mutex> lg( s_ecmutex );
    static KSEventCentreImpl inst;
    return inst;
}
