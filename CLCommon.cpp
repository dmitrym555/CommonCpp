
#include "CLCommon.h"

#include "Platform/Platform.h"
#include "KSUtils/KSUtil.h"

#include "KSEventLog/KSEventLog.h"


void CLInit() {

    clCommon.binPath = getBinPath();
    clCommon.binPath = KSExtractFilePath( clCommon.binPath );

    Log().startFileLogging( clCommon.binPath );
}

