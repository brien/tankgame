//
//  Bullet.cpp
//  tankgame
//
//  Created by Brien Smith Martínez on 27/12/15.
//  Copyright © 2015 Brien Smith Martínez. All rights reserved.
//

#include <OpenGL/gl.h>

#include "Bullet.h"

#include "GlobalTimer.h"
#include "math.h"
#include "LevelHandler.h"
#include "TankHandler.h"
#include "FXHandler.h"
#include "App.h"



Bullet::Bullet()
{
    movRate=33;
    alive=true;
    spec=false;
    id=0;
    numbounces=0;
    r=g=b=.5;
    r2=g2=b2=.5;
    dty=0;
    
    dT=0;
    maxdT=100;
}

void Bullet::NextFrame()
{
    
    dT+=GlobalTimer::dT;
    
    if(type1==4 && spec)
    {
        if(dty<0)
        {
            dty-=1000*GlobalTimer::dT;
        }
        else
        {
            dty+=1000*GlobalTimer::dT;
        }
        
    }
    
    
    ry+=GlobalTimer::dT*dty;
    
    
    
    if(ry>360)
    {
        ry-=360;
    }
    if(rx>360)
    {
        rz-=360;
    }
    if(rz>360)
    {
        rz-=360;
    }
    
    float xpp=(GlobalTimer::dT*movRate) * (float)cos(ry*DTR);
    float zpp=(GlobalTimer::dT*movRate) * (float)sin(ry*DTR);
    
    x += xpp;
    z += zpp;
    
    float ory=ry;
    
    
    if(LevelHandler::GetSingleton().PointCollision(x,y,z))
    {
        
        
        x -= xpp;
        z -= zpp;
        if(numbounces<maxbounces)
        {
            if((int)(x+xpp)!=(int)x && (int)(z+zpp)==(int)z)
            {
                ry=-ry+180;
            }
            //else
            if((int)(z+zpp)!=(int)z && (int)(x+xpp)==(int)x)
                ry=-ry;
            
            if((int)(x+xpp)!=(int)x && (int)(z+zpp)!=(int)z)
            {
                if(LevelHandler::GetSingleton().PointCollision((x+xpp),y,z) && !LevelHandler::GetSingleton().PointCollision(x,y,z+zpp))
                {
                    ry=-ry+180;
                }
                else
                {
                    ry=-ry;
                }
            }
            
            int dist=(int)sqrt((x-TankHandler::GetSingleton().players[0].x)*(x-TankHandler::GetSingleton().players[0].x)+(z-TankHandler::GetSingleton().players[0].z)*(z-TankHandler::GetSingleton().players[0].z));
             
            if(dist>128)
            {
                 dist=128;
            }
            Mix_Volume(9, 128-dist);
             
             //Mix_SetPosition(2, ryp, (int)dist);
            
            App::GetSingleton().soundTask->PlaySound(9);
            
            if(numbounces<1)
            {
                if(type1==3 && type2==3)
                {
                    power*=2.0;
                }
                else
                {
                    power*=1.5;
                }
            }
            
            if(tid<0 && spec && type1==3)
            {
                if(type2==1)
                {
                    for(int i=0; i<20; i++)
                    {
                        
                        Bullet temp;
                        
                        temp.tid=tid;
                        
                        temp.x=x;
                        temp.y=y;
                        temp.z=z;
                        
                        temp.rx=rx;
                        temp.ry=i*(360/20);
                        temp.rz=rz;
                        
                        temp.power=power;
                        
                        temp.r=1;
                        temp.b=0;
                        temp.g=0;
                        
                        temp.r2=1;
                        temp.b2=0;
                        temp.g2=0;
                        
                        temp.maxbounces=0;
                        temp.numbounces=0;
                        
                        temp.type1=type2;
                        temp.type2=type2;
                        
                        
                        temp.id=1;
                        temp.alive=true;
                        
                        TankHandler::GetSingleton().players[(-1*tid)-1].bulletq.push(temp);
                    }
                }
                else
                    if(type2==2)
                    {
                        for(int i=0; i<2; i++)
                        {
                            
                            Bullet temp;
                            
                            temp.tid=tid;
                            
                            
                            temp.rx=rx;
                            temp.ry=ry;
                            temp.rz=rz;
                            
                            temp.x= x + (.2 + i*.2+numbounces*.2) * (float)cos((ry+90)*DTR);
                            temp.y=y;
                            temp.z= z + (.2 + i*.2+numbounces*.2) *(float)sin((ry+90)*DTR);
                            
                            temp.power=power;
                            
                            temp.r=r;
                            temp.b=b;
                            temp.g=g;
                            
                            temp.r2=r2;
                            temp.b2=b2;
                            temp.g2=g2;
                            
                            temp.maxbounces=4;
                            
                            temp.type1=type1;
                            temp.type2=type2;
                            
                            
                            temp.id=1;
                            temp.alive=true;
                            
                            TankHandler::GetSingleton().players[(-1*tid)-1].bulletq.push(temp);
                        }
                    }
                    else
                        if(type2==3 || type2==0)
                        {
                            
                            Bullet temp;
                            
                            temp.tid=tid;
                            
                            temp.x=x;
                            temp.y=y;
                            temp.z=z;
                            
                            temp.rx=rx;
                            temp.ry=-ory;
                            temp.rz=rz;
                            
                            temp.power=power*2;
                            
                            temp.r=r;
                            temp.b=b;
                            temp.g=g;
                            
                            temp.r2=r2;
                            temp.b2=b2;
                            temp.g2=g2;
                            
                            temp.maxbounces=4;
                            
                            temp.type1=type1;
                            temp.type2=type2;
                            
                            
                            temp.id=1;
                            temp.alive=true;
                            
                            TankHandler::GetSingleton().players[(-1*tid)-1].bulletq.push(temp);
                        }
                        else
                            if(type2==4)
                            {
                                
                                Bullet temp;
                                
                                temp.tid=tid;
                                
                                temp.x=x;
                                temp.y=y;
                                temp.z=z;
                                
                                temp.rx=rx;
                                temp.ry=ry;
                                temp.rz=rz;
                                
                                temp.power=power*2;
                                
                                temp.dty = 10.0f;
                                
                                temp.r=r;
                                temp.b=b;
                                temp.g=g;
                                
                                temp.r2=r2;
                                temp.b2=b2;
                                temp.g2=g2;
                                
                                temp.maxbounces=16;
                                
                                temp.type1=type1;
                                temp.type2=type2;
                                
                                
                                temp.id=1;
                                temp.alive=true;
                                
                                TankHandler::GetSingleton().players[(-1*tid)-1].bulletq.push(temp);
                            }
                
            }
            
            numbounces++;
            
        }
        else
        {
            alive=false;
            
            if(tid<0)
            {
                //TankHandler::GetSingleton().combo[(-1*tid)-1]+=TankHandler::GetSingleton().hitCombo[(-1*tid)-1]/2;
                TankHandler::GetSingleton().hitCombo[(-1*tid)-1]=0;
            }
            
            
            if((int)(x+xpp)!=(int)x && (int)(z+zpp)==(int)z)
            {
                FXHandler::GetSingleton().CreateFX(4, x, y, z, 0, 0, 0, 0, 0, 90, r, g, b, 1);
            }
            //else
            if((int)(z+zpp)!=(int)z && (int)(x+xpp)==(int)x)
                FXHandler::GetSingleton().CreateFX(4, x, y, z, 0, 0, 0, 0, 90, 90, r, g, b, 1);
            
            if((int)(x+xpp)!=(int)x && (int)(z+zpp)!=(int)z)
            {
                if(LevelHandler::GetSingleton().PointCollision((x+xpp),y,z) && !LevelHandler::GetSingleton().PointCollision(x,y,z+zpp))
                    FXHandler::GetSingleton().CreateFX(4, x, y, z, 0, 0, 0, 0, 0, 90, r, g, b, 1);
                else
                    FXHandler::GetSingleton().CreateFX(4, x, y, z, 0, 0, 0, 0, 90, 90, r, g, b, 1);
            }
            
            
        }
        
    }
    
    for(int i=0; i<TankHandler::GetSingleton().numPlayers; i++)
    {
        
        if(TankHandler::GetSingleton().players[i].PointCollision(x,y,z))
        {
            if(tid!=TankHandler::GetSingleton().players[i].id )
            {
                if(tid<0 && !App::GetSingleton().gameTask->versus)
                {
                    App::GetSingleton().soundTask->PlaySound(3);
                    if(TankHandler::GetSingleton().players[i].energy<TankHandler::GetSingleton().players[i].maxEnergy*2)
                    {
                        TankHandler::GetSingleton().players[i].energy+=power/2;
                    }
                    else
                        if(TankHandler::GetSingleton().players[i].charge<TankHandler::GetSingleton().players[i].maxCharge*2)
                        {
                            TankHandler::GetSingleton().players[i].charge+=power/8;
                        }
                    alive=false;
                    
                }
                else
                {
                    TankHandler::GetSingleton().players[i].energy-=power;
                    alive=false;
                    FXHandler::GetSingleton().CreateFX(5, x, y, z, 0, .01, 0, 0, TankHandler::GetSingleton().players[i].ry, 90, r, g, b, 1);
                    FXHandler::GetSingleton().CreateFX(5, x, y, z, 0, .01, 2, 0, ry, 90, r, g, b, 1);
                    App::GetSingleton().soundTask->PlaySound(8);
                    
                    TankHandler::GetSingleton().hitCombo[(-1*tid)-1]++;
                    TankHandler::GetSingleton().combo[(-1*tid)-1]+=(float)TankHandler::GetSingleton().hitCombo[(-1*tid)-1]/10;
                }
                //x -= GlobalTimer::dT*movRate * (float)cos(ry*DTR);
                //z -= GlobalTimer::dT*movRate * (float)sin(ry*DTR);
                
            }
            else
            {
                if(dT>0.5)
                {
                    App::GetSingleton().soundTask->PlaySound(3);
                    if(TankHandler::GetSingleton().players[i].energy<TankHandler::GetSingleton().players[i].maxEnergy)
                    {
                        TankHandler::GetSingleton().players[i].energy+=power/2;
                    }
                    else
                        if(TankHandler::GetSingleton().players[i].charge<TankHandler::GetSingleton().players[i].maxCharge)
                        {
                            TankHandler::GetSingleton().players[i].charge+=power/8;
                        }
                    alive=false;
                }
            }
        }
    }
    /*
     if(TankHandler::GetSingleton().player2.PointCollision(x,y,z))
     {
     if(tid>=0)
     {
     //x -= GlobalTimer::dT*movRate * (float)cos(ry*DTR);
     //z -= GlobalTimer::dT*movRate * (float)sin(ry*DTR);
     TankHandler::GetSingleton().player2.energy-=power;
     alive=false;
     }
     else
     {
     App::GetSingleton().soundTask->PlaySound(3);
     if(TankHandler::GetSingleton().player2.energy<TankHandler::GetSingleton().player2.maxEnergy*2)
     {
     TankHandler::GetSingleton().player2.energy+=power/2;
     }
     else
     if(TankHandler::GetSingleton().player2.charge<TankHandler::GetSingleton().player2.maxCharge*2)
     {
     TankHandler::GetSingleton().player2.charge+=power/8;
     }
     
     alive=false;
     }
     }
     */
    
    
    for(vector<Tank>::iterator it = TankHandler::GetSingleton().tanks.begin(); it!=TankHandler::GetSingleton().tanks.end(); ++it)
    {
        if((*it).PointCollision(x,y,z) || (*it).PointCollision(x-xpp/2,y,z-zpp/2))
        {
            if((*it).id!=tid )
            {
                //x -= GlobalTimer::dT*movRate * (float)cos(ry*DTR);
                //z -= GlobalTimer::dT*movRate * (float)sin(ry*DTR);
                if((*it).energy>0)
                {
                    (*it).energy-=power;
                    TankHandler::GetSingleton().hitCombo[(-1*tid)-1]++;
                    TankHandler::GetSingleton().combo[(-1*tid)-1]+=(float)TankHandler::GetSingleton().hitCombo[(-1*tid)-1]/10;
                    
                    if(tid<0 && (*it).energy<=0)
                    {
                        //if(TankHandler::GetSingleton().combo[(-1*tid)-1]
                        
                        float distx=x-TankHandler::GetSingleton().players[(-1*tid)-1].x;
                        float distz=z-TankHandler::GetSingleton().players[(-1*tid)-1].z;
                        
                        float dist=sqrt(distx*distx + distz*distz );
                        
                        if(TankHandler::GetSingleton().hitCombo[(-1*tid)-1]>9)
                        {
                            TankHandler::GetSingleton().combo[(-1*tid)-1]+=TankHandler::GetSingleton().comboNum[(-1*tid)-1]/2;
                            TankHandler::GetSingleton().players[(-1*tid)-1].bonus=23;
                            TankHandler::GetSingleton().players[(-1*tid)-1].bonusTime=0;
                            FXHandler::GetSingleton().CreateFX(4, TankHandler::GetSingleton().players[(-1*tid)-1].x, TankHandler::GetSingleton().players[(-1*tid)-1].y, TankHandler::GetSingleton().players[(-1*tid)-1].z, 0, .01, 0, 0, 0, 90, 0.5, 0.5, 0, 1);
                        }
                        
                        
                        
                        if(dist>20)
                        {
                            TankHandler::GetSingleton().combo[(-1*tid)-1]+=dist/10;
                            TankHandler::GetSingleton().players[(-1*tid)-1].bonus=21;
                            TankHandler::GetSingleton().players[(-1*tid)-1].bonusTime=0;
                            FXHandler::GetSingleton().CreateFX(4, TankHandler::GetSingleton().players[(-1*tid)-1].x, TankHandler::GetSingleton().players[(-1*tid)-1].y, TankHandler::GetSingleton().players[(-1*tid)-1].z, 0, .01, 0, 0, 0, 90, 0, 1, 1, 1);
                        }
                        
                        if(numbounces>0 && dT<10.0f)
                        {
                            TankHandler::GetSingleton().combo[(-1*tid)-1]+=(1+dT/2);
                            TankHandler::GetSingleton().players[(-1*tid)-1].bonus=22;
                            TankHandler::GetSingleton().players[(-1*tid)-1].bonusTime=0;
                            FXHandler::GetSingleton().CreateFX(4, TankHandler::GetSingleton().players[(-1*tid)-1].x, TankHandler::GetSingleton().players[(-1*tid)-1].y, TankHandler::GetSingleton().players[(-1*tid)-1].z, 0, .01, 0, 0, 0, 90, 0.5, 0.5, 0, 1);
                        }
                        
                        
                        TankHandler::GetSingleton().combo[(-1*tid)-1]+=(10/(TankHandler::GetSingleton().combo[(-1*tid)-1]/10+1));
                        
                        if(TankHandler::GetSingleton().combo[(-1*tid)-1]>TankHandler::GetSingleton().special[(-1*tid)-1])
                        {
                            TankHandler::GetSingleton().special[(-1*tid)-1]=TankHandler::GetSingleton().combo[(-1*tid)-1];
                            //App::GetSingleton().soundTask->PlaySound(1);
                        }
                        
                        TankHandler::GetSingleton().comboNum[(-1*tid)-1]++;
                    }
                    
                    if(tid<0 && spec && type1==2)
                    {
                        alive=true;
                        power+=100;
                    }
                    else
                        alive=false;
                    
                    FXHandler::GetSingleton().CreateFX(5, x, y, z, 0, .01, 0, 0, (*it).ry, 90, r, g, b, 1);
                    FXHandler::GetSingleton().CreateFX(5, x, y, z, 0, .01, 2, 0, ry, 90, r, g, b, 1);
                }
                
            }
            else
            {
                if(dT>.5)
                {
                    if((*it).energy<(*it).maxEnergy*2)
                    {
                        (*it).energy+=power/2;
                        alive=false;
                    }
                }
                
            }
            
        }
        
    }
    if(x>=LevelHandler::GetSingleton().sizeX || x <=0 ||  z>=LevelHandler::GetSingleton().sizeZ || z <=0)
    {
        alive=false;
        
        
        if(tid<0)
        {
            //TankHandler::GetSingleton().combo[(-1*tid)-1]+=(float)TankHandler::GetSingleton().hitCombo[(-1*tid)-1]/2;
            TankHandler::GetSingleton().hitCombo[(-1*tid)-1]=0;
        }
    }
    if(dT>=maxdT)
    {
        alive=false;
        if(tid<0)
        {
            //TankHandler::GetSingleton().combo[(-1*tid)-1]+=(float)TankHandler::GetSingleton().hitCombo[(-1*tid)-1]/2;
            TankHandler::GetSingleton().hitCombo[(-1*tid)-1]=0;
        }
    }
    
}

void Bullet::Draw()
{
    
    glColor3f(r, g, b);
    
    
    if(type1==2)
    {
        
        glPushMatrix();
        
        glTranslatef(x, y-.07, z);
        glRotatef(rx, 1, 0, 0);
        glRotatef(-ry, 0, 1, 0);
        glRotatef(rz, 0, 0, 1);
        
        glDisable(GL_TEXTURE_2D);
        
        //glCallList(App::GetSingleton().graphicsTask->alphalist[0]);
        
        glColor3f(r, g, b);
        
        glScalef(1, 1, 0.15);
        
        glCallList(App::GetSingleton().graphicsTask->squarelist2);
        
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        
        glColor4f(r2, g2, b2, 0.1f + power/500);
        
        glCallList(App::GetSingleton().graphicsTask->squarelist);
        
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        
        glPopMatrix();
        
        glPushMatrix();
        
        glTranslatef(x, y+.03, z);
        glRotatef(-ry, 0, 1, 0);
        
        glTranslatef(0, 0, -0.06);
        
        glRotatef(-60, 1, 0, 0);
        
        glDisable(GL_TEXTURE_2D);
        
        //glCallList(App::GetSingleton().graphicsTask->alphalist[0]);
        
        glColor3f(r, g, b);
        
        glScalef(1, 1, 0.2);
        
        glCallList(App::GetSingleton().graphicsTask->squarelist2);
        
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        
        glColor4f(r2, g2, b2, 0.1f + power/500);
        
        glCallList(App::GetSingleton().graphicsTask->squarelist);
        
        
        
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        
        glPopMatrix();
        
        glPushMatrix();
        
        glTranslatef(x, y+.03, z);
        glRotatef(-ry, 0, 1, 0);
        
        glTranslatef(0, 0, 0.06);
        
        glRotatef(60, 1, 0, 0);
        
        glDisable(GL_TEXTURE_2D);
        
        //glCallList(App::GetSingleton().graphicsTask->alphalist[0]);
        
        
        glColor3f(r, g, b);
        
        glScalef(1, 1, 0.2);
        
        glCallList(App::GetSingleton().graphicsTask->squarelist2);
        
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        
        glColor4f(r2, g2, b2, 0.1f + power/500);
        
        glCallList(App::GetSingleton().graphicsTask->squarelist);
        
        
        
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        
        glPopMatrix();
    }
    else
    {
        glPushMatrix();
        
        glTranslatef(x, y-.05, z);
        glRotatef(rx, 1, 0, 0);
        glRotatef(-ry, 0, 1, 0);
        glRotatef(rz, 0, 0, 1);
        
        glDisable(GL_TEXTURE_2D);
        
        //glCallList(App::GetSingleton().graphicsTask->alphalist[0]);
        
        glScalef(1, 1, 0.2);
        
        glCallList(App::GetSingleton().graphicsTask->squarelist2);
        
        
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        
        glColor4f(r2, g2, b2, 0.1f + power/1000);
        
        glCallList(App::GetSingleton().graphicsTask->squarelist);
        
        
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        
        glPopMatrix();
    }
    
}