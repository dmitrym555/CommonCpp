#ifndef KSCONNECTIONCB_H
#define KSCONNECTIONCB_H


class KSConnectionCB
{
public:
    virtual void processIncoming(const unsigned char* buf, int len)=0;
};

#endif // KSCONNECTIONCB_H
