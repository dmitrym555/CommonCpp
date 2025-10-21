//
// C++ common units
// Author: Dmitry Melnik
//

#ifndef KSMEMUTILS_H
#define KSMEMUTILS_H

#include <cstring>

class KSMemBuf {
    byte* m_data;
    int64_t m_pointer;
    int64_t m_size;
    const int64_t chunkSize = 8192;
public:
    KSMemBuf() {
        m_pointer = 0;
        m_size = chunkSize;
        m_data = (byte*)malloc( m_size );
    }

    ~KSMemBuf() {
        free( m_data );
    }

    void reset() {
        m_pointer = 0;
    }

    int64_t getSize() { return m_pointer; }

    byte* getData() { return m_data; }

    int64_t add(const void* pbuf, int64_t len) {
        void* pdata = add( len );
        memcpy( pdata, pbuf, len );
        return getSize();
    }

    void* add( int64_t len ) {
        if ( m_pointer + len > m_size ) {
            m_size += ( ( (m_pointer + len) - m_size ) / chunkSize + 1 ) * chunkSize;
            m_data = (byte*)realloc( m_data, m_size );
        }
        void* res = m_data + m_pointer;
        m_pointer += len;
        return res;
    }

    void truncate( int64_t len ) {
        int64_t newSize = std::max( 0L, m_pointer - len );
        memcpy( m_data, m_data + len, newSize );
        m_pointer = newSize;
    }
};

#endif
