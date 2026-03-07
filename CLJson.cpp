
#include "CLJson.h"

#include "jsonBox/JsonBox.h"


int CLJsonVal::initFromString( const std::string& json ) {
    JsonBox::Value* jbv = new JsonBox::Value();
    jbv->loadFromString( json );
    setObject( jbv );
    return 0;
}

int CLJsonVal::initFromFile( const std::string& file ) {
    JsonBox::Value* jbv = new JsonBox::Value();
    jbv->loadFromFile( file );
    setObject( jbv );
    return 0;
}

int CLJsonVal::getEl( const std::string& name, CLJsonVal& jval ) {
    JsonBox::Value& jbv = *(JsonBox::Value*)m_jv;

    JsonBox::Value* jbv2 = new JsonBox::Value();
    *jbv2 = jbv[ name ];

    jval.setObject( jbv2 );
    return 0;
}

int CLJsonVal::getEl( size_t index, CLJsonVal& jval ) {
    JsonBox::Value* jbv2 = new JsonBox::Value( ((JsonBox::Value*)m_jv)->getArray()[index] );
    jval.setObject( jbv2 );
    return 0;
}

size_t CLJsonVal::getSize() {
    size_t res = ((JsonBox::Value*)m_jv)->getArray().size();
    return res;
}


std::string CLJsonVal::getString(const std::string& key) {
    JsonBox::Value& jbv = *(JsonBox::Value*)m_jv;
    return jbv[key].getString();
}

int CLJsonVal::getInt(const std::string& key) {
    JsonBox::Value& jbv = *(JsonBox::Value*)m_jv;
    return jbv[key].getInteger();
}

double CLJsonVal::getDouble( const std::string& key ) {
    JsonBox::Value& jbv = *(JsonBox::Value*)m_jv;
    double res = jbv[key].getDouble();
    return res;
}


void CLJsonVal::setObject(void* ptr) {
    m_jv = ptr;
}


CLJsonVal::~CLJsonVal() {
    if ( m_jv )
        delete (JsonBox::Value*)m_jv;
}
