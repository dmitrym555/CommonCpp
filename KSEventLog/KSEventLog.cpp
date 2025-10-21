//
// Event Log
// Author: Dmitry Melnik
//

#include "KSEventLog.h"

#include <stdio.h>

#include <iostream>
#include <stdarg.h>
#include <string>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
//#include <format>
#include <fstream>

#include "../strfmt.h"

#include <mutex>

#include "../KSUtils/KSUtil.h"

#include "../KSDateTime/KSDateTime.h"

#include <filesystem>

#define RED_TEXT \033[0;31m
#define GREEN_BG \033[42m
#define RESET \033[0m

#include "../Platform/Platform.h"

class KSEventLogImpl : public KSEventLog {

    std::string m_lastMsg;
    int m_lastMsgCount;
    int m_msgSuffixLen;
    int m_dbgLevel = 10;
    std::string m_logDir;
    std::vector<std::string> m_excludes;
    std::string m_filter;
    std::mutex m_mutex;
    FILE* m_logf = nullptr;

    void Msg( const std::string& m );

    void logWrite( const std::string& m);

public:
    virtual void I(const char* fmt, ... ) override;

    virtual void D1(const char* fmt, ... ) override;
    virtual void D(int level, const char* fmt, ... ) override;
    virtual void W(const char* fmt, ... ) override;
    virtual void E(const char* fmt, ... ) override;

    virtual void D1( const std::string& fmt ) override;
    virtual void D(int level, const std::string& fmt ) override;
    virtual void W( const std::string& fmt ) override;
    virtual void E( const std::string& fmt ) override;
    virtual void I( const std::string& fmt ) override;

    virtual void setLevel( int level) override;
    virtual int getLevel() override { return m_dbgLevel; };
    virtual void setExcludes( const std::string s) override;
    virtual void setFilter( const std::string s) override;

    virtual void startFileLogging( const std::string& logDir ) override;

    KSEventLogImpl();
    ~KSEventLogImpl();
};

KSEventLog& Log() {
    static KSEventLogImpl inst;
    return inst;
}

KSEventLogImpl::KSEventLogImpl()
{
    m_lastMsgCount = 0;
    m_msgSuffixLen = 0;
}

KSEventLogImpl::~KSEventLogImpl()
{
    if ( m_logf != nullptr )
        fclose( m_logf );
}

void KSEventLogImpl::setLevel( int level) {
    m_dbgLevel = level;
    Log().I( strfmt( "LogLevel: %d", level ) );
}


void KSEventLogImpl::setExcludes( const std::string s) {
    m_excludes = split<char>(s,"|");
}

void KSEventLogImpl::setFilter( const std::string s) {
    m_filter = s;
}


void KSEventLogImpl::I(const char* fmt, ... ) {
    const char* inp = ( fmt == nullptr )? "" : fmt;
    char buf[512];
    va_list args;
    va_start (args, inp);
    vsnprintf (buf,sizeof(buf),inp, args);
    va_end (args);
    I( (std::string)buf );
}

void KSEventLogImpl::D1(const char* fmt, ... ) {
    if ( m_dbgLevel < 1 )
        return;
    char buf[512];
    va_list args;
    va_start (args, fmt);
    vsnprintf (buf,sizeof(buf),fmt, args);
    va_end (args);
    D1( (std::string)buf );
}

void KSEventLogImpl::D(int level, const char* fmt, ... ) {
    if ( m_dbgLevel < level )
        return;
    char buf[512];
    va_list args;
    va_start (args, fmt);
    vsnprintf (buf,sizeof(buf),fmt, args);
    va_end (args);
    D(level, (std::string)buf );
}

void KSEventLogImpl::W(const char* fmt, ... ) {
    char buf[512];
    va_list args;
    va_start (args, fmt);
    vsnprintf (buf,sizeof(buf),fmt, args);
    va_end (args);
    W( (std::string)buf );
}

void KSEventLogImpl::E(const char* fmt, ... ) {
    char buf[512];
    va_list args;
    va_start (args, fmt);
    vsnprintf (buf,sizeof(buf),fmt, args);
    va_end (args);
    E( (std::string)buf );
}


void KSEventLogImpl::startFileLogging( const std::string& logDir ) {
    if ( logDir.length() == 0 )
        return;
    m_logDir = logDir;

    std::filesystem::create_directories( logDir );

    std::string time = isodatetime( KSTimeNow() );

    std::string logPath = std::format( "{}{}{}.log", logDir, CLpathDelimiter, time );

    m_logf = fopen( logPath.c_str(), "a" );
    if ( m_logf == nullptr ) {
        E( std::format( "{}::{} could not open file for logging {}", KSMETHOD, logPath ) );
    }
    else {
        I( std::format( "{}::{} logPath {}", KSMETHOD, logPath ) );
    }
}


void KSEventLogImpl::logWrite(const std::string& m ) {

    if ( m_logf == nullptr ) {
        return;
    }

    fwrite( m.c_str(), 1, m.length(), m_logf );
    if ( m.length() > 1 )
        fflush(m_logf);
}

void KSEventLogImpl::Msg( const std::string& m ) {
    std::lock_guard<std::mutex> lg(m_mutex);
    if (m_filter.length() && m.find(m_filter) == std::string::npos )
        return;
    if ( m_excludes.size() ) {
        for ( const std::string& s : m_excludes) {
            if ( m.find(s) != std::string::npos )
                return;
        }
    }
    bool isSameMessage = ( m == m_lastMsg );
    if ( isSameMessage ) {
        int backs = (++m_lastMsgCount == 2)? 0: m_msgSuffixLen;
        std::string msgSuffix = " (" + std::to_string(m_lastMsgCount) + ")";
        m_msgSuffixLen = msgSuffix.length();
        std::cout << std::string(backs, '\b');
        std::cout << msgSuffix << std::flush;
        logWrite( "." );
    }
    else {
        m_lastMsg = m;
        m_lastMsgCount = 1;
        //std::wcout << utf8(m);
        std::string str = m;
        size_t p = str.find(": ");
        if ( p == std::string::npos )
            p = 0;
        str.insert(p+2, shortdatetime() + " " );
        std::cout << str << std::flush;
        logWrite( str );
    }
}

void KSEventLogImpl::D1( const std::string& m ) {
    if ( m_dbgLevel == 0 )
        return;
    Msg( "\nD1: " +  m );
}

void KSEventLogImpl::D(int level, const std::string& m ) {
    if ( m_dbgLevel < level )
        return;
    std::string mprefix = "\nD1: ";
    mprefix[2] = '0' + level;
    Msg( mprefix +  m );
}

void KSEventLogImpl::W( const std::string& m ) {
    Msg( "\nW : \033[33m" +  m + "\033[0m" );
}

void KSEventLogImpl::E( const std::string& m ) {
    Msg( "\nE : \033[37;41m" +  m + "\033[0m" );
}

void KSEventLogImpl::I( const std::string& m ) {
    Msg( "\nI : \033[32m" +  m  + "\033[0m" );
}




