//
//  GlobalTimer.h
//  tankgame
//
//

#ifndef GlobalTimer_h
#define GlobalTimer_h

#include "ITask.h"

class GlobalTimer : public ITask
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
