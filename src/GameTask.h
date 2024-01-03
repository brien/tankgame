// GameTask.h: interface for the GameTask class.
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_TASK_H_
#define GAME_TASK_H_

#include "ITask.h"

class GameTask : public ITask
{
public:
    GameTask();
    virtual ~GameTask();
    
    bool Start();
    void Update();
    void Stop();
    void OnSuspend();
    void OnResume();
    
    void SetUpGame();
    void DisplayScore();
    void DisplayTime();
    void GameOver();
    
    void Visible(bool visible);
    
    bool paused;
    
    bool debug;
    
    int menuState;
    bool versus;
        
    int timer;
    
    bool gameOver;
    bool gameStarted;
    
};

#endif
