//
// Conf file wrapper
// Author: Dmitry Melnik
//

#include "KSConf.h"
#include "../KSUtils/KSUtil.h"

#include "../KSEventLog/KSEventLog.h"


std::map<std::string, std::string> getClArgs( int argc, char* argv[] ) {
    std::map<std::string, std::string> res;
    for ( int i=1; i < argc; ++i ) {
        std::string line = argv[i];
        size_t pos = line.find('=');
        if ( pos == std::string::npos ) {
            Log().W( strfmt( "Unknown arg: %s", argv[i] ) );
            continue;
        }
        std::string argname = line.substr(0, pos);
        std::string argval = line.substr(pos+1);
        res[trim(argname)] = trim(argval);
    }
    return res;
}

void KSConf::init(int argc, char *argv[], const char* confFile ) {
    m_clargs = getClArgs( argc, argv );
    if (confFile)
        m_conf.init(confFile);
}

std::string KSConf::get( const char* name, const char* defVal ) {
    if ( m_clargs.count(name) != 0 )
        return m_clargs[name];
    return m_conf.get( name, defVal );
}

KSConf g_conf;


