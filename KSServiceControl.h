#ifndef KSSERVICECONTROL_H
#define KSSERVICECONTROL_H


class KSServiceControl
{
protected:
    bool m_started = false;
    bool m_stop = false;
    bool m_finished = true;
public:
    virtual bool start(bool& terminate)=0;
    virtual void stop()=0;
    virtual bool finished()=0;

};

#endif // KSSERVICECONTROL_H

