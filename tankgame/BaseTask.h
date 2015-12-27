//
//  BaseTask.h
//  tankgame
//
//  Created by Brien Smith Martínez on 17/12/15.
//  Copyright © 2015 Brien Smith Martínez. All rights reserved.
//

#ifndef BSM_BaseTask_h
#define BSM_BaseTask_h

class BaseTask
{
public:
    BaseTask(){canKill=false;priority=5000;}
    virtual bool Start()=0;
    virtual void OnSuspend(){};
    virtual void Update()=0;
    virtual void OnResume(){};
    virtual void Stop()=0;
    
    bool canKill;
    long priority;
};

#endif /* BSM_BaseTask_h */
