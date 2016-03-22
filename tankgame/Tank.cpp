#include "Tank.h"

#include <OpenGL/gl.h>
#include "App.h"
#include "LevelHandler.h"
#include "FXHandler.h"
#include "TankHandler.h"


void Tank::SetType(TankType t1, TankType t2)
{
    type1=t1;
    type2=t2;
    
    movRate = 5;
    jumpCost = 250;
    
    jumpRate=18.0f;
    fallRate=16.0f;

    
    if(type1==TYPE_GREY)
    {
        fireRate=.5;
        
        attack=250;
        maxCharge=100;
        
        chargeRegen=50;
        
        moveCost=0;
        fireCost=maxCharge/4;
        chargeCost=200;
    }
    
    if(type1==TYPE_RED)
    {
        fireRate=.05;
        
        attack=100;
        maxCharge=100;
        
        chargeRegen=60;
        
        moveCost=0;
        fireCost=maxCharge/10;
        chargeCost=200;
        
        bounces=0;
        
        if(type2==TYPE_GREY)
        {
            fireRate=.05;
            attack=attack/2;
            fireCost*=2;
        }
        
        if(type2==TYPE_BLUE)
        {
            attack=200;
            fireCost=maxCharge/5;
            fireRate=.1;
        }
        
        if(type2==TYPE_YELLOW)
        {
            attack=120;
            fireCost=maxCharge/8;
            fireRate=.075;
            bounces=2;
        }
        
        if(type2==TYPE_PURPLE)
        {
            attack=200;
            fireCost=maxCharge/8;
            fireRate=.1;
        }
    }
    
    if(type1==TYPE_BLUE)
    {
        fireRate=0.05;
        
        attack=1100;
        maxCharge=100;
        
        chargeRegen=50;
        
        moveCost=0;
        fireCost=maxCharge;
        chargeCost=200;
        
        bounces=0;
        
        if(type2==TYPE_RED)
        {
            attack=750;
            fireCost=maxCharge/1.5;
            
        }
        
        if(type2==TYPE_YELLOW)
        {
            attack=800;
            fireCost=maxCharge/1.5;
            bounces=1;
        }
        
        if(type2==TYPE_PURPLE)
        {
            attack=500;
            fireCost=maxCharge/1.5;
        }
        
    }
    
    if(type1==TYPE_YELLOW)
    {
        fireRate=.5;
        
        attack=250;
        maxCharge=100;
        
        chargeRegen=50;
        
        moveCost=0;
        fireCost=maxCharge/2;
        chargeCost=200;
        
        bounces=8000;
        
        if(type2==1)
        {
            fireRate=.25;
            attack=150;
            fireCost=maxCharge/4;
            bounces=64;
        }
        
        if(type2==2)
        {
            fireRate=.8;
            attack=400;
            fireCost=maxCharge/2;
            bounces=32;
        }
        
        if(type2==4)
        {
            fireRate=.8;
            attack=500;
            fireCost=maxCharge/2;
            bounces=64;
        }
    }
    
    if(type1==TYPE_PURPLE)
    {
        fireRate=.5;
        
        attack=200;
        maxCharge=100;
        
        chargeRegen=50;
        
        moveCost=0;
        fireCost=maxCharge/2;
        chargeCost=200;
        
        bounces=0;
        
        if(type2==0)
        {
            fireRate=.25;
            attack=150;
            fireCost=maxCharge/4;
        }
        
        if(type2==1)
        {
            fireRate=.25;
            attack=150;
            fireCost=maxCharge/4;
        }
        
        if(type2==2)
        {
            fireRate=.8;
            attack=500;
            fireCost=maxCharge/2;
        }
        
        if(type2==3)
        {
            fireRate=.8;
            attack=500;
            fireCost=maxCharge/2;
            bounces=64;
        }
    }
    
    
    
    
    
    if(type1==0)
    {
        r=.5;
        g=.5;
        b=.5;
    }
    else if(type1==1)
    {
        r=1;
        g=0;
        b=0;
    }
    else if(type1==2)
    {
        r=0;
        g=0;
        b=1;
    }
    else if(type1==3)
    {
        r=1;
        g=1;
        b=0;
    }
    else if(type1==4)
    {
        r=1;
        g=0;
        b=1;
    }
    else if(type1==5)
    {
        r=0;
        g=1;
        b=0;
    }
    else if(type1==6)
    {
        r=.5;
        g=1;
        b=1;
    }
    else if(type1==7)
    {
        r=1;
        g=0;
        b=1;
    }
    
    
    if(type2==0)
    {
        r2=.5;
        g2=.5;
        b2=.5;
    }
    else if(type2==1)
    {
        r2=1;
        g2=0;
        b2=0;
    }
    else if(type2==2)
    {
        r2=0;
        g2=0;
        b2=1;
    }
    else if(type2==3)
    {
        r2=1;
        g2=1;
        b2=0;
    }
    else if(type2==4)
    {
        r2=1;
        g2=0;
        b2=1;
    }
    else if(type2==5)
    {
        r2=0;
        g2=1;
        b2=0;
    }
    else if(type2==6)
    {
        r2=.5;
        g2=1;
        b2=1;
    }
    else if(type2==7)
    {
        r2=1;
        g2=0;
        b2=1;
    }
    
}

void Tank::Die()
{
    
    if(!alive && deadtime<0.01)
    {
        if(id < 0)
        {
            App::GetSingleton().soundTask->PlaySound(7);
        }
        else
        {
            App::GetSingleton().soundTask->PlaySound(6);
        }
    }
    
    TankHandler::GetSingleton().attackingTanks--;
    
    if(TankHandler::GetSingleton().tanks.size()!=1 && id>=0)
    {
        LevelHandler::GetSingleton().AddItem(x, y+.2, z, type1);
    }
    
    if(deadtime < 0.01)
    {
        
        FXHandler::GetSingleton().CreateFX(6, x, y, z+.5,    0, .05, 0, 0, ry, 0, r, g, b, 1);
        
        FXHandler::GetSingleton().CreateFX(6, x, y, z+.5,    0, .05, 0, 0, ry, 0, r, g, b, .15);
        FXHandler::GetSingleton().CreateFX(6, x, y, z+.5,    0, .05, 2, 0, ry, 0, r, g, b, .15);
        
        FXHandler::GetSingleton().CreateFX(6, x+.5, y, z+.5, 0, .05, -3, 0, ry, 0, r, g, b, .15);
        FXHandler::GetSingleton().CreateFX(6, x+.5, y, z+.5, 0, .05, 2, 0, ry, 0, r, g, b, .15);
        
        FXHandler::GetSingleton().CreateFX(6, x+.5, y, z-.5, 3, .05, 0, 2, ry, 0, r2, g2, b2, .15);
        FXHandler::GetSingleton().CreateFX(6, x+.5, y, z-.5, 2, .05, 0, 0, ry, 0, r2, g2, b2, .15);
        
        FXHandler::GetSingleton().CreateFX(6, x+.5, y, z-.5, 2, .01, 0, 0, ry, 0, r2, g2, b2, .15);
        FXHandler::GetSingleton().CreateFX(6, x+.5, y, z-.5, 2, .05, 0, -2, ry, 0, r2, g2, b2, .15);
        
        FXHandler::GetSingleton().CreateFX(5, x, y, z+.5, 0, .01, 0, 0, ry, 90, r, g, b, 1);
         
        FXHandler::GetSingleton().CreateFX(5, x, y, z+.5, 0, .01, 0, 0, ry, 90, r, g, b, 1);
        FXHandler::GetSingleton().CreateFX(5, x, y, z+.5, 0, .01, 2, 0, ry, 90, r, g, b, 1);
         
        FXHandler::GetSingleton().CreateFX(5, x+.5, y, z+.5, 0, .01, -3, 0, ry, 90, r, g, b, 1);
        FXHandler::GetSingleton().CreateFX(5, x+.5, y, z+.5, 0, .01, 2, 0, ry, 90, r, g, b, 1);
         
        FXHandler::GetSingleton().CreateFX(5, x+.5, y, z-.5, 3, .01, 0, 2, ry, 90, r2, g2, b2, 1);
        FXHandler::GetSingleton().CreateFX(5, x+.5, y, z-.5, 2, .01, 0, 0, ry, 90, r2, g2, b2, 1);
         
        FXHandler::GetSingleton().CreateFX(5, x+.5, y, z-.5, 2, .01, 0, 0, ry, 90, r2, g2, b2, 1);
        FXHandler::GetSingleton().CreateFX(5, x+.5, y, z-.5, 2, .01, 0, -2, ry, 90, r2, g2, b2, 1);
        
        
    }
    if( TankHandler::GetSingleton().tanks.size()==1 && id>=0)
    {
        LevelHandler::GetSingleton().t[(int)x][(int)z]=-20;
    }
}

void Tank::Fire(float dTpressed)
{
    
    if(charge>=fireCost && fireTimer>fireRate)
    {
        
        
        float ratio = (double)(z-TankHandler::GetSingleton().players[0].z)/(double)(x-TankHandler::GetSingleton().players[0].x);
        float ryp=toDegrees(atan(ratio));
        
        if(TankHandler::GetSingleton().players[0].x<x)
        {
            ryp+=180;
        }
        
        ryp-=(TankHandler::GetSingleton().players[0].ry+TankHandler::GetSingleton().players[0].rty);
        
        float dist=sqrt((x-TankHandler::GetSingleton().players[0].x)*(x-TankHandler::GetSingleton().players[0].x)+(z-TankHandler::GetSingleton().players[0].z)*(z-TankHandler::GetSingleton().players[0].z));
        
        Mix_SetPosition(2, ryp, 10*(int)dist);
        
        App::GetSingleton().soundTask->PlaySound(2);
        
        
        Bullet temp;
        
        
        
        temp.tid=id;
        
        
        temp.x=x + (GlobalTimer::dT*temp.movRate) * (float)cos((rty+ry)*DTR);
        temp.y=y+.25;
        temp.z=z + (GlobalTimer::dT*temp.movRate) * (float)sin((rty+ry)*DTR);
        
        temp.rx=rtx+rx;
        temp.ry=rty+ry;
        temp.rz=rtz+rz;
        
        temp.power=attack;
        
        if(type1==4)
        {
            temp.dty=320*dTpressed;
            if(dTpressed<0)
            {
                dTpressed*=-1;
            }
            temp.power*=dTpressed;
            if(temp.power>1000)
            {
                temp.power=1000;
            }
            
            if(temp.power<100)
            {
                temp.power=100;
            }
        }
        else
            if(type2==4)
            {
                temp.power*=0.5f;
                temp.dty=160*dTpressed;
                if(dTpressed<0)
                {
                    dTpressed*=-1;
                }
                temp.power+=temp.power*dTpressed/2;
                if(temp.power>1000)
                {
                    temp.power=1000;
                }
                if(temp.power<100)
                {
                    temp.power=100;
                }
            }
        
        temp.r=r;
        temp.b=b;
        temp.g=g;
        
        temp.r2=r2;
        temp.b2=b2;
        temp.g2=g2;
        
        temp.maxbounces=bounces;
        
        temp.type1=type1;
        temp.type2=type2;
        
        
        temp.id=1;
        temp.alive=true;
        
        bullets.push_back(temp);
        
        fireTimer=0;
        
        charge-=fireCost;
    }
}


void Tank::Special(float dTpressed)
{
    
    if(TankHandler::GetSingleton().special[(-1*id)-1]>=fireCost/5 && fireTimer>fireRate)
    {
        
        
        float ratio = (double)(z-TankHandler::GetSingleton().players[0].z)/(double)(x-TankHandler::GetSingleton().players[0].x);
        float ryp=toDegrees(atan(ratio));
        
        if(TankHandler::GetSingleton().players[0].x<x)
        {
            ryp+=180;
        }
        
        ryp-=(TankHandler::GetSingleton().players[0].ry+TankHandler::GetSingleton().players[0].rty);
        
        float dist=sqrt((x-TankHandler::GetSingleton().players[0].x)*(x-TankHandler::GetSingleton().players[0].x)+(z-TankHandler::GetSingleton().players[0].z)*(z-TankHandler::GetSingleton().players[0].z));
        
        Mix_SetPosition(2, ryp, 10*(int)dist);
        
        App::GetSingleton().soundTask->PlaySound(2);
        
        
        Bullet temp;
        
        
        
        temp.tid=id;
        
        temp.spec=true;
        
        
        temp.x=x + (GlobalTimer::dT*temp.movRate) * (float)cos((rty+ry)*DTR);
        temp.y=y+.25;
        temp.z=z + (GlobalTimer::dT*temp.movRate) * (float)sin((rty+ry)*DTR);
        
        temp.rx=rtx+rx;
        temp.ry=rty+ry;
        temp.rz=rtz+rz;
        
        temp.power=attack;
        
        
        if(type1==4)
        {
            temp.dty=320*dTpressed;
            if(dTpressed<0)
                dTpressed*=-1;
            temp.power*=dTpressed;
            if(temp.power>1000)
                temp.power=1000;
            
            if(temp.power<100)
                temp.power=100;
        }
        else
            if(type2==4)
            {
                temp.dty=160*dTpressed;
                if(dTpressed<0)
                    dTpressed*=-1;
                temp.power*=dTpressed/2;
                if(temp.power>1000)
                    temp.power=1000;
                if(temp.power<100)
                    temp.power=100;
            }
        
        
        temp.r=r;
        temp.b=b;
        temp.g=g;
        
        temp.r2=r2;
        temp.b2=b2;
        temp.g2=g2;
        
        temp.maxbounces=bounces;
        
        temp.type1=type1;
        temp.type2=type2;
        
        
        temp.id=1;
        temp.alive=true;
        
        if(type1==1)
        {
            for(int i=0; i<10; i++)
            {
                
                Bullet temp2;
                
                temp2.tid=id;
                
		              temp2.x=x;
                temp2.y=y+.25;
                temp2.z=z;
                
                temp2.rx=rx;
                temp2.ry=
                temp2.rz=rz;
                
                temp2.rx=rtx+rx;
                temp2.ry=rty+ry+i*(270/10)+52;
                temp2.rz=rtz+rz;
                
                temp2.dty = temp.dty;
                
                temp2.power=temp.power;
                
                temp2.r=1;
                temp2.b=0;
                temp2.g=0;
                
                temp2.r2=.5;
                temp2.b2=.5;
                temp2.g2=.5;
                
                temp2.maxbounces=0;
                temp2.numbounces=0;
                
                temp2.type1=type1;
                temp2.type2=type2;
                
                temp2.maxdT=0.20;
                
                
                temp2.id=1;
                temp2.alive=true;
                
                bullets.push_back(temp2);
            }
            
            bullets.push_back(temp);
            
            temp.ry-=10;
            
            bullets.push_back(temp);
            
            temp.ry+=20;
            
            bullets.push_back(temp);
        }
        if(type1==2)
        {
            
            //temp.y+=.25;
            
            if(type2!=1)
            {
                temp.y+=.25;
            }
            
            bullets.push_back(temp);
            
            temp.y-=.25;
            
            temp.x+= .2 * (float)cos((rty+ry+90)*DTR);
            temp.y=y+.25;
            temp.z+= .2  * (float)sin((rty+ry+90)*DTR);
            
            if(type2==1)
            {
                temp.ry-=10;
            }
            
            bullets.push_back(temp);
            
            temp.x-= .4 * (float)cos((rty+ry+90)*DTR);
            temp.y=y+.25;
            temp.z-= .4  * (float)sin((rty+ry+90)*DTR);
            
            if(type2==1)
            {
                temp.ry+=20;
            }
            
            bullets.push_back(temp);
            
            //----
            
        }
        
        if(type1==3)
        {
            if(type2!=3)
            {
                temp.maxbounces=4;
            }
            
            bullets.push_back(temp);
            
            /*temp.ry-=90;
             
             bullets.push_back(temp);
             
             temp.ry+=180;
             
             bullets.push_back(temp);
             
             temp.ry-=45;
             
             bullets.push_back(temp);
             
             temp.ry-=90;
             
             bullets.push_back(temp);*/
        }
        
        if(type1==4)
        {
            
            bullets.push_back(temp);
            
            temp.ry-=90;
            
            bullets.push_back(temp);
            
            temp.ry+=180;
            
            bullets.push_back(temp);
            
            temp.ry+=90;
            
            bullets.push_back(temp);
        }
        
        
        fireTimer=0;
        
        TankHandler::GetSingleton().special[(-1*id)-1]-=fireCost/5;
        //energy-=0;
    }
}


bool Tank::PointCollision(float cx, float cy, float cz)
{
    bool result=false;
    if((cy-y)<.3 && (y-cy)<0)
    {
        if(sqrt((cx-x)*(cx-x)+(cz-z)*(cz-z))<size*2)
        {
            result=true;
        }
    }
    return result;
    
}

void Tank::Fall()
{
    
    
    vy -= (fallRate*GlobalTimer::dT);
    
    //vy=vy*GlobalTimer::dT;
    
    float dy=vy*GlobalTimer::dT;
    
    if(dy>10.0*GlobalTimer::dT)
    {
        dy=10.0*GlobalTimer::dT;
    }
    
    if(dy<-15.0*GlobalTimer::dT)
    {
        dy=-15.0*GlobalTimer::dT;
    }
    
    //vy=dy/GlobalTimer::dT;
    
    y+=dy;
    
    
    if(y>25)
    {
        y=25;
        vy=0;
    }
    
    if(LevelHandler::GetSingleton().FloatCollision(x, y+.2, z)
       ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[0],y+.2,z+collisionPoints[2])
       ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[3],y+.2,z+collisionPoints[5])
       ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[6],y+.2,z+collisionPoints[8])
       ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[9],y+.2,z+collisionPoints[11]))
    {
        y=(int)y+.8;
        vy=0;
    }
    
    
    int highest=-1;
    
    /*  if(LevelHandler::GetSingleton().f[(int)x][(int)z]>0)
     {
     highest=(float)LevelHandler::GetSingleton().f[(int)x][(int)z];
     }*/
    
    if(LevelHandler::GetSingleton().FloatCollision(x, y, z)
       ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[0],y,z+collisionPoints[2])
       ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[3],y,z+collisionPoints[5])
       ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[6],y,z+collisionPoints[8])
       ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[9],y,z+collisionPoints[11]) )
    {
        
        highest=LevelHandler::GetSingleton().f[(int)x][(int)z];
        
        if(highest<LevelHandler::GetSingleton().f[(int)(x+collisionPoints[0])][(int)(z+collisionPoints[2])])
        {
            highest=LevelHandler::GetSingleton().f[(int)(x+collisionPoints[0])][(int)(z+collisionPoints[2])];
        }
        if(highest<LevelHandler::GetSingleton().f[(int)(x+collisionPoints[3])][(int)(z+collisionPoints[5])])
        {
            highest=LevelHandler::GetSingleton().f[(int)(x+collisionPoints[3])][(int)(z+collisionPoints[5])];
        }
        if(highest<LevelHandler::GetSingleton().f[(int)(x+collisionPoints[6])][(int)(z+collisionPoints[8])])
        {
            highest=LevelHandler::GetSingleton().f[(int)(x+collisionPoints[6])][(int)(z+collisionPoints[8])];
        }
        if(highest<LevelHandler::GetSingleton().f[(int)(x+collisionPoints[9])][(int)(z+collisionPoints[11])])
        {
            highest=LevelHandler::GetSingleton().f[(int)(x+collisionPoints[9])][(int)(z+collisionPoints[11])];
        }
        
        
        y=(float)highest;
        
        if(!grounded)
        {
            if(charge<2*maxCharge/3)
            {
                charge+=maxCharge/3;
            }
            if(id<0)
            {
                App::GetSingleton().soundTask->PlaySound(5);
            }
        }
        jumping=false;
        grounded=true;
        jumpTime=0.0f;
        
        vy=0;
        
    }
    else
        if(LevelHandler::GetSingleton().FallCollision(x, y, z)
           ||LevelHandler::GetSingleton().FallCollision(x+collisionPoints[0],y,z+collisionPoints[2])
           ||LevelHandler::GetSingleton().FallCollision(x+collisionPoints[3],y,z+collisionPoints[5])
           ||LevelHandler::GetSingleton().FallCollision(x+collisionPoints[6],y,z+collisionPoints[8])
           ||LevelHandler::GetSingleton().FallCollision(x+collisionPoints[9],y,z+collisionPoints[11]))
        {
            //y-= vy;//(fallRate*GlobalTimer::dT);
            
            //Time to find the height of the highest collision point
            highest=LevelHandler::GetSingleton().t[(int)x][(int)z];
            
            if(highest<LevelHandler::GetSingleton().t[(int)(x+collisionPoints[0])][(int)(z+collisionPoints[2])])
            {
                highest=LevelHandler::GetSingleton().t[(int)(x+collisionPoints[0])][(int)(z+collisionPoints[2])];
            }
            if(highest<LevelHandler::GetSingleton().t[(int)(x+collisionPoints[3])][(int)(z+collisionPoints[5])])
            {
                highest=LevelHandler::GetSingleton().t[(int)(x+collisionPoints[3])][(int)(z+collisionPoints[5])];
            }
            if(highest<LevelHandler::GetSingleton().t[(int)(x+collisionPoints[6])][(int)(z+collisionPoints[8])])
            {
                highest=LevelHandler::GetSingleton().t[(int)(x+collisionPoints[6])][(int)(z+collisionPoints[8])];
            }
            if(highest<LevelHandler::GetSingleton().t[(int)(x+collisionPoints[9])][(int)(z+collisionPoints[11])])
            {
                highest=LevelHandler::GetSingleton().t[(int)(x+collisionPoints[9])][(int)(z+collisionPoints[11])];
            }
            
            //if(y>(float)highest)
            y=(float)highest;
            

            
            //y=(float)LevelHandler::t[(int)x][(int)z];
            if(highest<-10 && (id==-1 || id==-2))
            {
                LevelHandler::GetSingleton().NextLevel(true);
                x=LevelHandler::GetSingleton().start[0];
                z=LevelHandler::GetSingleton().start[1];
                y=24;
            }
            else
            {
                vy=0;
                if(!grounded)
                {
                    if(id<0)
                        App::GetSingleton().soundTask->PlaySound(5);
                }
            }
            if(!grounded)
            {
                if(charge<2*maxCharge/3)
                {
                    charge+=maxCharge/3;
                }
            }
            jumping=false;
            grounded=true;
            jumpTime=0.0f;
        }
        else
        {
            grounded=false;
        }
    
}


Tank::Tank()
{
    type1=TYPE_RED;
    type2=TYPE_BLUE;
    bonus=0;
    bonusTime=0.0f;
    deadtime=0.0f;
    jumpTime=0.0f;
    
}

void Tank::Init()
{
    inputmode=0;
    
    deadtime=0.0f;
    
    jumping=false;
    grounded=false;
    
    fireTimer=0;
    fireRate=.5;
    
    recharge=true;
    turbo=false;
    energy=1000;
    maxEnergy=1000;
    alive=true;
    charge=100;
    maxCharge=100;
    
    chargeRegen=50;
    energyRegen=100;
    
    moveCost=0;
    jumpCost=150;
    fireCost=maxCharge/2;
    chargeCost=200;
    
    bounces=2;
    
    size=0.2;
    
    collisionPoints[0]=size;
    collisionPoints[1]=0;
    collisionPoints[2]=size;
    
    collisionPoints[3]=-1*size;
    collisionPoints[4]= 0;
    collisionPoints[5]=-1*size;
    
    collisionPoints[6]=-1*size;
    collisionPoints[7]=0;
    collisionPoints[8]=size;
    
    collisionPoints[9]=size;
    collisionPoints[10]=0;
    collisionPoints[11]=-1*size;
    
    collisionPoints[12]=0;
    collisionPoints[13]=0;
    collisionPoints[14]=0;
    
    collisionPoints[15]=0;
    collisionPoints[16]=0;
    collisionPoints[17]=0;
    
    collisionPoints[18]=0;
    collisionPoints[19]=0;
    collisionPoints[20]=0;
    
    bullets.clear();
    while(!bulletq.empty())
    {
        bullets.push_back(bulletq.front());
        bulletq.pop();
    }
    
    x=LevelHandler::GetSingleton().start[0];
    y=24;
    z=LevelHandler::GetSingleton().start[1];;
    rx=0;
    ry=0;
    rz=0;
    rtx=0;
    rty=0;
    rtz=0;
    //	vx=0;
    //vy=0;
    //	vz=0;
    rotRate=75;
    movRate=5;
    fallRate=2;
    jumpRate=.3;
    
    
}


void Tank::RotBody(float rate)
{
    if(rate>1)
    {
        rate=1;
    }
    if(rate<-1)
    {
        rate=-1;
    }
    
    ry += rate*rotRate*GlobalTimer::dT;
}


void Tank::RotBody(bool forb)
{
    if(forb)
    {
        ry += rotRate*GlobalTimer::dT;
    }
    else
    {
        ry -= rotRate*GlobalTimer::dT;
    }
}

void Tank::RotTurret(float rate)
{
    float rtplus = rate*10*GlobalTimer::dT;
    
    if(rtplus>2.5)
    {
        rtplus=2.5;
    }
    if(rtplus<-2.5)
    {
        rtplus=-2.5;
    }
    
    rty += rtplus;
}

void Tank::RotBarrel(bool forb)
{
    
}

void Tank::Jump()
{
    
    if(charge>0)
    {
        jumpTime+=GlobalTimer::dT;
        
        charge-=jumpCost*GlobalTimer::dT;
        
        
        if(charge>5)
        {
            vy = (jumpRate * jumpTime);
        }
        
        //y += vy*GlobalTimer::dT;
        
        
        if(LevelHandler::GetSingleton().FloatCollision(x, y+.2, z)
           ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[0],y+.2,z+collisionPoints[2])
           ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[3],y+.2,z+collisionPoints[5])
           ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[6],y+.2,z+collisionPoints[8])
           ||LevelHandler::GetSingleton().FloatCollision(x+collisionPoints[9],y+.2,z+collisionPoints[11]))
        {
            y=(int)y+.8;
            vy=0;
        }
        
        
        //Jump damn it
        
        FXHandler::GetSingleton().CreateFX(1, x, y-.2, z, 0, .5*vy*GlobalTimer::dT, 0, rx, ry, rz, r, g, b, 1);
        if(!jumping && id<0)
        {
            App::GetSingleton().soundTask->PlaySound(4);
        }
        jumping=true;
    }
    
    //}
    //if(LevelHandler::Collision(x, y, z)||LevelHandler::FallCollision(x+.2,y,z+.2)||LevelHandler::FallCollision(x-.2,y,z-.2)||LevelHandler::FallCollision(x-.2,y,z+.2)||LevelHandler::FallCollision(x+.2,y,z-.2))
    //{
    //y+= (fallRate*GlobalTimer::dT);
    //	y-=vy;
    //}
    
}

void Tank::NextFrame()
{
    /*if(rty>360)
     rty=0;
     if(rty<0)
     rty=360;
     
     if(ry>360)
     ry=0;
     if(ry<0)
     ry=360;*/
    
    if(!jumping)
    {
        jumpTime=0.0f;
    }
    
    bonusTime+=GlobalTimer::dT;
    if(bonusTime>1)
    {
        bonus=0;
    }
    
    if(hitAlpha>0)
    {
        hitAlpha-=1*GlobalTimer::dT;
    }
    
    
    if(TankHandler::GetSingleton().hitCombo[(-1*id)-1]!=hitNum)
    {
        hitAlpha=1.0;
    }
    
    hitNum=TankHandler::GetSingleton().hitCombo[(-1*id)-1];
    
    
    if(energy>maxEnergy && !App::GetSingleton().gameTask->debug)
    {
        energy=maxEnergy;
    }
    
    if(energy<maxEnergy/2)
    {
        FXHandler::GetSingleton().CreateFX(2, x, y+.1, z, 0, .01, 0, 0, ry+rty, 90, .2, .2, .2, 1);
    }
    
    
    if(x>=128 || x<=0 || z>=128 || z<=0)
    {
        x=64;
        z=64;
    }
    if(y>=27)
    {
        y--;
        x=64;
        z=64;
    }
    
    if(energy<0 && alive)
    {
        alive=false;
        if(id<0 && App::GetSingleton().gameTask->versus)
        {
            if(id==-1)
            {
                TankHandler::GetSingleton().wins[1]++;
            }
            else
            {
                TankHandler::GetSingleton().wins[0]++;
            }
        }
    }
    else
    {
        fireTimer+=GlobalTimer::dT;
        Fall();
        if(charge<maxCharge && energy>maxEnergy/3 && recharge)
        {
            charge+=chargeRegen/2*GlobalTimer::dT;
            energy-=chargeCost*GlobalTimer::dT;
        }
        else if(energy>=maxEnergy && charge<maxCharge)
        {
            charge+=chargeRegen/2*GlobalTimer::dT;
            energy-=chargeCost*GlobalTimer::dT;
        }
        if(energy<maxEnergy)
        {
            energy+=energyRegen*GlobalTimer::dT;
        }
        if(charge<maxCharge)
        {
            charge+=chargeRegen/2*GlobalTimer::dT;
        }
        
        while(!bulletq.empty())
        {
            bullets.push_back(bulletq.front());
            bulletq.pop();
        }
        
        
        for(vector<Bullet>::iterator j = bullets.begin(); j != bullets.end();)
        {
            if(j->alive)
            {
                j->NextFrame();
                j++;
            }
            else
            {
                bullets.erase(j);
            }
        }
        
    }
}

bool Tank::Move(float rate)
{
    if(rate>1.25)
    {
        rate=1.25;
    }
    
    bool forb=true;
    
    if(rate<0)
    {
        forb=false;
    }
    
    bool moved;
    
    vx = rate*(GlobalTimer::dT*movRate) * (float)cos(ry*DTR);
    vz = rate*(GlobalTimer::dT*movRate) * (float)sin(ry*DTR);
    //float vy;
    
    
    x += vx;
    z += vz;
    
    moved=true;
    
    if(LevelHandler::GetSingleton().PointCollision(x,y,z)
       ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[0],y,z+collisionPoints[2])
       ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[3],y,z+collisionPoints[5])
       ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[6],y,z+collisionPoints[8])
       ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[9],y,z+collisionPoints[11]))
    {
        if (!forb)
        {
            vx=-1*vx;
            vz=-1*vz;
        }
        
        
        int kx=0;
        int kz=2;
        
        bool done=false;
        
        int which=0;
        if(LevelHandler::GetSingleton().PointCollision(x+collisionPoints[0],y,z+collisionPoints[2]))
        {
            FXHandler::GetSingleton().CreateFX(2, x-vx+collisionPoints[0], y, z-vz+collisionPoints[2], 0, 90, 90, r, g, b, 1);
            which=0;
        }
        else if(LevelHandler::GetSingleton().PointCollision(x+collisionPoints[3],y,z+collisionPoints[5]))
        {
            FXHandler::GetSingleton().CreateFX(2, x-vx+collisionPoints[3], y, z-vz+collisionPoints[5], 0, 90, 90, r, g, b, 1);
            which=1;
        }
        else if(LevelHandler::GetSingleton().PointCollision(x+collisionPoints[6],y,z+collisionPoints[8]))
        {
            FXHandler::GetSingleton().CreateFX(2, x-vx+collisionPoints[6], y, z-vz+collisionPoints[8], 0, 90, 90, r, g, b, 1);
            which=2;
        }
        else if(LevelHandler::GetSingleton().PointCollision(x+collisionPoints[9],y,z+collisionPoints[11]))
        {
            FXHandler::GetSingleton().CreateFX(2, x-vx+collisionPoints[9], y, z-vz+collisionPoints[11], 0, 90, 90, r, g, b, 1);
            which=3;
        }
        
        kx=which*3;
        kz=2+kx;
        
        x -= vx;
        z -= vz;
        
        if((int)(x+collisionPoints[kx]+vx)!=(int)(x+collisionPoints[kx]) && (int)(z+collisionPoints[kz]+vz)==(int)(z+collisionPoints[kz]) )
        {
            //FXHandler::GetSingleton().CreateFX(3, x, y, z, 0, 0, 0, 0, 0, 90, r, g, b, 1);
            z+=vz/2;
            done=true;
        }
        //else
        if((int)(x+collisionPoints[kx]+vx)==(int)(x+collisionPoints[kx]) && (int)(z+collisionPoints[kz]+vz)!=(int)(z+collisionPoints[kz]) )
        {
            //FXHandler::GetSingleton().CreateFX(3, x, y, z, 0, 0, 0, 0, 90, 90, r, g, b, 1);
            x+=vx/2;
            done=true;
        }
        //else
        if((int)(x+collisionPoints[kx]+vx)!=(int)(x+collisionPoints[kx]) && (int)(z+collisionPoints[kz]+vz)!=(int)(z+collisionPoints[kz]))
        {
            if(LevelHandler::GetSingleton().PointCollision((x+vx+collisionPoints[kx]),y,z+collisionPoints[kz]) && !LevelHandler::GetSingleton().PointCollision(x+collisionPoints[kx],y,z+vz+collisionPoints[kz]))
            {
                //FXHandler::GetSingleton().CreateFX(3, x, y, z, 0, 0, 0, 0, 0, 90, r, g, b, 1);
                z+=vz/2;
                done=true;
            }
            else
            {
                //FXHandler::GetSingleton().CreateFX(3, x, y, z, 0, 0, 0, 0, 90, 90, r, g, b, 1);
                x+=vx/2;
                done=true;
            }
        }
        
        if(LevelHandler::GetSingleton().PointCollision(x+collisionPoints[0],y,z+collisionPoints[2])
		         ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[3],y,z+collisionPoints[5])
		         ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[6],y,z+collisionPoints[8])
		         ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[9],y,z+collisionPoints[11]))
        {
            x-=vx;
            z-=vz;
        }
        
        moved=false;
    }
    
    charge-=rate*moveCost*GlobalTimer::dT;
    
    if(x>=128 || x<=0 || z>=128 || z<=0)
    {
        x=64;
        z=64;
    }
    
    return moved;
    
}


bool Tank::Move(bool forb)
{
    bool moved;
    
    vx = (GlobalTimer::dT*movRate) * (float)cos(ry*DTR);
    vz = (GlobalTimer::dT*movRate) * (float)sin(ry*DTR);
    //float vy;
    
    if (forb)
    {
        x += vx;
        z += vz;
    }
    else
    {
        x -= vx;
        z -= vz;
    }
    moved=true;
    
    if(LevelHandler::GetSingleton().PointCollision(x,y,z)
       ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[0],y,z+collisionPoints[2])
       ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[3],y,z+collisionPoints[5])
       ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[6],y,z+collisionPoints[8])
       ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[9],y,z+collisionPoints[11]))
    {
        if (!forb)
        {
            vx=-1*vx;
            vz=-1*vz;
        }
        
        
        int kx=0;
        int kz=2;
        
        bool done=false;
        
        int which=0;
        if(LevelHandler::GetSingleton().PointCollision(x+collisionPoints[0],y,z+collisionPoints[2]))
        {
            FXHandler::GetSingleton().CreateFX(2, x-vx+collisionPoints[0], y, z-vz+collisionPoints[2], 0, 90, 90, r, g, b, 1);
            which=0;
        }
        else if(LevelHandler::GetSingleton().PointCollision(x+collisionPoints[3],y,z+collisionPoints[5]))
        {
            FXHandler::GetSingleton().CreateFX(2, x-vx+collisionPoints[3], y, z-vz+collisionPoints[5], 0, 90, 90, r, g, b, 1);
            which=1;
        }
        else if(LevelHandler::GetSingleton().PointCollision(x+collisionPoints[6],y,z+collisionPoints[8]))
        {
            FXHandler::GetSingleton().CreateFX(2, x-vx+collisionPoints[6], y, z-vz+collisionPoints[8], 0, 90, 90, r, g, b, 1);
            which=2;
        }
        else if(LevelHandler::GetSingleton().PointCollision(x+collisionPoints[9],y,z+collisionPoints[11]))
        {
            FXHandler::GetSingleton().CreateFX(2, x-vx+collisionPoints[9], y, z-vz+collisionPoints[11], 0, 90, 90, r, g, b, 1);
            which=3;
        }
        
        kx=which*3;
        kz=2+kx;
        
        x -= vx;
        z -= vz;
        
        if((int)(x+collisionPoints[kx]+vx)!=(int)(x+collisionPoints[kx]) && (int)(z+collisionPoints[kz]+vz)==(int)(z+collisionPoints[kz]) )
        {
            //FXHandler::GetSingleton().CreateFX(3, x, y, z, 0, 0, 0, 0, 0, 90, r, g, b, 1);
            z+=vz/2;
            //done=true;
        }
        //else
        if((int)(x+collisionPoints[kx]+vx)==(int)(x+collisionPoints[kx]) && (int)(z+collisionPoints[kz]+vz)!=(int)(z+collisionPoints[kz]) )
        {
            //FXHandler::GetSingleton().CreateFX(3, x, y, z, 0, 0, 0, 0, 90, 90, r, g, b, 1);
            x+=vx/2;
            //done=true;
        }
        //else
        if((int)(x+collisionPoints[kx]+vx)!=(int)(x+collisionPoints[kx]) && (int)(z+collisionPoints[kz]+vz)!=(int)(z+collisionPoints[kz]))
        {
            if(LevelHandler::GetSingleton().PointCollision((x+vx+collisionPoints[kx]),y,z+collisionPoints[kz]) && !LevelHandler::GetSingleton().PointCollision(x+collisionPoints[kx],y,z+vz+collisionPoints[kz]))
            {
                //FXHandler::GetSingleton().CreateFX(3, x, y, z, 0, 0, 0, 0, 0, 90, r, g, b, 1);
                z+=vz/2;
                //done=true;
            }
            else
            {
                //FXHandler::GetSingleton().CreateFX(3, x, y, z, 0, 0, 0, 0, 90, 90, r, g, b, 1);
                x+=vx/2;
                //done=true;
            }
        }
        
        if(LevelHandler::GetSingleton().PointCollision(x+collisionPoints[0],y,z+collisionPoints[2])
		         ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[3],y,z+collisionPoints[5])
		         ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[6],y,z+collisionPoints[8])
		         ||LevelHandler::GetSingleton().PointCollision(x+collisionPoints[9],y,z+collisionPoints[11]))
        {
            x-=vx;
            z-=vz;
        }
        
        moved=false;
        
        //LevelHandler::GetSingleton().HandlePointCollision(x, y, vx, vy, vz);
    }
    
    if(x>=128 || x<=0 || z>=128 || z<=0)
    {
        x=64;
        z=64;
    }
    
    charge-=moveCost*GlobalTimer::dT;
    
    return moved;
    
}



void Tank::HandleInput()
{
    static float firePressed=0;
    
    //static string jname(SDL_JoystickName(jid));
    
    //static bool PS2=false;
    /*
     if(SDL_JoystickNumAxes(InputTask::joysticks[jid])==4 && SDL_JoystickNumButtons(InputTask::joysticks[jid])==12 && jname.rfind("P880")==string::npos)
     PS2=true;
     else
     PS2=false;*/
    
    
    if(inputmode==0)
    {
        RotTurret(InputTask::dX*2);
        
        if((type1==TYPE_PURPLE || type2==TYPE_PURPLE) && InputTask::MouseStillDown(1))
        {
            Fire(InputTask::dX*GlobalTimer::dT);
        }
        else if( InputTask::MouseStillDown(1))
        {
            Fire(1);
        }
        
        if((type1==TYPE_PURPLE || type2==TYPE_PURPLE) && InputTask::MouseStillDown(3))
        {
            Special(InputTask::dX*GlobalTimer::dT);
        }
        else if( InputTask::MouseStillDown(3))
        {
            Special(1);
        }
        if( InputTask::KeyDown(SDL_SCANCODE_T) )
        {
            float oldy;
            
            oldy=y;
            
            LevelHandler::GetSingleton().NextLevel(true);
            x=LevelHandler::GetSingleton().start[0];
            z=LevelHandler::GetSingleton().start[1];
            y=oldy;
        }
        
        if( InputTask::KeyStillDown(SDL_SCANCODE_W) )//|| SDL_JoystickGetHat(App::GetSingleton().inputTask->joysticks[jid], 0) == SDL_HAT_UP)
        {
            Move(true);
            if(turbo)
            {
                Move(true);
                charge-=jumpCost*GlobalTimer::dT;
            }
            
        }
        if( InputTask::KeyStillDown(SDL_SCANCODE_A) )
        {
            RotBody(false);
            if(turbo)
            {
                RotBody(false);
                charge-=jumpCost*GlobalTimer::dT;
            }
        }
        if( InputTask::KeyStillDown(SDL_SCANCODE_D) )
        {
            RotBody(true);
            if(turbo)
            {
                RotBody(true);
                charge-=jumpCost*GlobalTimer::dT;
            }
        }
        if( InputTask::KeyStillDown(SDL_SCANCODE_S))
        {
            Move(false);
            if(turbo)
            {
                Move(false);
                charge-=jumpCost*GlobalTimer::dT;
            }
        }
        
        if(InputTask::KeyStillDown(SDL_SCANCODE_SPACE))
        {
            Jump();
            if(turbo)
            {
                Jump();
            }
        }
        else
        {
            jumping=false;
        }
        if( InputTask::KeyStillDown(SDL_SCANCODE_LEFT) )
        {
            RotTurret(-1);
            if(turbo)
            {
                RotTurret(-1);
                charge-=jumpCost*GlobalTimer::dT;
            }
        }
        if( InputTask::KeyStillDown(SDL_SCANCODE_RIGHT))
        {
            RotTurret(1);
            if(turbo)
            {
                RotTurret(1);
                charge-=jumpCost*GlobalTimer::dT;
            }
        }
        
        if( InputTask::CurKey(SDL_SCANCODE_LSHIFT) && charge>0 )
        {
            turbo=true;
        }
        else
            turbo=false;
        
        if( InputTask::KeyStillDown(SDL_SCANCODE_UP) )
        {
            App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist+=10*GlobalTimer::dT;
        }
        if( InputTask::KeyStillDown(SDL_SCANCODE_DOWN))
        {
            App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist-=10*GlobalTimer::dT;
        }
        
        if( InputTask::KeyDown(SDL_SCANCODE_C))
        {
            if(App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist>20)
            {
                App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=0.1;
            }
            else
                if(App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist<=0.1)
                {
                    App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=1.5;
                }
                else
                    if(App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist<=1.5)
                    {
                        App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=2.0;
                    }
                    else
                        if(App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist<=2.0)
                        {
                            App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=4.0;
                        }
                        else
                            if(App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist<=4.0)
                            {
                                App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=8.0;
                            }
                            else
                                if(App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist<=8.0)
                                {
                                    App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=20.2;
                                }
        }
        
        
    }
    else if(inputmode>0 && inputmode<4) //Jay's flight stick + PS2, P880 stupid mode
    {
        if((type1==TYPE_PURPLE || type2==TYPE_PURPLE) && (InputTask::GetButton(jid,0) || InputTask::GetButton(jid,3) || InputTask::GetButton(jid,7)) )
        {
            Fire((float)InputTask::GetAxis(jid,3)/(float)3200);
        }
        else
            if( InputTask::GetButton(jid,3) || InputTask::GetButton(jid,0) || InputTask::GetButton(jid,7) )//|| InputTask::GetButton(jid,0))
            {
                Fire(1);
            }
        
        if((type1==4 || type2==4) && (InputTask::GetButton(jid,5) || InputTask::GetButton(jid,11)) )
        {
            Special((float)InputTask::GetAxis(jid,3)/(float)3200);
            /*if( InputTask::MouseStillDown(1))
             {
             firePressed+=GlobalTimer::dT;
             }
             else if(InputTask::MouseUp(1) )
             {
             Fire(firePressed);
             firePressed=0;
             }*/
        }
        else if( InputTask::GetButton(jid,5) || InputTask::GetButton(jid,11) )
        {
            Special(1);
        }
        
        
        if( InputTask::GetAxis(jid,1) < -5000 ) //SDL_JoystickGetButton(App::GetSingleton().inputTask->joysticks[jid], 2) )//InputTask::GetAxis(jid,0)==0)//|| SDL_JoystickGetHat(App::GetSingleton().inputTask->joysticks[jid], 0) == SDL_HAT_UP)
        {
            Move(-1.0f*(float)InputTask::GetAxis(jid,1)/(float)20000);//true);
            if(turbo && charge>0)
            {
                Move(true);
                charge-=jumpCost*GlobalTimer::dT;
            }
            
        }
        if( InputTask::GetAxis(jid,0) < -8000)
        {
            RotBody((float)InputTask::GetAxis(jid,0)/(float)32000); //RotBody(false);
        }
        if( InputTask::GetAxis(jid,0) > 8000)
        {
            RotBody((float)InputTask::GetAxis(jid,0)/(float)32000);//RotBody(true);
            
        }
        if( InputTask::GetAxis(jid,1) > 8000)
        {
            Move(false);
            if(turbo && charge>0)
            {
                Move(false);
                charge-=jumpCost*GlobalTimer::dT;
            }
        }
        
        if( InputTask::GetButton(jid,1) || InputTask::GetButton(jid,2) || InputTask::GetButton(jid,6))
        {
            Jump();
            if(turbo && charge>0)
            {
                Jump();
            }
        }
        if(inputmode==1)
        {
            if( InputTask::GetAxis(jid,2) < -6000 )
            {
                RotTurret((float)InputTask::GetAxis(jid,2)/(float)3200);
            }
            if( InputTask::GetAxis(jid,2) > 6000)
            {
                RotTurret((float)InputTask::GetAxis(jid,2)/(float)3200);
            }
        }
        else
        {
            if( InputTask::GetAxis(jid,3) < -5000 )
            {
                RotTurret((float)InputTask::GetAxis(jid,3)/(float)3200);
            }
            if( InputTask::GetAxis(jid,3) > 5000)
            {
                RotTurret((float)InputTask::GetAxis(jid,3)/(float)3200);
            }
        }
        
        if(charge>0 && (InputTask::GetButton(jid,4) || InputTask::GetButton(jid,10)) )
        {
            turbo=true;
        }
        else
        {
            turbo=false;
        }
        
        /* if( InputTask::GetButton(jid,11))
         {
         if(App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist>20)
         {
         App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=0.8;
         App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=1.0;
         }
         else
         if(App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist<1)
         {
         App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=1.2;
         App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=2;
         }
         else
         if(App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist<2)
         {
         App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=3.2;
         App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=2.2;
         }
         else
         if(App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist<4)
         {
         App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=20.2;
         App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=0.2;
         }
         }*/
        
        if( SDL_JoystickGetHat(InputTask::joysticks[jid], 0) == SDL_HAT_UP)
        {
            App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=0.8;
            App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=1.0;
        }
        else
            if(SDL_JoystickGetHat(InputTask::joysticks[jid], 0) ==  SDL_HAT_DOWN)
            {
                App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=1.2;
                App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=2;
            }
            else
                if( SDL_JoystickGetHat(InputTask::joysticks[jid], 0) ==  SDL_HAT_LEFT)
                {
                    App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=3.2;
                    App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=2.2;
                }
                else
                    if( SDL_JoystickGetHat(InputTask::joysticks[jid], 0) ==  SDL_HAT_RIGHT)
                    {
                        App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=20.2;
                        App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=0.2;
                    }
        
    }
    /*   else if(inputmode==1) //Gravis Differential Drive
     {
     
     if( InputTask::GetButton(jid,7) )//|| InputTask::GetButton(jid,0))
     {
		   Fire(1);
     }
     
     if( InputTask::GetAxis(jid,2) < -8000 && InputTask::GetAxis(jid,1) < -8000 )
     {
		   Move(true);//(float)-1*InputTask::GetAxis(jid,2)/32000);
     if(turbo)
     {
     Move(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     
     }
     else
     if( InputTask::GetAxis(jid,2) > 8000 && InputTask::GetAxis(jid,1) > 8000)
     {
		   Move(false);
     if(turbo)
     {
     RotBody(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     else
     if( InputTask::GetAxis(jid,1) > 8000 && InputTask::GetAxis(jid,2) < -8000)
     {
		   RotBody(false);
     if(turbo)
     {
     RotBody(false);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     else
     if( InputTask::GetAxis(jid,1) < -8000 && InputTask::GetAxis(jid,2) > 8000) //SDL_JoystickGetButton(App::GetSingleton().inputTask->joysticks[jid], 2) )//InputTask::GetAxis(jid,0)==0)//|| SDL_JoystickGetHat(App::GetSingleton().inputTask->joysticks[jid], 0) == SDL_HAT_UP)
     {
		   RotBody(true);
     if(turbo)
     {
     RotBody(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     
     }
     else
     if(InputTask::GetAxis(jid,1) < -8000)
     {
     Move((float)-1*InputTask::GetAxis(jid,1)/32000);
     RotBody(true);
     if(turbo)
     {
     Move(true);
     RotBody(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     else
     if(InputTask::GetAxis(jid,1) > 8000)
     {
     Move((float)-1*InputTask::GetAxis(jid,1)/32000);
     RotBody(false);
     if(turbo)
     {
     Move(false);
     RotBody(false);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     else
     if(InputTask::GetAxis(jid,2) < -8000)
     {
     Move((float)-1*InputTask::GetAxis(jid,2)/32000);
     RotBody(false);
     if(turbo)
     {
     Move(true);
     RotBody(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     else
     if(InputTask::GetAxis(jid,2) > 8000)
     {
     Move((float)-1*InputTask::GetAxis(jid,2)/32000);
     RotBody(true);
     if(turbo)
     {
     Move(false);
     RotBody(false);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     
     
     if( InputTask::GetAxis(jid,3) < -8000 )
     {
     
		   RotTurret((float)InputTask::GetAxis(jid,3)/(float)32000);
     if(turbo)
     {
     RotTurret(InputTask::GetAxis(jid,3)/8000);
     charge-=jumpCost*GlobalTimer::dT;
     }
     
     }
     if( InputTask::GetAxis(jid,3) > 8000)
     {
     
		   RotTurret((float)InputTask::GetAxis(jid,3)/(float)32000);
     if(turbo)
     {
     RotTurret(1);
     charge-=jumpCost*GlobalTimer::dT;
     }
     
     }
     
     if( InputTask::GetButton(jid,6))
     {
		   Jump();
     if(turbo)
     {
     Jump();
     }
     }
     
     }
     else if(inputmode==2) //Gravis Stupid
     {
     
     if(  InputTask::GetButton(jid,7) )//|| InputTask::GetButton(jid,0))
     {
		   Fire(1);
     }
     
     if( InputTask::GetAxis(jid,1) < -8000 ) //SDL_JoystickGetButton(App::GetSingleton().inputTask->joysticks[jid], 2) )//InputTask::GetAxis(jid,0)==0)//|| SDL_JoystickGetHat(App::GetSingleton().inputTask->joysticks[jid], 0) == SDL_HAT_UP)
     {
		   Move(true);
     if(turbo)
     {
     Move(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     
     }
     if( InputTask::GetAxis(jid,0) < -8000)
     {
		   RotBody(false);
     if(turbo)
     {
     RotBody(false);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     if( InputTask::GetAxis(jid,0) > 8000)
     {
		   RotBody(true);
     if(turbo)
     {
     RotBody(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     if( InputTask::GetAxis(jid,1) > 8000)
     {
		   Move(false);
     if(turbo)
     {
     Move(false);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     
     if( InputTask::GetButton(jid,6))
     {
		   Jump();
     if(turbo)
     {
     Jump();
     }
     }
     if( InputTask::GetAxis(jid,3) < -8000 )
     {
		   RotTurret(InputTask::GetAxis(jid,3)/16000);
     if(turbo)
     {
     RotTurret(InputTask::GetAxis(jid,3)/8000);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     if( InputTask::GetAxis(jid,3) > 8000)
     {
		   RotTurret(1*InputTask::GetAxis(jid,3)/16000);
     if(turbo)
     {
     RotTurret(1);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     }
     else if(inputmode==3) //Broken Differential drive PS2
     {
     
     if( InputTask::GetButton(jid,7) )//|| InputTask::GetButton(jid,0))
     {
		   Fire(1);
     }
     
     if( InputTask::GetAxis(jid,3) < -7000 && InputTask::GetAxis(jid,1) < -7000 )
     {
		   Move(true);//(float)-1*InputTask::GetAxis(jid,3)/32000);
     if(InputTask::GetAxis(jid,1) > InputTask::GetAxis(jid,3) )
     {
     RotBody((float)(InputTask::GetAxis(jid,1)/60000.0f));//-InputTask::GetAxis(jid,3)/32000.0f));
     }
     if(InputTask::GetAxis(jid,3) > InputTask::GetAxis(jid,1))
     {
     RotBody(-1*(float)(InputTask::GetAxis(jid,3)/60000.0f));//-InputTask::GetAxis(jid,3)/32000.0f));
     }
     if(turbo)
     {
     Move(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     
     }
     else
     if( InputTask::GetAxis(jid,3) > 8000 && InputTask::GetAxis(jid,1) > 8000)
     {
		   Move(false);
     if(InputTask::GetAxis(jid,1) < InputTask::GetAxis(jid,3) )
     {
     RotBody((float)(InputTask::GetAxis(jid,1)/80000.0f));//-InputTask::GetAxis(jid,3)/32000.0f));
     }
     if(InputTask::GetAxis(jid,3) < InputTask::GetAxis(jid,1))
     {
     RotBody(-1*(float)(InputTask::GetAxis(jid,3)/80000.0f));//-InputTask::GetAxis(jid,3)/32000.0f));
     }
     if(turbo)
     {
     RotBody(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     else
     if( InputTask::GetAxis(jid,1) > 8000 && InputTask::GetAxis(jid,3) < -8000)
     {
		   RotBody(false);
     if(turbo)
     {
     RotBody(false);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     else
     if( InputTask::GetAxis(jid,1) < -8000 && InputTask::GetAxis(jid,3) > 8000) //SDL_JoystickGetButton(App::GetSingleton().inputTask->joysticks[jid], 2) )//InputTask::GetAxis(jid,0)==0)//|| SDL_JoystickGetHat(App::GetSingleton().inputTask->joysticks[jid], 0) == SDL_HAT_UP)
     {
		   RotBody(true);
     if(turbo)
     {
     RotBody(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     
     }
     else
     if(InputTask::GetAxis(jid,1) < -8000)
     {
     Move((float)-1*InputTask::GetAxis(jid,1)/32000);
     RotBody(true);
     if(turbo)
     {
     Move(true);
     RotBody(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     else
     if(InputTask::GetAxis(jid,1) > 8000)
     {
     Move((float)-1*InputTask::GetAxis(jid,1)/32000);
     RotBody(false);
     if(turbo)
     {
     Move(false);
     RotBody(false);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     else
     if(InputTask::GetAxis(jid,3) < -8000)
     {
     Move((float)-1*InputTask::GetAxis(jid,3)/32000);
     RotBody(false);
     if(turbo)
     {
     Move(true);
     RotBody(true);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     else
     if(InputTask::GetAxis(jid,3) > 8000)
     {
     Move((float)-1*InputTask::GetAxis(jid,3)/32000);
     RotBody(true);
     if(turbo)
     {
     Move(false);
     RotBody(false);
     charge-=jumpCost*GlobalTimer::dT;
     }
     }
     
     
     if( InputTask::GetAxis(jid,2) < -8000 )
     {
     
		   RotTurret((float)InputTask::GetAxis(jid,2)/(float)32000);
     if(turbo)
     {
     RotTurret(InputTask::GetAxis(jid,2)/8000);
     charge-=jumpCost*GlobalTimer::dT;
     }
     
     }
     if( InputTask::GetAxis(jid,2) > 8000)
     {
     
		   RotTurret((float)InputTask::GetAxis(jid,2)/(float)32000);
     if(turbo)
     {
     RotTurret(1);
     charge-=jumpCost*GlobalTimer::dT;
     }
     
     }
     
     if( InputTask::GetButton(jid,6))
     {
		   Jump();
     if(turbo)
     {
     Jump();
     }
     }
     
     }*/
    else if(inputmode==4) //NGC
    {
        if( InputTask::GetButton(jid,0) || InputTask::GetButton(jid,7) || InputTask::GetAxis(jid,3) > -5000 )
        {
            if(type1==4 || type2==4)
            {
                Fire((float)InputTask::GetAxis(jid,2)/(float)6400);
            }
            else
            {
                Fire(1);
            }
        }
        
        if(InputTask::GetButton(jid,5) || InputTask::GetButton(jid,6))
        {
            if(type1==4 || type2==4)
            {
                Special((float)InputTask::GetAxis(jid,2)/(float)6000);
            }
            else
            {
                Special(1);
            }
            
            /*if( InputTask::MouseStillDown(1))
             {
             firePressed+=GlobalTimer::dT;
             }
             else if(InputTask::MouseUp(1) )
             {
             Fire(firePressed);
             firePressed=0;
             }*/
        }
        
        if( InputTask::GetAxis(jid,1) < -4000 ) //SDL_JoystickGetButton(App::GetSingleton().inputTask->joysticks[jid], 2) )//InputTask::GetAxis(jid,0)==0)//|| SDL_JoystickGetHat(App::GetSingleton().inputTask->joysticks[jid], 0) == SDL_HAT_UP)
        {
            Move(-1.0f*(float)InputTask::GetAxis(jid,1)/(float)18000);//true);
            if(turbo)
            {
                Move(true);
                charge-=jumpCost*GlobalTimer::dT;
            }
            
        }
        if( InputTask::GetAxis(jid,0) < -8000)
        {
            RotBody((float)InputTask::GetAxis(jid,0)/(float)28000); //RotBody(false);
        }
        if( InputTask::GetAxis(jid,0) > 8000)
        {
            RotBody((float)InputTask::GetAxis(jid,0)/(float)28000);//RotBody(true);
            
        }
        if( InputTask::GetAxis(jid,1) > 8000)
        {
            Move(false);
            if(turbo)
            {
                Move(false);
                charge-=jumpCost*GlobalTimer::dT;
            }
        }
        
        if( InputTask::GetButton(jid,1) )
        {
            Jump();
            if(turbo)
            {
                Jump();
            }
        }
        
        if( InputTask::GetAxis(jid,2) < -6000 )
        {
            RotTurret((float)InputTask::GetAxis(jid,2)/(float)2500);
        }
        if( InputTask::GetAxis(jid,2) > 6000)
        {
            RotTurret((float)InputTask::GetAxis(jid,2)/(float)2500);
        }
        
        /* if( InputTask::GetAxis(jid,3) > -5000 )
         {
         Fire(1);
         //RotTurret((float)InputTask::GetAxis(jid,3)/(float)3200);
         }*/
        if( InputTask::GetAxis(jid,4) > 5000)
        {
            Jump();
            if(turbo)
            {
                Jump();
            }
        }
        
        if(charge>0 && (InputTask::GetButton(jid,3) || InputTask::GetButton(jid,2)) )
        {
            turbo=true;
        }
        else
        {
            turbo=false;
        }
        
        
        if( SDL_JoystickGetHat(InputTask::joysticks[jid], 0) == SDL_HAT_UP)
        {
            App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=0.8;
            App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=1.0;
        }
        else
            if(SDL_JoystickGetHat(InputTask::joysticks[jid], 0) ==  SDL_HAT_DOWN)
            {
                App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=1.2;
                App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=2;
            }
            else
                if( SDL_JoystickGetHat(InputTask::joysticks[jid], 0) ==  SDL_HAT_LEFT)
                {
                    App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=3.2;
                    App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=2.2;
                }
                else
                    if( SDL_JoystickGetHat(InputTask::joysticks[jid], 0) ==  SDL_HAT_RIGHT)
                    {
                        App::GetSingleton().graphicsTask->cams[-1*(id+1)].ydist=20.2;
                        App::GetSingleton().graphicsTask->cams[-1*(id+1)].xzdist=0.2;
                    }
        
        
    }
    
    
    if( InputTask::KeyDown(SDL_SCANCODE_I) && App::GetSingleton().gameTask->debug )
    {
        TankHandler::GetSingleton().special[0]+=100;
        
        energy+=maxEnergy*20;
        charge+=maxCharge*20;
    }
    
    if( InputTask::KeyDown(SDL_SCANCODE_HOME) && App::GetSingleton().gameTask->debug )
    {
        TankHandler::GetSingleton().special[0]=10;
        
        energy=maxEnergy;
        charge=maxCharge;
    }
    
    if( InputTask::KeyDown(SDL_SCANCODE_K) && inputmode!=0)
    {
        inputmode=0;
    }
    
    
}

void Tank::AI()
{
    static int state=2;
    bool p2target=false;
    
    float angle=30;
    float frames=10;
    float xpp = x + (GlobalTimer::dT*frames*movRate) * (float)cos((ry-angle)*DTR);
    float zpp = z + (GlobalTimer::dT*frames*movRate) * (float)sin((ry-angle)*DTR);
    
    
    float xpp2 = x + (GlobalTimer::dT*frames*movRate) * (float)cos((ry+angle)*DTR);
    float zpp2 = z + (GlobalTimer::dT*frames*movRate) * (float)sin((ry+angle)*DTR);
    
    dist=sqrt((x-TankHandler::GetSingleton().players[0].x)*(x-TankHandler::GetSingleton().players[0].x)+(z-TankHandler::GetSingleton().players[0].z)*(z-TankHandler::GetSingleton().players[0].z));
    
    if(TankHandler::GetSingleton().numPlayers>1 && TankHandler::GetSingleton().players[1].alive)
    {
        float dist2=sqrt((x-TankHandler::GetSingleton().players[1].x)*(x-TankHandler::GetSingleton().players[1].x)+(z-TankHandler::GetSingleton().players[1].z)*(z-TankHandler::GetSingleton().players[1].z));
        
        if(dist2<dist || !TankHandler::GetSingleton().players[1].alive)
        {
            p2target=true;
            dist=dist2;
        }
    }
    
    if(!TankHandler::GetSingleton().players[1].alive)
        p2target=false;
    
    
    /*if(LevelHandler::GetSingleton().PointCollision(xpp,y,zpp) || LevelHandler::GetSingleton().PointCollision(xpp2,y,zpp2))
     {
     state=0;
     }
     else
     {
     state=5;
     }*/
    
    if(dist>(15+3*(LevelHandler::GetSingleton().levelNumber-48)))
    {
        state=0;
    }
    else
    {
        if(energy>(maxEnergy/2) && !App::GetSingleton().gameTask->versus)
            state=5;
        else
            if((LevelHandler::GetSingleton().levelNumber-48)<2)
                state=5;
            else
                state=1;
    }
    
    if(TankHandler::GetSingleton().tanks.size()==1 || (TankHandler::GetSingleton().attackingTanks < (LevelHandler::GetSingleton().levelNumber-47) && !App::GetSingleton().gameTask->versus) )
    {
        state=5;
        TankHandler::GetSingleton().attackingTanks++;
    }
    
    
    
    
    switch(state)
    {
        case 0:
            Wander();
            break;
        case 1:
            Fear();
            break;
        case 2:
            RotBody(true);
            break;
        case 5:
            if(p2target)
                Hunt(TankHandler::GetSingleton().players[1]);
            else
                Hunt(TankHandler::GetSingleton().players[0]);
            
            
        default:
            RotTurret(false);
    }
    
}

//AIstate:
//0 - wait
//1 - turnbody

void Tank::Wander()
{
    float angle=20;
    float frames=15;
    float xpp = x + (GlobalTimer::dT*frames*movRate) * (float)cos((ry-angle)*DTR);
    float zpp = z + (GlobalTimer::dT*frames*movRate) * (float)sin((ry-angle)*DTR);
    
    
    float xpp2 = x + (GlobalTimer::dT*frames*movRate) * (float)cos((ry+angle)*DTR);
    float zpp2 = z + (GlobalTimer::dT*frames*movRate) * (float)sin((ry+angle)*DTR);
    
    Move(true);
    
    if(LevelHandler::GetSingleton().PointCollision(xpp,y,zpp) || LevelHandler::GetSingleton().PointCollision(xpp2,y,zpp2))
    {
        if(LevelHandler::GetSingleton().t[(int)xpp][(int)zpp]<(y+vy+3) && charge>(maxCharge/4) && LevelHandler::GetSingleton().t[(int)x][(int)z]>(int)(y-7) )
        {
            Jump();
        }
        else
        {
            ry+=90;
            //RotBody(true);
        }
    }
    
}

void Tank::Fear()
{
    recharge=false;
    
    double ratio;
    
    float dist=sqrt((x-TankHandler::GetSingleton().players[0].x)*(x-TankHandler::GetSingleton().players[0].x)+(z-TankHandler::GetSingleton().players[0].z)*(z-TankHandler::GetSingleton().players[0].z));
    
    
    ratio = (double)(z-TankHandler::GetSingleton().players[0].z)/(double)(x-TankHandler::GetSingleton().players[0].x);
    float ryp=toDegrees(atan(ratio));
    
    if(TankHandler::GetSingleton().players[0].x<x)
    {
        ryp+=180;
    }
    
    ryp-=180;
    
    if(ryp<(ry-5))
    {
        RotBody(false);
    }
    else
    {
        if(ryp>(ry+5))
        {
            RotBody(true);
        }
    }
    
    
    ratio = (double)(z-TankHandler::GetSingleton().players[0].z)/(double)(x-TankHandler::GetSingleton().players[0].x);
    
    float rtyp=toDegrees(atan(ratio)) - ry;
    
    if(TankHandler::GetSingleton().players[0].x<x)
    {
        rtyp+=180;
    }
    
    rty=rtyp;
    
    
    float angle=30;
    float frames=20;
    float xpp = x + (GlobalTimer::dT*frames*movRate) * (float)cos((ry-angle)*DTR);
    float zpp = z + (GlobalTimer::dT*frames*movRate) * (float)sin((ry-angle)*DTR);
    
    float xpp2 = x + (GlobalTimer::dT*frames*movRate) * (float)cos((ry+angle)*DTR);
    float zpp2 = z + (GlobalTimer::dT*frames*movRate) * (float)sin((ry+angle)*DTR);
    
    if(LevelHandler::GetSingleton().PointCollision(xpp,y,zpp) || LevelHandler::GetSingleton().PointCollision(xpp2,y,zpp2))
    {
        if(LevelHandler::GetSingleton().t[(int)xpp][(int)zpp]<(y+3) && charge>(maxCharge/4))
        {
            Jump();
        }
        else
        {
            ry+=30;
            //RotBody(float(2));
        }
    }
    
    if(!Move(true))
    {
        ry+=30;
        //RotBody(true);
    }
}

void Tank::Hunt(Tank& player)
{
    
    recharge=true;
    
    double ratio;
    float xpp;
    float zpp;
    
    //float dist=sqrt((x-player.x)*(x-TankHandler::GetSingleton().player.x)+(z-TankHandler::GetSingleton().player.z)*(z-TankHandler::GetSingleton().player.z));
    
    
    ratio = (double)(z-player.z)/(double)(x-player.x);
    float ryp=toDegrees(atan(ratio));
    
    if(player.x<x)
    {
        ryp+=180;
    }
    
    //ry=ryp;
    
    if(ryp<(ry-5))
        RotBody(false);
    else
        if(ryp>(ry+5))
            RotBody(true);
    
    
    ratio = (double)(z-player.z)/(double)(x-player.x);
    
    float rtyp=toDegrees(atan(ratio)) - ry;
    
    if(player.x<x)
    {
        rtyp+=180;
    }
    
    rty=rtyp;
    
    
    xpp = x + (GlobalTimer::dT*10*movRate) * (float)cos(ry*DTR);
    zpp = z + (GlobalTimer::dT*10*movRate) * (float)sin(ry*DTR);
    
    if(LevelHandler::GetSingleton().PointCollision(xpp,y,zpp) )
    {
        if(LevelHandler::GetSingleton().t[(int)xpp][(int)zpp]<(y+5) && charge>(maxCharge/4))
        {
            Jump();
        }
        else
        {
            if(ryp<(ry-5))
                ry-=30;
            else
            {
                if(ryp>(ry+5))
                    ry+=30;
            }
        }
    }
    if(dist>10)
    {
        Move(true);
    }
    else
        if(charge>=(maxCharge/2) && (int)player.y == (int)y)
            Fire(1);
}


Tank::~Tank()
{
    /*	for(vector<Bullet*>::iterator j = bullets.begin(); j != bullets.end(); ++j)
     {
     delete (*j);
     }
     bullets.empty();*/
    bullets.clear();
    while(!bulletq.empty())
    {
        bullets.push_back(bulletq.front());
        bulletq.pop();
    }
}

void Tank::Draw()
{
    
    
    if(alive)
    {
        
        
        glColor3f((4*r2+(maxEnergy/energy))/2,(4*g2+(maxEnergy/energy))/2,(4*b2+(maxEnergy/energy))/2);
        
        
        glPushMatrix();
        
        //glDisable(GL_TEXTURE_2D);
        
        //glEnable(GL_TEXTURE_2D);
        //glBindTexture(GL_TEXTURE_2D, CApplication::GetSingleton().graphicsTask->textureArray[11]);
        
        glTranslatef(x, y, z);
        glRotatef(rx, 1, 0, 0);
        glRotatef(-ry, 0, 1, 0);
        glRotatef(rz, 0, 0, 1);
        
        /* glFrontFace(GL_CCW);
         bodymesh.DrawTriangles();
         glFrontFace(GL_CW);*/
        
        
        
        glScalef(.06f,.06f,.06f);
        glBegin(GL_TRIANGLES);
        glNormal3f(0.894427,-0.447214,0.000000);
        glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
        glTexCoord2f(1.000000,0.000000);glVertex3i(4,0,4);
        glTexCoord2f(1.000000,1.000000);glVertex3i(5,2,3);
        glNormal3f(0.894427,-0.447214,0.000000);
        glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
        glTexCoord2f(1.000000,1.000000);glVertex3i(5,2,3);
        glTexCoord2f(0.000000,1.000000);glVertex3i(5,2,-3);
        glNormal3f(-0.894427,-0.447214,0.000000);
        glTexCoord2f(0.000000,0.000000);glVertex3i(-4,0,-4);
        glTexCoord2f(0.000000,1.000000);glVertex3i(-5,2,-5);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
        glNormal3f(-0.894427,-0.447214,0.000000);
        glTexCoord2f(0.000000,0.000000);glVertex3i(-4,0,-4);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-4,0,4);
        glNormal3f(0.182574,0.365148,-0.912871);
        glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
        glTexCoord2f(0.000000,1.000000);glVertex3i(5,2,-3);
        glTexCoord2f(0.000000,1.000000);glVertex3i(-5,2,-5);
        glNormal3f(0.000000,-0.447214,-0.894427);
        glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
        glTexCoord2f(0.000000,1.000000);glVertex3i(-5,2,-5);
        glTexCoord2f(0.000000,0.000000);glVertex3i(-4,0,-4);
        glNormal3f(0.000000,-0.447214,0.894427);
        glTexCoord2f(1.000000,0.000000);glVertex3i(4,0,4);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-4,0,4);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
        glNormal3f(0.182574,0.365148,0.912871);
        glTexCoord2f(1.000000,0.000000);glVertex3i(4,0,4);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
        glTexCoord2f(1.000000,1.000000);glVertex3i(5,2,3);
        glNormal3f(0.000000,1.000000,0.000000);
        glTexCoord2f(0.000000,1.000000);glVertex3i(5,2,-3);
        glTexCoord2f(1.000000,1.000000);glVertex3i(5,2,3);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
        glNormal3f(0.000000,1.000000,0.000000);
        glTexCoord2f(0.000000,1.000000);glVertex3i(5,2,-3);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
        glTexCoord2f(0.000000,1.000000);glVertex3i(-5,2,-5);
        glNormal3f(0.000000,-1.000000,0.000000);
        glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
        glTexCoord2f(0.000000,0.000000);glVertex3i(-4,0,-4);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-4,0,4);
        glNormal3f(0.000000,-1.000000,0.000000);
        glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-4,0,4);
        glTexCoord2f(1.000000,0.000000);glVertex3i(4,0,4);
        glEnd();
        glScalef(10,10,10);
        
        
        glRotatef(rtx, 1, 0, 0);
        glRotatef(-rty, 0, 1, 0);
        glRotatef(rtz, 0, 0, 1);
        glColor3f((4*r+(maxEnergy/energy))/2,(4*g+(maxEnergy/energy))/2,(4*b+(maxEnergy/energy))/2);
        
        
        //Tank barrel
        glScalef(.1,.1,.1);
        glBegin(GL_TRIANGLES);
        glNormal3f(0.163846,-0.081923,0.983078);
        glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,3);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,3);
        glNormal3f(0.180156,-0.041001,0.982783);
        glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,3);
        glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,2);
        glNormal3f(0.196116,0.000000,-0.980581);
        glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,-2);
        glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,-2);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
        glNormal3f(0.180156,-0.041001,-0.982783);
        glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,-2);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,-3);
        glNormal3f(1.000000,0.000000,0.000000);
        glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
        glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,2);
        glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,-2);
        glNormal3f(1.000000,0.000000,0.000000);
        glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
        glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,-2);
        glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,-2);
        glNormal3f(-0.894427,0.447214,0.000000);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,3);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,-3);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
        glNormal3f(-0.894427,0.447214,0.000000);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,3);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,3);
        glNormal3f(0.000000,1.000000,0.000000);
        glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,2);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,3);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
        glNormal3f(0.000000,1.000000,0.000000);
        glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,2);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
        glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,-2);
        glNormal3f(0.000000,-1.000000,0.000000);
        glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
        glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,-2);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,-3);
        glNormal3f(0.000000,-1.000000,0.000000);
        glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,-3);
        glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,3);
        glEnd();
        glScalef(10,10,10);
        
        //Tank turret
        glTranslatef(.1,0,0);
        
        glScalef(.1,.1,.1);
        glBegin(GL_TRIANGLES);
        glNormal3f(0.000000,1.000000,0.000000);
        glTexCoord2f(0.000000,-0.707107);glVertex3f(-3,4,-0.25f);
        glTexCoord2f(0.000000,0.000000);glVertex3f(5,4,-0.25f);
        glTexCoord2f(1.000000,0.000000);glVertex3f(5,4,0.25f);
        glNormal3f(0.000000,1.000000,0.000000);
        glTexCoord2f(1.000000,1.000000);glVertex3f(-3,4,-0.25f);
        glTexCoord2f(0.000000,0.000000);glVertex3f(5,4,0.25f);
        glTexCoord2f(1.000000,1.000000);glVertex3f(-3,4,0.25f);
        glNormal3f(0.000000,-0.707107,-0.707107);
        glTexCoord2f(0.000000,1.000000);glVertex3f(-3,4,-0.25f);
        glTexCoord2f(0.000000,0.000000);glVertex3i(-3,3,0);
        glTexCoord2f(0.000000,1.000000);glVertex3i(5,3,0);
        glNormal3f(0.000000,-0.707107,-0.707107);
        glTexCoord2f(1.000000,1.000000);glVertex3f(-3,4,-0.25f);
        glTexCoord2f(0.000000,0.000000);glVertex3i(5,3,0);
        glTexCoord2f(1.000000,1.000000);glVertex3f(5,4,-0.25f);
        glNormal3f(-1.000000,0.000000,0.000000);
        glTexCoord2f(1.000000,0.000000);glVertex3f(-3,4,-0.25f);
        glTexCoord2f(0.000000,0.000000);glVertex3f(-3,4,0.25f);
        glTexCoord2f(0.000000,1.000000);glVertex3i(-3,3,0);
        glNormal3f(1.000000,0.000000,0.000000);
        glTexCoord2f(0.000000,1.000000);glVertex3f(5,4,-0.25f);
        glTexCoord2f(1.000000,0.000000);glVertex3i(5,3,0);
        glTexCoord2f(1.000000,0.000000);glVertex3f(5,4,0.25f);
        glNormal3f(0.000000,-0.707107,0.707107);
        glTexCoord2f(1.000000,1.000000);glVertex3f(-3,4,0.25f);
        glTexCoord2f(0.000000,1.000000);glVertex3f(5,4,0.25f);
        glTexCoord2f(1.000000,1.000000);glVertex3i(5,3,0);
        glNormal3f(0.000000,-0.707107,0.707107);
        glTexCoord2f(1.000000,1.000000);glVertex3f(-3,4,0.25f);
        glTexCoord2f(0.000000,1.000000);glVertex3i(5,3,0);
        glTexCoord2f(1.000000,1.000000);glVertex3i(-3,3,0);
        glEnd();
        glScalef(10,10,10);
        
        glPopMatrix();
        
    }
    else
    {
        //Tank::Draw2();
        
    }
    
}


void Tank::Draw2()
{
    
    //glColor3f((4*r2+(maxEnergy/energy))/2,(4*g2+(maxEnergy/energy))/2,(4*b2+(maxEnergy/energy))/2);
    
    
    glPushMatrix();
    
    //glDisable(GL_TEXTURE_2D);
    
    //glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, CApplication::GetSingleton().graphicsTask->textureArray[11]);
    
    glTranslatef(x, y, z);
    glRotatef(rx, 1, 0, 0);
    glRotatef(-ry, 0, 1, 0);
    glRotatef(rz, 0, 0, 1);
    
    /* glFrontFace(GL_CCW);
     bodymesh.DrawTriangles();
     glFrontFace(GL_CW);*/
    
    
    
    glScalef(.06f,.06f,.06f);
    glBegin(GL_TRIANGLES);
    glNormal3f(0.894427,-0.447214,0.000000);
    glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
    glTexCoord2f(1.000000,0.000000);glVertex3i(4,0,4);
    glTexCoord2f(1.000000,1.000000);glVertex3i(5,2,3);
    glNormal3f(0.894427,-0.447214,0.000000);
    glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
    glTexCoord2f(1.000000,1.000000);glVertex3i(5,2,3);
    glTexCoord2f(0.000000,1.000000);glVertex3i(5,2,-3);
    glNormal3f(-0.894427,-0.447214,0.000000);
    glTexCoord2f(0.000000,0.000000);glVertex3i(-4,0,-4);
    glTexCoord2f(0.000000,1.000000);glVertex3i(-5,2,-5);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
    glNormal3f(-0.894427,-0.447214,0.000000);
    glTexCoord2f(0.000000,0.000000);glVertex3i(-4,0,-4);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-4,0,4);
    glNormal3f(0.182574,0.365148,-0.912871);
    glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
    glTexCoord2f(0.000000,1.000000);glVertex3i(5,2,-3);
    glTexCoord2f(0.000000,1.000000);glVertex3i(-5,2,-5);
    glNormal3f(0.000000,-0.447214,-0.894427);
    glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
    glTexCoord2f(0.000000,1.000000);glVertex3i(-5,2,-5);
    glTexCoord2f(0.000000,0.000000);glVertex3i(-4,0,-4);
    glNormal3f(0.000000,-0.447214,0.894427);
    glTexCoord2f(1.000000,0.000000);glVertex3i(4,0,4);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-4,0,4);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
    glNormal3f(0.182574,0.365148,0.912871);
    glTexCoord2f(1.000000,0.000000);glVertex3i(4,0,4);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
    glTexCoord2f(1.000000,1.000000);glVertex3i(5,2,3);
    glNormal3f(0.000000,1.000000,0.000000);
    glTexCoord2f(0.000000,1.000000);glVertex3i(5,2,-3);
    glTexCoord2f(1.000000,1.000000);glVertex3i(5,2,3);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
    glNormal3f(0.000000,1.000000,0.000000);
    glTexCoord2f(0.000000,1.000000);glVertex3i(5,2,-3);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-5,2,5);
    glTexCoord2f(0.000000,1.000000);glVertex3i(-5,2,-5);
    glNormal3f(0.000000,-1.000000,0.000000);
    glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
    glTexCoord2f(0.000000,0.000000);glVertex3i(-4,0,-4);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-4,0,4);
    glNormal3f(0.000000,-1.000000,0.000000);
    glTexCoord2f(0.000000,0.000000);glVertex3i(4,0,-4);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-4,0,4);
    glTexCoord2f(1.000000,0.000000);glVertex3i(4,0,4);
    glEnd();
    glScalef(10,10,10);
    
    
    glRotatef(rtx, 1, 0, 0);
    glRotatef(-rty, 0, 1, 0);
    glRotatef(rtz, 0, 0, 1);
    glColor3f((4*r+(maxEnergy/energy))/2,(4*g+(maxEnergy/energy))/2,(4*b+(maxEnergy/energy))/2);
    
    
    //Tank barrel
    glScalef(.1,.1,.1);
    glBegin(GL_TRIANGLES);
    glNormal3f(0.163846,-0.081923,0.983078);
    glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,3);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,3);
    glNormal3f(0.180156,-0.041001,0.982783);
    glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,3);
    glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,2);
    glNormal3f(0.196116,0.000000,-0.980581);
    glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,-2);
    glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,-2);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
    glNormal3f(0.180156,-0.041001,-0.982783);
    glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,-2);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,-3);
    glNormal3f(1.000000,0.000000,0.000000);
    glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
    glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,2);
    glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,-2);
    glNormal3f(1.000000,0.000000,0.000000);
    glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
    glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,-2);
    glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,-2);
    glNormal3f(-0.894427,0.447214,0.000000);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,3);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,-3);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
    glNormal3f(-0.894427,0.447214,0.000000);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,3);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,3);
    glNormal3f(0.000000,1.000000,0.000000);
    glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,2);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,3);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
    glNormal3f(0.000000,1.000000,0.000000);
    glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,2);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-2,4,-3);
    glTexCoord2f(0.000000,1.000000);glVertex3i(3,4,-2);
    glNormal3f(0.000000,-1.000000,0.000000);
    glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
    glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,-2);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,-3);
    glNormal3f(0.000000,-1.000000,0.000000);
    glTexCoord2f(0.000000,0.000000);glVertex3i(3,2,2);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,-3);
    glTexCoord2f(1.000000,0.000000);glVertex3i(-3,2,3);
    glEnd();
    glScalef(10,10,10);
    
    //Tank turret
    glTranslatef(.1,0,0);
    
    glScalef(.1,.1,.1);
    glBegin(GL_TRIANGLES);
    glNormal3f(0.000000,1.000000,0.000000);
    glTexCoord2f(0.000000,-0.707107);glVertex3f(-3,4,-0.25f);
    glTexCoord2f(0.000000,0.000000);glVertex3f(5,4,-0.25f);
    glTexCoord2f(1.000000,0.000000);glVertex3f(5,4,0.25f);
    glNormal3f(0.000000,1.000000,0.000000);
    glTexCoord2f(1.000000,1.000000);glVertex3f(-3,4,-0.25f);
    glTexCoord2f(0.000000,0.000000);glVertex3f(5,4,0.25f);
    glTexCoord2f(1.000000,1.000000);glVertex3f(-3,4,0.25f);
    glNormal3f(0.000000,-0.707107,-0.707107);
    glTexCoord2f(0.000000,1.000000);glVertex3f(-3,4,-0.25f);
    glTexCoord2f(0.000000,0.000000);glVertex3i(-3,3,0);
    glTexCoord2f(0.000000,1.000000);glVertex3i(5,3,0);
    glNormal3f(0.000000,-0.707107,-0.707107);
    glTexCoord2f(1.000000,1.000000);glVertex3f(-3,4,-0.25f);
    glTexCoord2f(0.000000,0.000000);glVertex3i(5,3,0);
    glTexCoord2f(1.000000,1.000000);glVertex3f(5,4,-0.25f);
    glNormal3f(-1.000000,0.000000,0.000000);
    glTexCoord2f(1.000000,0.000000);glVertex3f(-3,4,-0.25f);
    glTexCoord2f(0.000000,0.000000);glVertex3f(-3,4,0.25f);
    glTexCoord2f(0.000000,1.000000);glVertex3i(-3,3,0);
    glNormal3f(1.000000,0.000000,0.000000);
    glTexCoord2f(0.000000,1.000000);glVertex3f(5,4,-0.25f);
    glTexCoord2f(1.000000,0.000000);glVertex3i(5,3,0);
    glTexCoord2f(1.000000,0.000000);glVertex3f(5,4,0.25f);
    glNormal3f(0.000000,-0.707107,0.707107);
    glTexCoord2f(1.000000,1.000000);glVertex3f(-3,4,0.25f);
    glTexCoord2f(0.000000,1.000000);glVertex3f(5,4,0.25f);
    glTexCoord2f(1.000000,1.000000);glVertex3i(5,3,0);
    glNormal3f(0.000000,-0.707107,0.707107);
    glTexCoord2f(1.000000,1.000000);glVertex3f(-3,4,0.25f);
    glTexCoord2f(0.000000,1.000000);glVertex3i(5,3,0);
    glTexCoord2f(1.000000,1.000000);glVertex3i(-3,3,0);
    glEnd();
    glScalef(10,10,10);
    
    glPopMatrix();
}