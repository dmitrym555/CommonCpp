//
// C++ common units
// Author: Dmitry Melnik
//

#ifndef KSSHAREDMEMORY_H
#define KSSHAREDMEMORY_H

#include <string>

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <format>

#include "KSEventLog/KSEventLog.h"

template <class T>
class KSSharedMemory {
    int m_fd = -1;
    bool m_persist = false;
    T* m_mem;
    std::string m_name;
    T* openInt();
public:
    T* create( const std::string& aname );
    void shmclose();
    T* open( const std::string& aname, bool persist = false );
    T* get() { return m_mem; };
    T* obtain( const std::string& aname, bool& created, bool persist );
    ~KSSharedMemory();
};

template <class T>
KSSharedMemory<T>::~KSSharedMemory() {
    shmclose();
}

template <class T>
void KSSharedMemory<T>::shmclose() {
    if ( m_fd != -1 )
        close( m_fd );
    m_fd = -1;
    if ( !m_persist )
        shm_unlink( m_name.c_str() );
}

template <class T>
T* KSSharedMemory<T>::openInt() {
    m_mem = (T*)mmap( NULL, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0 );
    if (m_mem == MAP_FAILED) {
        Log().E( std::format( "{}::{} Could not mmap shared memory object {}", KSMETHOD, m_name ) );
        return nullptr;
    }
    return m_mem;
}

template <class T>
T* KSSharedMemory<T>::open( const std::string& aname, bool persist ) {
    m_persist = persist;
    m_name = aname;
    m_fd = shm_open( aname.c_str(), O_RDWR, 0 );
    if ( m_fd == -1 ) {
        Log().E( std::format( "{}::{} Could not open shared memory object {}",KSMETHOD, m_name ) );
        return nullptr;
    }
    return openInt();
}

template <class T>
T* KSSharedMemory<T>::create( const std::string& aname ) {
    m_name = aname;
    m_fd = shm_open( m_name.c_str(), O_CREAT | O_EXCL | O_RDWR, 0600 );
    if ( m_fd == -1 ) {
        Log().E( std::format( "{}::{} Could not create shared memory object {}",KSMETHOD, m_name ) );
        return nullptr;
    }
    if ( -1 == ftruncate( m_fd, sizeof(T) ) ) {
        Log().E( std::format( "{}::{} Could not truncate shared memory object {}",KSMETHOD, m_name ) );
        return nullptr;
    }
    return openInt();
}

template <class T>
T* KSSharedMemory<T>::obtain( const std::string& aname, bool& created, bool persist ) {
    T* res = open( aname, persist );
    created = false;
    if ( !res ) {
        res = create( aname );
        created = true;
    }
    return res;
}


#endif // KSSHAREDMEMORY_H
