#ifndef CLSERVICECONTROL_H
#define CLSERVICECONTROL_H


class CLServiceControl
{
protected:
    bool m_started = false;
    bool m_stop = false;
    bool m_finished = true;

public:
    virtual bool start(bool& terminate)=0;
    virtual void stop()=0;
    virtual bool finished()=0;
    virtual void iterate()=0;

};

#endif // CLSERVICECONTROL_H
