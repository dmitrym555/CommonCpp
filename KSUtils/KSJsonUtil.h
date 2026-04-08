#ifndef KSJSONUTIL_H
#define KSJSONUTIL_H

#include <string>

#include "../jsonBox/JsonBox.h"


template<class T>
T KSJsonGet(JsonBox::Value& jval, std::string& propName, T defVal ) {
    if ( jval[propName].isNull() )
        return defVal;
    return jval[propName];
}


#endif // KSJSONUTIL_H
