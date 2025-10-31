//
// CommonCPP utils
// Author: Dmitry Melnik
//

#ifndef KSUTIL20_H
#define KSUTIL20_H


#include "KSUtil.h"

#include <string>

#include <iomanip>
#include <charconv>


template<class T>
int stringScan( const std::string& str, int searchStart, T& outVal, const char* delimiter=" " ) {
    std::string delim = delimiter? delimiter : " ";
    size_t pos = str.find( delim, searchStart );
    if ( pos != std::string::npos ) {
        std::from_chars( str.c_str() + searchStart, str.c_str() + searchStart + pos, outVal );
        return pos+delim.length();
    }
    std::from_chars( str.c_str() + searchStart, str.c_str() + str.length(), outVal );
    return str.length();
};

double KSStr2Double(const std::string& str );

template <typename T> T convert_to (const std::string& str) {
    if constexpr (std::is_same_v<T, std::string>) {
        return str;
    }
    else if constexpr ( std::is_same_v<T, bool> ) {
        bool bres = ( str == "1" || str == "true" || str == "True" || str == "TRUE" );
        return bres;
    }
    else if constexpr ( std::is_same_v<T, float> ) {
        return KSStr2Double(str);
    }
    else if constexpr ( std::is_same_v<T, double> ) {
        return KSStr2Double(str);
    }
    else {
        T res;
        int radix = str.starts_with("0x")? 16:10;
        res = (T)std::stol( str, 0, radix );
//        std::istringstream ss(str);
//        T num;
//        ss >> num;
        return res;
    }
}


template<class T1, class T2> T2 GetText( T1 obj, T2 defaultValue ) {
    if ( !obj )
        return defaultValue;

    T2 res;
    res = convert_to<T2>( obj->GetText() );
    return res;
}

template<class T1, class T2> T2 getXmlChildElValue( const T1* xmlel, const char* name, const T2 defaultValue )
{
    xmlel = xmlel->FirstChildElement(name);
    if ( !xmlel )
        return defaultValue;
        //throw std::runtime_error( std::format("xmlel not found: {0}", name ) );

    T2 res;
    std::string text = xmlel->GetText();
    res = convert_to<T2>( text );
    return res;
}


#endif // KSUTIL_H
