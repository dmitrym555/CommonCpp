#ifndef CLJSON_H
#define CLJSON_H

#include <string>

class CLJsonVal
{
    void* m_jv = nullptr;
public:
    int initFromString( const std::string& json );
    int initFromFile( const std::string& file );
    int getEl( const std::string& name, CLJsonVal& jval );
    int getEl( size_t index, CLJsonVal& jval );
    size_t getSize();
    std::string getString( const std::string& key );
    int getInt( const std::string& key );
    double getDouble( const std::string& key );

    void setObject(void* ptr);

    ~CLJsonVal();
};




#endif // CLJSON_H
