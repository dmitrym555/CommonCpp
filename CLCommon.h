#ifndef CLCOMMON_H
#define CLCOMMON_H

#include "../CommonCpp/KSEventLog/KSEventLog.h"

#include <string>


class KSMain {
public:
    ~KSMain() {
        Log().I("Main exit\n");
    }
};

class CLCommon {
public:
    bool mainTerminate;
    std::string binPath;

    void CLMain();
};

void CLInit();

extern CLCommon clCommon;


void sig_handler(int s);



#endif
