
#include "CLCommon.h"

#include "Platform/Platform.h"
#include "KSUtils/KSUtil.h"

#include "KSEventLog/KSEventLog.h"

#include <signal.h>


void sig_handler(int s) {
    Log().I("Caught signal %d",s);
    clCommon.mainTerminate = true;
}


void CLInit(const std::string& unitName ) {

    clCommon.binPath = getBinPath();
    clCommon.binPath = KSExtractFilePath( clCommon.binPath );

    signal (SIGINT,sig_handler);
    signal (SIGTERM,sig_handler);

    Log().startFileLogging( std::format( "{}{}{}_logs", clCommon.binPath, psep(), unitName ) );
}

