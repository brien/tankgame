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