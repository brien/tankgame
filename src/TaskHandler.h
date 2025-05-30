//
//  TaskHandler.hpp
//  tankgame
//
//

#ifndef TaskHandler_h
#define TaskHandler_h

#include <list>

#include "Singleton.h"
#include "ITask.h"


class TaskHandler : public Singleton<TaskHandler>
{
public:
    TaskHandler();
    ~TaskHandler() = default;
    
    int Execute();
    
    bool AddTask(ITask *t);
    void SuspendTask(ITask *t);
    void ResumeTask(ITask *t);
    void RemoveTask(ITask *t);
    void KillAllTasks();
    
protected:
    std::list< ITask* > taskList;
    std::list< ITask* > pausedTaskList;
};

#endif /* TaskHandler_hpp */
