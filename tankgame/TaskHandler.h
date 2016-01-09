//
//  TaskHandler.hpp
//  tankgame
//
//  Created by Brien Smith Martínez on 5/1/16.
//  Copyright © 2016 Brien Smith Martínez. All rights reserved.
//

#ifndef TaskHandler_h
#define TaskHandler_h

#include <list>

#include "Singleton.h"
#include "BaseTask.h"


class TaskHandler : public Singleton<TaskHandler>
{
public:
    TaskHandler();
    virtual ~TaskHandler();
    
    int Execute();
    
    bool AddTask(BaseTask *t);
    void SuspendTask(BaseTask *t);
    void ResumeTask(BaseTask *t);
    void RemoveTask(BaseTask *t);
    void KillAllTasks();
    
protected:
    std::list< BaseTask* > taskList;
    std::list< BaseTask* > pausedTaskList;
};

#endif /* TaskHandler_hpp */
