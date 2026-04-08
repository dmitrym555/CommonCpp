//
// Author: Dmitry Melnik
//

#include "CLUdpClient.h"

#include "../../CommonCpp/KSUtils/KSUtil.h"

#include <sys/types.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "../../CommonCpp/Platform/Platform.h"
#include "../../CommonCpp/KSEventLog/KSEventLog.h"



bool CLApiJsonRequest::validate(std::string* message) {

    if ( 0 != memcmp( &apih.signature, CLAPISignature, sizeof(CLAPISignature)-1 ) ) {
        if ( message != nullptr ) {
            *message = std::format( "{}::{} Signature is missing", KSMETHOD );
            return false;
        }
    }

    if ( query != QUERY_JSON ) {
        if ( message != nullptr ) {
            *message = std::format( "{}::{} query is not QUERY_JSON", KSMETHOD );
            return false;
        }
    }

    if ( 0 != memcmp( &cljq.signature, CLApiJsonRequestSignature, sizeof(CLApiJsonRequestSignature)-1 ) ) {
        if ( message != nullptr ) {
            *message = std::format( "{}::{} Signature KSJQ is missing", KSMETHOD );
            return false;
        }
    }

    return true;
}


void CLApiJsonRequest::getJson( std::string& json ) {
    byte* jsonptr = (byte*)&cljq + sizeof(cljq);
    json.resize( cljq.strLen );
    memcpy( json.data(), jsonptr, cljq.strLen );
}

int CLApiJsonRequest::setJson( const std::string& json ) {
    int ires = cljq.setJson(json) + sizeof(CLApiJsonRequest) - sizeof(CLApiStringHeader);
    return ires;
}



int CLApiStringHeader::setJson( const std::string& json ) {
    byte* pdata = (byte*)this + sizeof(CLApiStringHeader);
    memcpy( pdata, json.data(), json.length() );
    strLen = json.length();
    strCrc = DWCalcCrc( json.data(), json.length() );
    int res = json.length() + sizeof(CLApiStringHeader);
    return res;
}



std::string CLUdpClient::request( const CLUdpRequest& rargs ) {

    int sock = CLMakeSocket(CLSocketType::udp );
    if ( sock < 0 ) {
        return "{}";
    }

    std::string port;
    std::string saddress = keyval( rargs.address, ":", port );

    int apiPort = KSStrToInt( port, 21923 );

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( apiPort );
    addr.sin_addr.s_addr = inet_addr( saddress.c_str() );

    char buf[ 4096 * 16 ];
    memset( buf, 0, sizeof( buf) );

    CLApiJsonRequest& jr = *(CLApiJsonRequest*)buf;
    jr.init();
    int reqsize = jr.setJson( rargs.body );

    //memcpy( buf, rargs.body.data(), rargs.body.length() );
    //int reqsize = rargs.body.length();

    socklen_t clilen = sizeof( struct sockaddr);

    int ires = sendto( sock, buf, reqsize, 0, (struct sockaddr*)&addr, clilen );
    if ( ires <= 0 ) {
        CLCloseSocket( sock );
        return "{}";
    }

    int ret = CLFdWait( sock, 2000 );

    std::string res;
    if ( ret > 0 ) {
        memset( buf, 0, sizeof( buf) );
        ires = recv( sock, buf, sizeof( buf ), 0 );
        if ( ires > 0 ) {
            res.resize( ires );
            memcpy( res.data(), buf, ires );
            //CLApiJsonRequest& ja = *(CLApiJsonRequest*)buf;
            //std::string error;
            //if ( ja.validate(&error) ) {
            //    ja.getJson( res );
            //}
            //else {
            //    res = strfmt( "{ \"error\":\"%s\"}", error.c_str() );
            //}
        }
    }

    CLCloseSocket( sock );
    return res;
}


CLUdpClient& UdpClient() {
    static CLUdpClient inst;
    return inst;
}





