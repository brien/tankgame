//
//  TaskHandler.cpp
//  tankgame
//
//

#include "TaskHandler.h"
#include <algorithm>

TaskHandler::TaskHandler()
{
    

}

TaskHandler::~TaskHandler()
{
    
}


int TaskHandler::Execute()
{
    while(!taskList.empty())
    {
        for( std::list<ITask*>::iterator it = taskList.begin(); it != taskList.end(); it++)
        {
            if(!(*it)->canKill)
            {
                (*it)->Update();
            }
        }
        
        for( std::list<ITask*>::iterator it = taskList.begin(); it != taskList.end();)
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
    }
    
    return 0;
}

bool TaskHandler::AddTask(ITask *t)
{
    if( !t->Start() )
    {
        return false;
    }
    std::list<ITask*>::iterator it;
    for(it = taskList.begin(); it != taskList.end(); it++)
    {
        if( (*it)->priority > t->priority)
        {
            break;
        }
    }
    taskList.insert(it, t);
    return true;
}

void TaskHandler::SuspendTask(ITask *t)
{
    if(std::find(taskList.begin(),taskList.end(),t)!=taskList.end())
    {
        t->OnSuspend();
        taskList.remove(t);
        pausedTaskList.push_back(t);
    }
}

void TaskHandler::ResumeTask(ITask *t)
{
    if(std::find(pausedTaskList.begin(),pausedTaskList.end(),t)
       !=pausedTaskList.end())
    {
        t->OnResume();
        pausedTaskList.remove(t);
        //keep the order of priorities straight
        std::list< ITask* >::iterator it;
        for(it=taskList.begin();it!=taskList.end();it++)
        {
            if((*it)->priority>t->priority)break;
        }
        taskList.insert(it,t);
    }
}

void TaskHandler::RemoveTask(ITask *t)
{
    if(std::find(taskList.begin(),taskList.end(),t)!=taskList.end())
    {
        t->canKill=true;
    }
}

void TaskHandler::KillAllTasks()
{
    for(std::list< ITask* >::iterator it=taskList.begin(); it!=taskList.end();it++)
    {
        (*it)->canKill=true;
    }
}
