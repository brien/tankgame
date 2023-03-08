#ifdef _WIN32
// If building in windows:
#include <windows.h>
#include <GL/gl.h>
#else
//if not:
#include <OpenGL/gl.h>
#endif


#include "TankHandler.h"
#include "LevelHandler.h"
#include "GlobalTimer.h"
#include "math.h"
#include "App.h"
#include <string>


TankHandler::TankHandler()
{
    numPlayers=1;
    attackingTanks=0;
    isInputJoy=false;
    
    combo[0]=0;
    combo[1]=0;
    comboNum[0]=0;
    comboNum[1]=0;
    
    hitCombo[0]=0;
    hitCombo[1]=0;
    
    wins[0]=0;
    wins[1]=0;
    
    /*FILE* tload = fopen("body.gsm","rb");
     
     if(tload!=NULL)
     {
     bodymesh.LoadGSM(tload);
     fclose(tload);
     }
     
     bodymesh.SafetyCheck();
     bodymesh.Unitize(.5);
     bodymesh.ScaleVerticies(1, 1.5, 1);
     bodymesh.GenerateFacets();
     bodymesh.MergeVerticies();
     bodymesh.GenerateEdges();
     
     
     
     tload = fopen("turret.gsm","rb");
     
     if(tload!=NULL)
     {
     turretmesh.LoadGSM(tload);
     fclose(tload);
     }
     
     turretmesh.SafetyCheck();
     turretmesh.Unitize(.3);
     turretmesh.GenerateFacets();
     turretmesh.MergeVerticies();
     turretmesh.GenerateEdges();
     
     tload = fopen("turret.gsm","w+b");
     
     if(tload!=NULL)
     {
     turretmesh.SaveGSM(tload);
     fclose(tload);
     }
     
     
     tload = fopen("cannon.gsm","rb");
     
     if(tload!=NULL)
     {
     cannonmesh.LoadGSM(tload);
     fclose(tload);
     }
     
     cannonmesh.SafetyCheck();
     cannonmesh.Unitize(.3);
     cannonmesh.GenerateFacets();
     cannonmesh.MergeVerticies();
     cannonmesh.GenerateEdges();*/
    
}

TankHandler::~TankHandler()
{
    tanks.clear();
    
}

void TankHandler::Init()
{
    combo[0]=0;
    combo[1]=0;
    special[0]+=(float)comboNum[0];
    special[1]+=(float)comboNum[1];
    comboNum[0]=0;
    comboNum[1]=0;
    
    for(int j=0; j<numPlayers; j++)
    {
        players[j].Init();
        players[j].isPlayer = true;

        if(App::GetSingleton().gameTask->versus)
        {
            players[j].x=LevelHandler::GetSingleton().enemy[8][0];
            players[j].z=LevelHandler::GetSingleton().enemy[8][1];
            
            players[0].x=LevelHandler::GetSingleton().enemy[9][0];
            players[0].z=LevelHandler::GetSingleton().enemy[9][1];
        }
        players[j].x+=j;
        
        players[j].id=-1*(j+1);
        
        players[j].r=1;
        players[j].g=0;
        players[j].b=0;
        
        players[j].r2=1;
        players[j].g2=1;
        players[j].b2=1;
        
        players[j].jid=j;

        players[j].inputMode = j == 0 ? InputMode::MODE_KEYBOARD_MOUSE : InputMode::MODE_JOYSTICK_GENERIC;
        
        players[j].SetType(players[j].type1, players[j].type2);
        
        if(diff==0)
        {
            players[j].energyRegen*=2;
            players[j].maxEnergy*=3;
            players[j].energy*=3;
        }
        else
            if(diff==1)
            {
                players[j].energyRegen*=1.5;
                players[j].maxEnergy*=1.5;
                players[j].energy*=1.5;
            }
        
        players[j].rotRate*=2;
        
    }
    if(numPlayers>1)
    {
        if(SDL_NumJoysticks()==1)
        {
            players[1].jid=0;
            if(InputTask::joynames[0].find("PS2")!=string::npos || InputTask::joynames[0].find("MP-8866")!=string::npos)
                players[1].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            else
                if(InputTask::joynames[0].find("Extreme 3D")!=string::npos)
                    players[1].inputMode = InputMode::MODE_EXTREME_3D;
                else
                    if(InputTask::joynames[0].find("NGC")!=string::npos || InputTask::joynames[0].find("GameCube")!=string::npos)
                        players[1].inputMode = InputMode::MODE_NINTENDO_GC;
        }
        if(SDL_NumJoysticks()>1)
        {
            players[0].jid=1;
            players[1].jid=0;
            players[0].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            players[1].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            if(InputTask::joynames[0].find("PlayStation")!=string::npos || InputTask::joynames[0].find("Playstation")!=string::npos || InputTask::joynames[1].find("PS2")!=string::npos || InputTask::joynames[1].find("MP-8866")!=string::npos)
                players[0].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            else
                if(InputTask::joynames[1].find("Extreme 3D")!=string::npos)
                    players[0].inputMode = InputMode::MODE_EXTREME_3D;
                else
                    if(InputTask::joynames[1].find("NGC")!=string::npos || InputTask::joynames[1].find("GameCube")!=string::npos)
                        players[0].inputMode = InputMode::MODE_NINTENDO_GC;
            
            if(InputTask::joynames[0].find("PS2")!=string::npos || InputTask::joynames[0].find("MP-8866")!=string::npos)
                players[1].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            else
                if(InputTask::joynames[0].find("Extreme 3D")!=string::npos)
                    players[1].inputMode = InputMode::MODE_EXTREME_3D;
                else
                    if(InputTask::joynames[0].find("NGC")!=string::npos || InputTask::joynames[0].find("GameCube")!=string::npos)
                        players[1].inputMode = InputMode::MODE_NINTENDO_GC;
        }
        
    }
    else
        if(SDL_NumJoysticks()>0 && isInputJoy)
        {
            players[0].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            if(InputTask::joynames[0].find("PlayStation")!=string::npos || InputTask::joynames[0].find("Playstation")!=string::npos || InputTask::joynames[0].find("PS2")!=string::npos || InputTask::joynames[0].find("MP-8866")!=string::npos)
                players[0].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
            else
                if(InputTask::joynames[0].find("Extreme 3D")!=string::npos)
                    players[0].inputMode = InputMode::MODE_EXTREME_3D;
                else
                    if(InputTask::joynames[0].find("NGC")!=string::npos || InputTask::joynames[0].find("GameCube")!=string::npos)
                        players[0].inputMode = InputMode::MODE_NINTENDO_GC;
            //players[0].inputMode = InputMode::MODE_JOYSTICK_GENERIC;
        }
    
    
    /*if(twop)
     {
     
     player2.Init();
     
     player2.id=-2;
     
     player2.r=1;
     player2.g=0;
     player2.b=0;
     
     player2.r2=1;
     player2.g2=1;
     player2.b2=1;
     
     
     player2.inputMode = InputMode::MODE_JOYSTICK_GENERIC;
     
     
     player2.SetType(player2.type1, player2.type2);
     
     player2.rotRate*=2;
     player2.energyRegen*=1.5;
     player2.maxEnergy*=1.5;
     }*/
    
    //player.energy=999999;
    //player.charge=999999;
    //player.chargeRegen=50;
    //  player.energyRegen=2000;
    
    
    //tanks.reserve(6);
    
    //tanks.bullets.clear();
    tanks.clear();
    
    int enemyCount = 5+3*(LevelHandler::GetSingleton().levelNumber-48);
    
    
    for(int i = 0; i<enemyCount; ++i)
    {
        Tank temp;
        
        temp.Init();
        
        if(i<16)
        {
            temp.x=LevelHandler::GetSingleton().enemy[i][0];//60+i%2;
            temp.z=LevelHandler::GetSingleton().enemy[i][1];//60+i%2;
        }
        else
        {
            temp.x=LevelHandler::GetSingleton().enemy[i%16][0]+i%2;
            temp.z=LevelHandler::GetSingleton().enemy[i%16][1]+i%2;
        }
        
        
        
        
        
        temp.y=LevelHandler::GetSingleton().GetTerrainHeight((int)temp.x, (int)temp.z);
        temp.ry=i*45;
        temp.id=i;
        
        
        if((LevelHandler::GetSingleton().levelNumber-48)==0)
        {
            temp.SetType(static_cast<TankType>(1+i%3), TYPE_GREY);
            if(temp.type1==TYPE_BLUE)
                temp.SetType(TYPE_YELLOW, TYPE_GREY);
            if(temp.id==4)
            {
                temp.SetType(TYPE_BLUE,TYPE_GREY);
                temp.attack=temp.attack/3;
            }
        }
        
        else
            if((LevelHandler::GetSingleton().levelNumber-48)==2 ||  (LevelHandler::GetSingleton().levelNumber-48)==5 || (LevelHandler::GetSingleton().levelNumber-48)==12 )
            {
                temp.SetType(static_cast<TankType>(i%4+1), static_cast<TankType>(i%3+1));
            }
            else
                temp.SetType(static_cast<TankType>(1+i%3), static_cast<TankType>(i%3));
        
        temp.jumpCost=0;
        
        tanks.push_back(temp);
    }
    
}



void TankHandler::NextFrame()
{
    for(int i=0; i<numPlayers; i++)
    {
        if(combo[i]>0.0f)
        {
            combo[i]-=1.0f*GlobalTimer::dT;
            if(combo[i]<0.0f)
            {
                combo[i]=0.0f;
            }
        }
        else
        {
            comboNum[i]=0;
        }
        
        if(players[i].alive )
        {
            players[i].NextFrame();
            players[i].HandleInput();
            players[i].deadtime=0.0f;
        }
        else
        {
            players[i].Die();
            if(App::GetSingleton().gameTask->versus )
            {
                if(players[i].deadtime>1.5)
                {
                    LevelHandler::GetSingleton().NextLevel(true);
                }
            }
            else
            {
                if(players[i].deadtime>0.5)
                    LevelHandler::GetSingleton().NextLevel(false);
            }
            players[i].deadtime+=GlobalTimer::dT;
        }
        
    }
    
    
    
    float dist;
    players[0].dist=2024;
    players[1].dist=2024;
    
    if(!tanks.empty())
    {
        vector< Tank >::iterator it;
        for(it=tanks.begin();it!=tanks.end();)
        {
            if(it->alive)
            {
                it->AI();
                it->NextFrame();
                
                dist=sqrt((it->x-players[0].x)*(it->x-players[0].x)+(it->z-players[0].z)*(it->z-players[0].z));
                
                
                if(dist<players[0].dist)
                {
                    players[0].dist=dist;
                    
                    float ratio = (double)(players[0].z-it->z)/(double)(players[0].x-it->x);
                    
                    float rtyp=toDegrees(atan(ratio));// - players[0].ry;
                    
                    if(it->x<players[0].x)
                    {
                        rtyp+=180;
                    }
                    
                    players[0].rr=rtyp;
                    
                    players[0].rrl=players[0].rr;
                }
                
                dist=sqrt((it->x-players[1].x)*(it->x-players[1].x)+(it->z-players[1].z)*(it->z-players[1].z));
                
                if(dist<players[1].dist)
                {
                    players[1].dist=dist;
                    
                    float ratio = (double)(players[1].z-it->z)/(double)(players[1].x-it->x);
                    
                    float rtyp=toDegrees(atan(ratio));// - players[0].ry;
                    
                    if(it->x<players[0].x)
                    {
                        rtyp+=180;
                    }
                    
                    players[1].rr=rtyp;
                    
                    players[1].rrl=players[1].rr;
                }
                
                ++it;
            }
            else
            {
                it->Die();
                it = tanks.erase(it);
            }
        }
    }
    
    if(App::GetSingleton().gameTask->versus)
    {
        players[0].dist=sqrt((players[0].x-players[1].x)*(players[0].x-players[1].x)+(players[0].z-players[1].z)*(players[0].z-players[1].z));
        players[1].dist=players[0].dist;
        
        float ratio = (double)(players[0].z-players[1].z)/(double)(players[0].x-players[1].x);
        
        float rtyp=toDegrees(atan(ratio));
        
        if(players[1].x<players[0].x)
        {
            rtyp+=180;
        }
        
        players[0].rr=rtyp;
        
        players[0].rrl=players[0].rr;
        
        
        rtyp+=180;
        
        players[1].rr=rtyp;
        
        players[1].rrl=players[1].rr;
    }
    
    LevelHandler::GetSingleton().ItemCollision();
}

void TankHandler::DrawTanks()
{
    /*glColor3f(player.r, player.g, player.b);
     
     
     glPushMatrix();
     
     glTranslatef(player.x, player.y+.2, player.z);
     glRotatef(player.rx, 1, 0, 0);
     glRotatef(-player.ry, 0, 1, 0);
     glRotatef(player.rz, 0, 0, 1);
     
     glDisable(GL_TEXTURE_2D);
     
     
     glScalef(.5, 1, 0.4);
     glCallList(App::GetSingleton().graphicsTask->squarelist2);
     
     glEnable(GL_BLEND);
     glEnable(GL_DEPTH_TEST);
     //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     glBlendFunc(GL_SRC_ALPHA, GL_ONE);
     glDepthMask(GL_FALSE);
     
     glColor4f(player.r, player.g, player.b, player.energy/2000);
     
     App::GetSingleton().graphicsTask->cubelist1.Call(0);
     
     glDisable(GL_BLEND);
     glDepthMask(GL_TRUE);
     glEnable(GL_CULL_FACE);
     
     glPopMatrix();*/
    
    //TankHandler::GetSingleton().player.Draw();
    
    static float drift=0;
    
    for(int k=0; k<numPlayers; k++)
    {
        if(players[k].alive)
        {
            glPushMatrix();
            
            glDisable(GL_TEXTURE_2D);
            
            glEnable(GL_LIGHTING);
            
            //glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
            
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);		//This is the key
            
            //players[k].Draw();
            //glCallList(App::GetSingleton().graphicsTask->bodylist);
            
            //-------------
            glPushMatrix();
            
            //glDisable(GL_TEXTURE_2D);
            //glDisable(GL_LIGHTING);
            
            glTranslatef(players[k].x, players[k].y+.05, players[k].z);
            glRotatef(players[k].rx, 1, 0, 0);
            glRotatef(-players[k].ry-90, 0, 1, 0);
            glRotatef(players[k].rz, 0, 0, 1);
            
            glFrontFace(GL_CCW);
            glColor3f(players[k].r2, players[k].g2, players[k].b2);
            
            //glCallList(App::GetSingleton().graphicsTask->bodylist);
            App::GetSingleton().graphicsTask->bodylist.Call(0);
            
            
            
            glTranslatef(0, .10, 0);
            
            glRotatef(players[k].rtx, 1, 0, 0);
            glRotatef(-players[k].rty+180, 0, 1, 0);
            glRotatef(players[k].rtz, 0, 0, 1);
            
            glColor3f(players[k].r, players[k].g, players[k].b);
            
            //glCallList(App::GetSingleton().graphicsTask->turretlist);
            App::GetSingleton().graphicsTask->turretlist.Call(0);
            
            
            glFrontFace(GL_CW);
            
            glPopMatrix();
            //-----
            
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
            
            glDepthMask(GL_FALSE);				//Allow depth testing to be performed
            glDepthFunc(GL_LEQUAL);	//
            
            glEnable(GL_BLEND);					//Blend em'
            glBlendFunc(GL_SRC_ALPHA,GL_ONE);
            
            
            
            drift+=GlobalTimer::dT;
            
            if(drift>1)
                drift=0;
            
            //Awesome effects are performed using the texture matrix stack. (depth = 2)
            glMatrixMode(GL_TEXTURE);
            
            glPushMatrix();
            //glScalef(5,5,5);
            glTranslatef(100*drift,0,0);
            //glRotatef(players[k].rty+players[k].ry,0,0,1);
            glMatrixMode(GL_MODELVIEW);
            
            
            glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[17]);
            
            //players[k].r=players[k].g=players[k].b=players[k].r=players[k].g2=players[k].b2=1;
            glColor3f(players[k].r, players[k].g, players[k].b);
            //players[k].Draw2();
            //glCallList(App::GetSingleton().graphicsTask->bodylist);
            
            //-------------
            glPushMatrix();
            
            //glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            
            glTranslatef(players[k].x, players[k].y+.05, players[k].z);
            glRotatef(players[k].rx, 1, 0, 0);
            glRotatef(-players[k].ry-90, 0, 1, 0);
            glRotatef(players[k].rz, 0, 0, 1);
            
            glFrontFace(GL_CCW);
            
            //glCallList(App::GetSingleton().graphicsTask->bodylist);
            App::GetSingleton().graphicsTask->bodylist.Call(0);
            
            glTranslatef(0, .10, 0);
            
            glRotatef(players[k].rtx, 1, 0, 0);
            glRotatef(-players[k].rty+180, 0, 1, 0);
            glRotatef(players[k].rtz, 0, 0, 1);
            
            glColor3f(players[k].r2, players[k].g2, players[k].b2);
            
            if(special[k]>=players[k].fireCost/5)
            {
                //glCallList(App::GetSingleton().graphicsTask->turretlist);
                App::GetSingleton().graphicsTask->turretlist.Call(0);
            }
            
            
            glFrontFace(GL_CW);
            
            glPopMatrix();
            
            
            
            glMatrixMode(GL_TEXTURE);//Set matrix mode to projection
            
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);	//Select modelview matrix
            
            glDepthMask(GL_TRUE);
        }
        
        if(tanks.size()>0)
        {
            
            glDisable(GL_LIGHTING);
            
            glTranslatef( players[k].x,  players[k].y+.25,  players[k].z);
            glRotatef(-players[k].rrl, 0, 1, 0);
            
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[20]);
            glEnable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDepthMask(GL_FALSE);
            
            glColor4f(1.0f,players[k].dist/50.0f,0.1f,1.0);
            
            //glCallList(App::GetSingleton().graphicsTask->squarelist);
            App::GetSingleton().graphicsTask->squarelist.Call(0);
            
            
            if(players[k].fireTimer>players[k].fireRate && players[k].fireCost<players[k].charge)
            {
                glTranslatef(0, +0.018f, 0);
                glRotatef(100*drift, 0, 1, 0);
                
                glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[17]);
                
                glColor4f(1.0f,1.0f,1.0f,1.0f);
                
                glScalef(0.2, 0.2, 0.2);
                
                //glCallList(App::GetSingleton().graphicsTask->squarelist);
                App::GetSingleton().graphicsTask->squarelist.Call(0);
                
                glScalef(5.0, 5.0, 5.0);
                
            }
            
            
            
            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glDepthMask(GL_TRUE);
            glEnable(GL_CULL_FACE);
            
            glEnable(GL_LIGHTING);
            
            glRotatef(players[k].rrl, 0, 1, 0);
            
            glTranslatef( -players[k].x,  -players[k].y+.25,  -players[k].z);
        }
        
        
        //glDisable(GL_LIGHTING);
        /*
         glTranslatef(players[k].x, players[k].y+.01, players[k].z);
         glRotatef(players[k].rx, 1, 0, 0);
         glRotatef(-players[k].ry, 0, 1, 0);
         glRotatef(players[k].rz, 0, 0, 1);
         
         glFrontFace(GL_CCW);
         glColor3f(players[k].r2, players[k].g2, players[k].b2);
         
         glCallList(App::GetSingleton().graphicsTask->bodylist);
         
         glEnable(GL_LIGHTING);
         
         glColor3f(players[k].r*(players[k].energy/players[k].maxEnergy), players[k].g*(players[k].energy/players[k].maxEnergy), players[k].b*(players[k].energy/players[k].maxEnergy));
         glCallList(App::GetSingleton().graphicsTask->bodylistEx);
         
         glTranslatef(0, .15, 0);
         
         glRotatef(players[k].rtx, 1, 0, 0);
         glRotatef(-players[k].rty, 0, 1, 0);
         glRotatef(players[k].rtz, 0, 0, 1);
         
         glDisable(GL_LIGHTING);
         
         //glFrontFace(GL_CW);
         glCallList(App::GetSingleton().graphicsTask->turretlistEx);
         glCallList(App::GetSingleton().graphicsTask->cannonlistEx);
         
         glColor3f(players[k].r2, players[k].g2, players[k].b2);
         
         glCallList(App::GetSingleton().graphicsTask->turretlist);
         glCallList(App::GetSingleton().graphicsTask->cannonlist);
         
         glFrontFace(GL_CW); */
        
        glPopMatrix();
    }
    
    
    for(vector<Tank>::iterator i = tanks.begin(); i!=tanks.end(); ++i)
    {
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
        i->Draw();
        /*
         glPushMatrix();
         
         glDisable(GL_TEXTURE_2D);
         
         glEnable(GL_LIGHTING);
         
         //glEnable(GL_TEXTURE_2D);
         glEnable(GL_DEPTH_TEST);
         
         glDepthMask(GL_TRUE);
         glDepthFunc(GL_LESS);		//This is the key
         
         //i->Draw();
         //glCallList(App::GetSingleton().graphicsTask->bodylist);
         
         //-------------
         glPushMatrix();
         
         //glDisable(GL_TEXTURE_2D);
         //glDisable(GL_LIGHTING);
         
         glTranslatef(i->x, i->y+.05, i->z);
         glRotatef(i->rx, 1, 0, 0);
         glRotatef(-i->ry, 0, 1, 0);
         glRotatef(i->rz, 0, 0, 1);
         
         glFrontFace(GL_CCW);
         glColor3f(i->r2, i->g2, i->b2);
         
         //glCallList(App::GetSingleton().graphicsTask->bodylist);
         
         
         glCallList(App::GetSingleton().graphicsTask->itemlist);
         
         
         
         
         
         glTranslatef(0, .15, 0);
         
         glRotatef(i->rtx, 1, 0, 0);
         glRotatef(-i->rty+90, 0, 1, 0);
         glRotatef(i->rtz, 0, 0, 1);
         
         glColor3f(i->r, i->g, i->b);
         
         glCallList(App::GetSingleton().graphicsTask->turretlist);
         
         
         glFrontFace(GL_CW);
         
         glPopMatrix();
         //-----
         
         glEnable(GL_TEXTURE_2D);
         glEnable(GL_DEPTH_TEST);
         
         glDepthMask(GL_FALSE);				//Allow depth testing to be performed
         glDepthFunc(GL_LEQUAL);	//
         
         glEnable(GL_BLEND);					//Blend em'
         glBlendFunc(GL_SRC_ALPHA,GL_ONE);
         
         static float drift=0;
         
         drift+=GlobalTimer::dT;
         
         if(drift>1)
         drift=0;
         
         //Awesome effects are performed using the texture matrix stack. (depth = 2)
         glMatrixMode(GL_TEXTURE);
         
         glPushMatrix();
         //glScalef(5,5,5);
         glTranslatef(100*drift,0,0);
         //glRotatef(i->rty+i->ry,0,0,1);
         glMatrixMode(GL_MODELVIEW);
         
         
         glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[17]);
         
         //i->r=i->g=i->b=i->r=i->g2=i->b2=1;
         glColor3f(i->r, i->g, i->b);
         //i->Draw2();
         //glCallList(App::GetSingleton().graphicsTask->bodylist);
         
         //-------------
         glPushMatrix();
         
         //glDisable(GL_TEXTURE_2D);
         glDisable(GL_LIGHTING);
         
         glTranslatef(i->x, i->y+.05, i->z);
         glRotatef(i->rx, 1, 0, 0);
         glRotatef(-i->ry, 0, 1, 0);
         glRotatef(i->rz, 0, 0, 1);
         
         glFrontFace(GL_CCW);
         
         
         //glCallList(App::GetSingleton().graphicsTask->bodylist);
         
         glCallList(App::GetSingleton().graphicsTask->itemlist);
         
         
         
         
         glTranslatef(0, .10, 0);
         
         glRotatef(i->rtx, 1, 0, 0);
         glRotatef(-i->rty+180, 0, 1, 0);
         glRotatef(i->rtz, 0, 0, 1);
         
         glColor3f(i->r2, i->g2, i->b2);
         
         glCallList(App::GetSingleton().graphicsTask->turretlist);
         
         
         glFrontFace(GL_CW);
         
         glPopMatrix();
         //-----
         
         
         
         glMatrixMode(GL_TEXTURE);//Set matrix mode to projection		
         
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);	//Select modelview matrix	
         
         glDepthMask(GL_TRUE);
         
         glPopMatrix();
         */
        
        
        /* glPushMatrix();
         glDisable(GL_TEXTURE_2D);
         
         glTranslatef(i->x, i->y, i->z);
         glRotatef(i->rx, 1, 0, 0);
         glRotatef(-i->ry, 0, 1, 0);
         glRotatef(i->rz, 0, 0, 1);
         
         glFrontFace(GL_CCW);
         glColor3f(i->r2, i->g2, i->b2);
         glCallList(App::GetSingleton().graphicsTask->bodylist);
         glDisable(GL_LIGHTING);
         
         glColor3f(i->r*(i->energy/i->maxEnergy), i->g*(i->energy/i->maxEnergy), i->b*(i->energy/i->maxEnergy));
         glCallList(App::GetSingleton().graphicsTask->bodylistEx);
         
         glTranslatef(0, .15, 0);
         
         glRotatef(i->rtx, 1, 0, 0);
         glRotatef(-i->rty, 0, 1, 0);
         glRotatef(i->rtz, 0, 0, 1);
         
         
         glCallList(App::GetSingleton().graphicsTask->turretlistEx);
         
         glCallList(App::GetSingleton().graphicsTask->cannonlistEx);
         
         
         glEnable(GL_LIGHTING);
         
         glColor3f(i->r2, i->g2, i->b2);
         glCallList(App::GetSingleton().graphicsTask->turretlist);
         
         glCallList(App::GetSingleton().graphicsTask->cannonlist);
         
         glFrontFace(GL_CW);
         
         glPopMatrix();*/
    }
    
}

void TankHandler::DrawBullets()
{
    glDisable(GL_TEXTURE_2D);
    
    for(vector<Tank>::iterator i = tanks.begin(); i!=tanks.end(); ++i)
    {
        for(vector<Bullet>::iterator j = i->bullets.begin(); j != i->bullets.end(); ++j)
        {
            if(j->alive)
            {
                j->Draw();
            }
        }
    }
    
    for(int k=0; k<numPlayers; k++)
    {
        for(vector<Bullet>::iterator j = players[k].bullets.begin(); j != players[k].bullets.end(); ++j)
        {
            if(j->alive)
            {
                j->Draw();
            }
        }
    }
    
}

