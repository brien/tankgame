//
//  ITask.h
//  tankgame
//
//

#ifndef BSM_ITask_h
#define BSM_ITask_h

class ITask
{
public:
    ITask()
    {
        canKill = false;
        priority = 5000;
    }
    virtual ~ITask() {};
    virtual bool Start() = 0;
    virtual void OnSuspend() {};
    virtual void Update() = 0;
    virtual void OnResume() {};
    virtual void Stop() = 0;

    bool canKill;
    long priority;
};

#endif /* BSM_ITask_h */
