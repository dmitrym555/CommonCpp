//
// Conf file wrapper
// Author: Dmitry Melnik
//

#ifndef KSCONFPARSER_H
#define KSCONFPARSER_H

#include <string>

class KSConfParser
{
public:
    virtual void init( const std::string& confPath )=0;
    virtual std::string get( const char* name, const char* defaultVal = nullptr )=0;
    virtual std::string getFromSection( const char* section, const char* name, const char* defaultVal )=0;
    virtual void save()=0;
    virtual void set( const char* name, const char* val )=0;
    virtual void set( const char* name, const std::string& val )=0;

    static KSConfParser* inst();
    virtual ~KSConfParser() {}
};


#endif // KSCONFPARSER_H




