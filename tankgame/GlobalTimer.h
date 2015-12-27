//
//  GlobalTimer.h
//  tankgame
//
//  Created by Brien Smith Martínez on 24/12/15.
//  Copyright © 2015 Brien Smith Martínez. All rights reserved.
//

#ifndef GlobalTimer_h
#define GlobalTimer_h

#include "BaseTask.h"

class GlobalTimer : public BaseTask
{
public:
    GlobalTimer();
    virtual ~GlobalTimer();
    
    
    static float dT;
    static unsigned long lastFrameIndex;
    static unsigned long thisFrameIndex;
    
    bool Start();
    void Update();
    void Stop();
    
    void Reset();
    
    static inline unsigned long TimeToIndex(float t) { return (unsigned long)(t*1000.0f); }
    static inline float IndexToTime(unsigned long i) { return ((float)i)/1000.0f;	      }
};

#endif /* GlobalTimer_h */
