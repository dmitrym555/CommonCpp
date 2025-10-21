//
// Author: Dmitry Melnik
//

#include "SDDAPIStructs.h"

#include "KSEventLog/KSEventLog.h"

#include "KSUtils/KSUtil.h"

#include <format>


std::string STATE_PASSP::getStrVal() const {
    std::string res;
    switch (ValueType) {
        case ValueTypeAnalog:
            res = std::format( "{}", Value.fValue );
            break;
        case ValueTypeDiscret:
            res = std::format( "{}", Value.bValue );
            break;
        case ValueTypeInt32:
            res = std::format( "{}", Value.iValue );
            break;
        case ValueTypeString: {
            const char* ps = (const char*)this + sizeof(STATE_PASSP);
            res.resize(Value.iValue);
            memcpy( res.data(), ps, Value.iValue );
            break;
        }
        default: {
            Log().W( std::format("{}::{} Unhandled value type {}", KSMETHOD, ValueType ) );
            break;
        }
    }
    return res;
}


bool KSApiJsonRequest::validate(std::string* message) {

    if ( 0 != memcmp( &dasapih.signature, KSDASAPISignature, sizeof(KSDASAPISignature)-1 ) ) {
        if ( message != nullptr ) {
            *message = std::format( "{}::{} Signature SDDv is missing", KSMETHOD );
            return false;
        }
    }

    if ( query != QUERY_JSON ) {
        if ( message != nullptr ) {
            *message = std::format( "{}::{} query is not QUERY_JSON", KSMETHOD );
            return false;
        }
    }

    if ( 0 != memcmp( &ksjq.signature, KSApiJsonRequestSignature, sizeof(KSApiJsonRequestSignature)-1 ) ) {
        if ( message != nullptr ) {
            *message = std::format( "{}::{} Signature KSJQ is missing", KSMETHOD );
            return false;
        }
    }


    return true;
}


void KSApiJsonRequest::getJson(std::string& json) {
    byte* jsonptr = (byte*)&ksjq + sizeof(ksjq);
    json.resize( ksjq.strLen );
    memcpy( json.data(), jsonptr, ksjq.strLen );
}

int KSApiStringHeader::setJson(const std::string& json) {
    byte* pdata = (byte*)this + sizeof(KSApiStringHeader);
    memcpy( pdata, json.data(), json.length() );
    strLen = json.length();
    strCrc = DWCalcCrc( json.data(), json.length() );
    int res = json.length() + sizeof(KSApiStringHeader);
    return res;
}

int KSApiJsonRequest::setJson(const std::string& json) {
    int ires = ksjq.setJson(json) + sizeof(KSApiJsonRequest) - sizeof(KSApiStringHeader);
    return ires;
}
