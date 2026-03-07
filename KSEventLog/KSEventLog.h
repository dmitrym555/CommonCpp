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

#include <cinttypes>


#define DLE -2
#define DLW -1
#define DLI 0
#define DL1 1
#define DL2 2
#define DL3 3
#define DL4 4
#define DL5 5
#define DL6 6
#define DL7 7
#define DL8 8
#define DL9 9

class KSEventLog
{
public:
    virtual void I(const char* fmt=nullptr, ... )=0;
    virtual void D1(const char* fmt, ... )=0;
    virtual void D(int level, const char* fmt, ... )=0;
    virtual void W(const char* fmt, ... )=0;
    virtual void E(const char* fmt, ... )=0;

    virtual void L( int loglevel, uint32_t source, const std::string& fmt )=0;

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

    virtual void setDisabled( bool disable )=0;

    virtual ~KSEventLog() {}
};

KSEventLog& Log();


#endif // KSEVENTLOG_H
