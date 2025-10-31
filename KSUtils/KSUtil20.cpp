//
// CommonCPP utils
// Author: Dmitry Melnik
//

#include "KSUtil20.h"

#include <filesystem>


double KSStr2Double(const std::string& str ) {
    double res;
    size_t comapos = str.find( "," );
    if ( comapos != std::string::npos ) {
        std::string strcopy = str;
        strcopy[comapos] = '.';
        std::from_chars( strcopy.data(), strcopy.data() + strcopy.length(), res );
    }
    else {
        std::from_chars( str.data(), str.data() + str.length(), res );
    }
    return res;
}




