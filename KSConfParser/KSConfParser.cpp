//
// Conf file wrapper
// Author: Dmitry Melnik
//

#include "KSConfParser.h"

#include <map>
#include <cstdlib>
#include <fstream>

#include "../KSEventLog/KSEventLog.h"
#include "../KSUtils/KSUtil.h"

class KSConfParserImpl : public KSConfParser {
    std::string m_path;
    std::string m_section;
    std::map<std::string, std::string> m_conf;
    std::map<std::string, std::map<std::string, std::string> > m_sections;
    bool m_wasRead;
    void load();

public:
    virtual void init( const std::string& confPath ) override;
    virtual std::string get( const char* name, const char* defaultVal ) override;
    virtual void save() override;
    virtual void set( const char* name, const char* val ) override;
    virtual void set( const char* name, const std::string& val ) override;

    virtual std::string getFromSection( const char* section, const char* name, const char* defaultVal ) override;

    KSConfParserImpl();
    virtual ~KSConfParserImpl() {}
};

KSConfParser* KSConfParser::inst() {
    KSConfParser* res = new KSConfParserImpl();
    return res;
}

KSConfParserImpl::KSConfParserImpl() {
    m_wasRead = false;
}

void KSConfParserImpl::init( const std::string& confPath ) {
    m_wasRead = false;
    m_conf.clear();
    m_sections.clear();
    m_path = confPath;
}

void KSConfParserImpl::load() {
    if ( !m_path.length() )
        return;
    std::ifstream infile( m_path.c_str() );
    if (!infile.is_open()) {
        Log().W( std::format( "{}::{} Could not read conf file {}", KSMETHOD, m_path ) );
        return;
    }
    std::string line;
    while ( std::getline( infile, line) ) {
        line = trim( line );
        if ( line.size() == 0 )
            continue;
        if ( line[0] == '#' )
            continue;
        if ( line[0] == '[' ) {
            m_section = line;
            continue;
        }
        size_t pos = line.find('=');
        if ( pos == std::string::npos )
            continue;

        std::string argname = line.substr(0, pos);
        std::string argval = line.substr(pos+1);
        if ( m_section.length() )
            m_sections[m_section][trim(argname)] = trim(argval);
        else
            m_conf[trim(argname)] = trim(argval);
    }
}

void KSConfParserImpl::save() {
    FILE* f = fopen( m_path.c_str(), "w" );
    if ( !f ) {
        Log().W( strfmt("could not save conf file %s", m_path.c_str() ) );
        return;
    }
    for (auto const & [k, v] : m_conf) {
        std::string line = strfmt( "%s=%s\n", k.c_str(), v.c_str() );
        fprintf( f, line.c_str() );
    }
    fclose( f );
}

void KSConfParserImpl::set( const char* name, const char* val ) {
    set(name, (std::string)val );
}

void KSConfParserImpl::set( const char* name, const std::string& val ) {
    m_conf[name] = val;
}

std::string KSConfParserImpl::get( const char* name, const char* defaultVal ) {
    if ( !m_wasRead ) {
        load();
        m_wasRead = true;
    }

    if ( m_conf.count(name) == 0 )
        return  defaultVal? defaultVal : "";
    std::string res = m_conf[name];
    return res;
}


std::string KSConfParserImpl::getFromSection( const char* section, const char* name, const char* defaultVal ) {
    if ( !m_wasRead ) {
        load();
        m_wasRead = true;
    }

    std::string ssection = section? section : "";

    if ( m_sections.count(ssection) == 0 )
        return defaultVal? defaultVal : "";

    if ( m_sections[ssection].count(name) == 0 )
        return  defaultVal? defaultVal : "";

    std::string res = m_sections[ssection][name];
    return res;
}







