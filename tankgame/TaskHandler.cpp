//
//  TaskHandler.cpp
//  tankgame
//
//  Created by Brien Smith Martínez on 5/1/16.
//  Copyright © 2016 Brien Smith Martínez. All rights reserved.
//

#include "TaskHandler.h"

TaskHandler::TaskHandler()
{
    

}

TaskHandler::~TaskHandler()
{
    
}


int TaskHandler::Execute()
{
    for( std::list<BaseTask*>::iterator it = taskList.begin(); it != taskList.end(); it++)
    {
        if(!(*it)->canKill)
        {
            (*it)->Update();
        }
    }
    
    for( std::list<BaseTask*>::iterator it = taskList.begin(); it != taskList.end();)
    {
        if((*it)->canKill)
        {
            (*it)->Stop();
            delete (*it);
            it = taskList.erase(it);
        }
        else
        {
            it++;
        }
    }
    
    return 0;
}

bool TaskHandler::AddTask(BaseTask *t)
{
    if( !t->Start() )
    {
        return false;
    }
    for( std::list<BaseTask*>::iterator it = taskList.begin(); it != taskList.end(); it++)
    {
        if( (*it)->priority > t->priority)
        {
            taskList.insert(it, t);
            break;
        }
    }
    return true;
}

void TaskHandler::SuspendTask(BaseTask *t)
{
    if(std::find(taskList.begin(),taskList.end(),t)!=taskList.end())
    {
        t->OnSuspend();
        taskList.remove(t);
        pausedTaskList.push_back(t);
    }
}

void TaskHandler::ResumeTask(BaseTask *t)
{
    if(std::find(pausedTaskList.begin(),pausedTaskList.end(),t)
       !=pausedTaskList.end())
    {
        t->OnResume();
        pausedTaskList.remove(t);
        //keep the order of priorities straight
        std::list< BaseTask* >::iterator it;
        for(it=taskList.begin();it!=taskList.end();it++)
        {
            if((*it)->priority>t->priority)break;
        }
        taskList.insert(it,t);
    }
}

void TaskHandler::RemoveTask(BaseTask *t)
{
    if(std::find(taskList.begin(),taskList.end(),t)!=taskList.end())
    {
        t->canKill=true;
    }
}

void TaskHandler::KillAllTasks()
{
    for(std::list< BaseTask* >::iterator it=taskList.begin(); it!=taskList.end();it++)
    {
        (*it)->canKill=true;
    }
}