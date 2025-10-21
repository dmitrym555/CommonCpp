//
// Conf file wrapper
// Author: Dmitry Melnik
//

#ifndef KSCONF_H
#define KSCONF_H
#include "KSConfParser.h"

#include <string>
#include <map>


class KSConf {
    KSConfParser& m_conf = *KSConfParser::inst();
    std::map<std::string, std::string> clargs;
public:
    void init(int argc, char *argv[], const char* confFile=nullptr );
    std::string get( const char* name, const char* defVal=nullptr );
};


extern KSConf g_conf;

#endif
