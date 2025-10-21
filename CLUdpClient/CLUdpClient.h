#ifndef CLUDPCLIENT_H
#define CLUDPCLIENT_H

#include <string>
#include <cinttypes>
#include <cstring>

#include "../../CommonCpp/KSuperBasic.h"


const char CLAPISignature[] = "CLAPI";
const int posSignature = 11;
const int CLAPIVERSION = 1;
const char CLApiJsonRequestSignature[] = "CLJQ";
const int QUERY_JSON = 0x7000;


struct CLUdpRequest {
    std::string address;
    std::string body;
};



enum class CLApiRequestCmd { start = 1, next = 2, resend = 3, close = 4 };

struct CLRequestHeader {
    uint32_t fnubmer = 0;
    byte reserved[posSignature - sizeof(fnubmer)];
    uint32_t signature;
    word clientVersion = CLAPIVERSION;

    void init() {
        memcpy( &signature, CLAPISignature, sizeof(signature) );
    }
    CLRequestHeader() {
        init();
    }
};

struct KSDASRequestHeaderWithCmd : public CLRequestHeader
{
    word  query = 0;
};

struct CLApiStringHeader {
    dword  signature; // CLQJ signature
    dword  strCrc;
    dword  strLen;
    byte   reserved[32-12];

    void init() {
        memcpy( &signature, CLApiJsonRequestSignature, sizeof(signature) );
    }
    CLApiStringHeader() {
        init();
    }
    int setJson(const std::string& json);
};


struct CLApiJsonRequest {
    CLRequestHeader    apih;
    word               query;
    CLApiStringHeader  cljq;

    void init() {
        apih.init();
        query = QUERY_JSON;
        cljq.init();
    }

    bool validate(std::string* message=nullptr);
    void getJson(std::string& json);
    int setJson(const std::string& json);
};


class CLUdpClient
{
public:
	std::string request( const CLUdpRequest& rargs );
};

CLUdpClient& UdpClient();

#endif
