//
//  App.h
//  tankgame
//
//  Created by Brien Smith Martínez on 22/12/15.
//  Copyright © 2015 Brien Smith Martínez. All rights reserved.
//

#ifndef App_h
#define App_h

#include "Singleton.h"

class App : public Singleton<App>
{
public:
    //InputTask* inputTask;
    //VideoTask* videoTask;
    //GlobalTimer* globalTimer;
    //SoundTask* soundTask;
    //GraphicsTask* graphicsTask;
    
    //GameTask* gameTask;
    
    
    bool quit;
    
    void Run(int argc, char *argv[]);
};

#endif /* App_h */