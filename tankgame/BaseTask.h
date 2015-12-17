//
//  BaseTask.h
//  tankgame
//
//  Created by Brien Smith Martínez on 17/12/15.
//  Copyright © 2015 Brien Smith Martínez. All rights reserved.
//

#ifndef BaseTask_h
#define BaseTask_h

class BaseTask
{
    BaseTask(){canKill=false;priority=5000;}
    virtual bool Start()=0;
    virtual void OnSuspend{};
    virtual void Update()=0;
    virtual void OnResume(){};
    virtual void Stop()=0;
    
    bool canKill;
    long priority;
};

#endif /* BaseTask_h */
