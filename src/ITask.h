//
//  ITask.h
//  tankgame
//
//

#pragma once

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
