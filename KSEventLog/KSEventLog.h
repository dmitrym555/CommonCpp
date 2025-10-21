//
// Event Log
// Author: Dmitry Melnik
//

#ifndef KSEVENTLOG_H
#define KSEVENTLOG_H

#include <string>


#define KSMETHOD typeid(*this).name(), __func__

#define KSMETHODS "class", __func__

#define KSFUNC __func__


class KSEventLog
{
public:
    virtual void I(const char* fmt=nullptr, ... )=0;
    virtual void D1(const char* fmt, ... )=0;
    virtual void D(int level, const char* fmt, ... )=0;
    virtual void W(const char* fmt, ... )=0;
    virtual void E(const char* fmt, ... )=0;

    virtual void D1( const std::string& fmt )=0;
    virtual void D(int level, const std::string& fmt )=0;
    virtual void W( const std::string& fmt )=0;
    virtual void E( const std::string& fmt )=0;
    virtual void I( const std::string& fmt )=0;
    virtual void setLevel( int level)=0;
    virtual int getLevel()=0;

    virtual void setExcludes( const std::string s)=0;
    virtual void setFilter( const std::string s)=0;

    virtual void startFileLogging( const std::string& path )=0;

    virtual ~KSEventLog() {}
};

KSEventLog& Log();


#endif // KSEVENTLOG_H
