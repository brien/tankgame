//
//  InputTask.h
//  tankgame
//
//

#ifndef InputTask_h
#define InputTask_h

#include <string>
using namespace std;

#include <SDL2/SDL.h>
#include "ITask.h"

class InputTask : public ITask
{
private:
    const static int MAX_JOYSTICKS = 4;
    const static int MAX_JOYSTICK_AXES = 4;
public:
    InputTask();
    virtual ~InputTask();
    
    bool Start();
    void Update();
    void Stop();
    
    static SDL_Joystick *joysticks[MAX_JOYSTICKS];
    static string joynames[2];
    static int jdA[MAX_JOYSTICKS][MAX_JOYSTICK_AXES];
    
    static Uint8 *keys;
    static Uint8 *oldKeys;
    static int keyCount;
    
    static int GetAxis(int joystickId, int axis);
    static unsigned char GetButton(int joystickId, int bid);
    
    static int dX,dY;
    static unsigned int buttons;
    static unsigned int oldButtons;
    
    static bool inline CurKey(int index) { return (keys[index]!=0); }
    static bool inline OldKey(int index) { return (oldKeys[index]!=0); }
    
    //some helper functions to make certain things easier
    static bool inline KeyDown(int index)		{ return ( CurKey(index))&&(!OldKey(index)); }
    static bool inline KeyStillDown(int index)	{ return ( CurKey(index))&&( OldKey(index)); }
    static bool inline KeyUp(int index)		{ return (!CurKey(index))&&( OldKey(index)); }
    static bool inline KeyStillUp(int index)	{ return (!CurKey(index))&&(!OldKey(index)); }
    
    static bool inline CurMouse(int button) { return (buttons&SDL_BUTTON(button))!=0; }
    static bool inline OldMouse(int button) { return (oldButtons&SDL_BUTTON(button))!=0; }
    
    static bool inline MouseDown(int button)	{ return ( CurMouse(button))&&(!OldMouse(button)); }
    static bool inline MouseStillDown(int button)	{ return ( CurMouse(button))&&( OldMouse(button)); }
    static bool inline MouseUp(int button)		{ return (!CurMouse(button))&&( OldMouse(button)); }
    static bool inline MouseStillUp(int button)	{ return (!CurMouse(button))&&(!OldMouse(button)); }
};

#endif /* InputTask_h */
