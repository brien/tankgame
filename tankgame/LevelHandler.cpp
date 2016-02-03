
#include <OpenGL/gl.h>
#include "LevelHandler.h"
#include "TankHandler.h"
#include "FXHandler.h"
#include "App.h"
#include <iostream>
#include <mach-o/dyld.h>

//cheap strlen function, requires a NULL TERMINATED STRING be passed.
//Returns the length of the passed string
int LevelHandler::Strlen(const char * stringy)
{
    int cp=0;			//Incrementor
    bool cpdone=false;	//boolean looper
    //Error checking
    if(stringy!=NULL)
    {
        //Read entire string
        while(!cpdone)
        {
            cp++;
            //Check for null terminator
            if(stringy[cp]==NULL || stringy[cp]=='\n')
            {
                cpdone=true;
            }
        }
    }
    return cp;
}

LevelHandler::LevelHandler()
{
    //t = new int*[64];
    levelNumber=48;
    colorNumber2=colorNumber=0;
    drawFloor=true;
    drawWalls=false;
    drawTop=false;
}

void LevelHandler::Init()
{
    levelNumber=48;
    sizeX=128;
    sizeZ=128;
    colorNumber=0;
    drawFloor=true;
    drawWalls=false;
    drawTop=false;
}

LevelHandler::~LevelHandler()
{
    //delete t[0];
    //delete t;
}


bool LevelHandler::Load(const char filePath[])
{
    
    for(int q=0; q<128; q++)
    {
        for(int w=0; w<128; w++)
        {
            f[q][w]=0;
        }
    }
    
    Flatten(1);
    
    for(int e=0; e<16; e++)
    {
        enemy[e][0]=64;
        enemy[e][1]=64;
    }
    
    /*if(fileName!=NULL)
     {
     delete [] fileName;
     }*/
    
    //fileName=NULL;
    
    if(filePath[0]!=NULL)
    {
        FILE *filein;
        int errnum;
        filein=fopen(filePath, "rt");
        
        //Error checking
        if(!filein)
        {
            //Error Logging goes here.
            std::cerr << "LevelHandler: failed to open file:" << filePath << std::endl;
            errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            perror("Error printed by perror");
            fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
            
            char path[1024];
            uint32_t size = sizeof(path);
            if (_NSGetExecutablePath(path, &size) == 0)
                printf("executable path is %s\n", path);
            else
                printf("buffer too small; need size %u\n", size);
            return false;
        }
        else
        {
            //fileName=new char[Strlen(filePath)+1];
            //fileName[Strlen(filePath)]='\n';
            
            for(int k=Strlen(filePath); k>-1; k--)
            {
                fileName[k]=filePath[k];
            }
            
            //These two lines get the map size
            //The last two characters in the file's name are the map's size.
            //Ex:
            //level@0.txt
            //That means that sizeX=64 and sizeZ=48
            //This is not yet implimented because we're using static arrays
            
            //sizeX=2*filePath[Strlen(filePath)-6];
            
            //sizeZ=2*filePath[Strlen(filePath)-5];
            
            sizeX=128;
            sizeZ=128;
            
            //			TankHandler::GetSingleton().numberTanks=sizeX/16;
            
            
            
            /*
             
             t = new int*[sizeX];
             
             
             //Recall that ap[i] is a pointer to an array of int's
             t[0]=new int[sizeX*sizeZ];
             // This "connects" the array of pointers to the arrays of ints
             for (int m=1;m<sizeX;m++)
             {
             t[m]=t[m-1]+sizeZ;
             }*/
            
            
            
            int cdepth=sizeZ;
            char* oneline = new char[cdepth];	//Storage per line
            //Get first line
            //fgets(oneline, cdepth, filein);
            
            for(int i=0; i<sizeX; i++)
            {
                //Get next line
                fgets(oneline, cdepth, filein);
                for(int j=0; j<sizeZ; j++)
                {
                    t[i][j]=oneline[j];
                    
                    if(t[i][j]==33)
                    {
                        start[0]=i;
                        start[1]=j;
                        t[i][j]=t[i-1][j];
                    }
                    else
                        if(t[i][j]>=48 && t[i][j]<58)
                        {
                            enemy[t[i][j]-48][0]=i;
                            enemy[t[i][j]-48][1]=j;
                            t[i][j]=t[i-1][j];
                        }
                        else
                        {
                            t[i][j]=oneline[j]-100;
                            
                            if(t[i][j]==-1)
                            {
                                if(App::GetSingleton().gameTask->versus)
                                {
                                    t[i][j]=10;
                                }
                                else
                                {
                                    t[i][j]=-20;
                                }
                            }
                            
                            
                            if(t[i][j]>26)
                                t[i][j]=0;
                            if(i==0 || i==(sizeX-1) || j==0 || j==(sizeZ-1))
                                t[i][j]=26;
                        }
                }
                fgets(oneline, cdepth, filein);
            }
            
            delete [] oneline;
            
            fclose(filein);
            return true;
        }
    }
    
    
    levelNumber=fileName[5];
    
    
    return false;
}

void LevelHandler::NextLevel(bool forb)
{
    
    items.clear();
    
    levelNumber=fileName[12];
    
    if(forb)
    {
        levelNumber++;
        colorNumber++;
    }
    
    if(colorNumber>=15)
    {
        colorNumber=0;
    }
    
    if(levelNumber==58)
    {
        if(forb)
        {
            levelNumber=65;
        }
    }
    
    if(levelNumber<76)
    {
        fileName[12]=levelNumber;
    }
    else
    {
        fileName[12]=levelNumber-25;
    }
    
    Load(fileName);
    
    if(levelNumber==51 || levelNumber==52 || levelNumber==57 || levelNumber==55 || levelNumber>70 || levelNumber==66)
    {
        colorNumber2=colorNumber+2;
    }
    else
    {
        colorNumber2=colorNumber;
    }

    TankHandler::GetSingleton().Init();
    
    return;
}


void LevelHandler::Flatten(int height)
{
    int i=0;
    while(i<sizeX)
    {
        int j=0;
        while(j<sizeZ)
        {
            t[i][j]=height;
            j++;
        }
        i++;
    }
}

bool LevelHandler::FloatCollision(float x, float y, float z)
{
    
    bool ret;
    
    if(x>=sizeX || z>=sizeZ)
    {
        return(true);
    }
    
    if( y<f[(int)x][(int)z] && y>(f[(int)x][(int)z]-1) && f[(int)x][(int)z]>0 )
        ret = true;
    else
        ret=false;
    
    return(ret);
}


bool LevelHandler::FallCollision(float x, float y, float z)
{
    
    bool ret;
    
    if(x>=sizeX || z>=sizeZ)
    {
        return(true);
    }
    
    if(!(y>t[(int)x][(int)z]))
        ret = true;
    else
        ret = false;
    
    return(ret);
}

void LevelHandler::AddItem(float x, float y, float z, TankType type)
{
    if(x<sizeX || z<sizeZ)
    {
        Item temp(x, t[(int)x][(int)z]+.2, z, type);
        items.push_back(temp);
    }
    
}

void LevelHandler::ItemCollision()
{
    for(vector<Item>::iterator j = items.begin(); j != items.end(); j++)
    {
        for(int i=0; i<TankHandler::GetSingleton().numPlayers; i++)
        {
            if(TankHandler::GetSingleton().players[i].PointCollision(j->x,j->y,j->z))
            {
                TankHandler::GetSingleton().players[i].SetType(j->type, TankHandler::GetSingleton().players[i].type1);
                
                if(j->type == TankHandler::GetSingleton().players[i].type1)
                    TankHandler::GetSingleton().players[i].energy+=TankHandler::GetSingleton().players[i].maxEnergy;
                else
                    TankHandler::GetSingleton().players[i].energy+=TankHandler::GetSingleton().players[i].maxEnergy/2;
                
                if(TankHandler::GetSingleton().players[i].energy>TankHandler::GetSingleton().players[i].maxEnergy)
                    TankHandler::GetSingleton().players[i].energy=TankHandler::GetSingleton().players[i].maxEnergy;
                
                //NextLevel(true);
                
                j->alive=false;
                
                FXHandler::GetSingleton().CreateFX(3, j->x, j->y, j->z, 90, 0, 90, j->r, j->g, j->b, 1);
                
                App::GetSingleton().soundTask->PlaySound(3);
                return;
            }
        }
        
        /*	for(vector<Tank>::iterator i = TankHandler::GetSingleton().tanks.begin(); i!=TankHandler::GetSingleton().tanks.end(); ++i)
         {
         if((*i).PointCollision(j->x,j->y,j->z))
         {
         //(*i).energy-=power;
         j->alive=false;
         }
         
         }*/
    }
    
}



bool LevelHandler::PointCollision(float x, float y, float z)
{
    bool ret;
    
    if( x<0 || z<0 || x>=sizeX || z>=sizeZ)
    {
        return(true);
    }
    else
    {
        
        if((int)y<t[(int)x][(int)z])
            ret = true;
        else
            ret = false;
        
        
        /*
         if( ((int)y-1)==f[(int)x][(int)z] && f[(int)x][(int)z]>0)
         ret = true;
         */
        
        if( y<f[(int)x][(int)z] && y>=(f[(int)x][(int)z]-1) && f[(int)x][(int)z]>0 )
            ret = true;
    }
    
    return(ret);
}

bool LevelHandler::HandlePointCollision(float &x, float &y, float &z, float &vx, float &vz)
{
    x-=vx;
    z-=vz;
    
    int Xint = (int)x;
    int Zint = (int)z;
    
    x+=vx;
    z+=vz;
    
    int Xor=(int)x;
    int Zor=(int)z;
    
    if((int)x!=Xint && (int)z!=Zint)
    {
        if( (vz<0 && LevelHandler::PointCollision(x, y, z+1) || vz>0 && LevelHandler::PointCollision(x, y, z-1) ) && (vx<0 && LevelHandler::PointCollision(x+1, y, z) || vx>0 && LevelHandler::PointCollision(x-1, y, z) ) )
        {
            x-=vx;
            vx=0;
            z-=vz;
            vz=0;
        }
        else
        {
            
            if(vx>0 && LevelHandler::PointCollision(x-1, y, z))
            {
                z-=vz;
                vz=0;
            }
            //else
            if(vx<0 && LevelHandler::PointCollision(x+1, y, z))
            {
                z-=vz;
                vz=0;
                
            }
            //else
            if(vz>0 && LevelHandler::PointCollision(x, y, z-1))
            {
                x-=vx;
                vx=0;
            }
            //else
            if(vz<0 && LevelHandler::PointCollision(x, y, z+1))
            {
                x-=vx;
                vx=0;
            }
        }
        
    }
    else
    {
        
        if((int)x!=Xint)
        {
            x-=vx;
            if(vx>0)
                x=Xor-.001;
            else
                x=Xint+.001;
            
            vx=0;
        }
        if((int)z!=Zint)
        {
            z-=vz;
            if(vz>0)
                z=Zor-.001;
            else
                z=Zint+.001;
            vz=0;
        }
    }
    return(1);
}

void LevelHandler::GenerateTerrain()
{
    for(int i=0; i<sizeX; i++)
    {
        for(int j=0; j<sizeZ; j++)
        {
            t[i][j]=2;
        }
    }
}

void LevelHandler::DrawItems()
{
    for(vector<Item>::iterator j = items.begin(); j != items.end();)
    {
        if(j->alive)
        {
            j->Draw();
            j++;
        }
        else
        {
            items.erase(j);
        }
    }
}


void LevelHandler::DrawTerrain()
{
    
    
    GLfloat colors[32][4]=
    {
        {1.0, 1.0, 1.0, 1.0},
        {0.0, 1.0, 0.0, 1.0},
        {1.0, 0.0, 0.0, 1.0},
        {0.0, 0.0, 1.0, 1.0},
        {1.0, 0.0, 1.0, 1.0},
        {0.0, 1.0, 0.0, 1.0},
        {1.0, 1.0, 0.0, 1.0},
        {0.0, 0.0, 1.0, 1.0},
        {0.8, 1.0, 0.5, 1.0},
        {1.0, 1.0, 1.0, 1.0},
        {0.5, 0.9, 0.0, 1.0},
        {0.5, 0.5, 0.5, 1.0},
        {0.0, 1.0, 0.5, 1.0},
        {0.3, 0.7, 0.7, 1.0},
        {0.5, 1.0, 0.0, 1.0},
        {0.8, 0.7, 0.0, 1.0},
        
        {1.0, 1.0, 0.0, 1.0},
        {0.0, 1.0, 0.0, 1.0},
        {1.0, 0.0, 0.0, 1.0},
        {0.0, 0.0, 1.0, 1.0},
        {1.0, 0.0, 1.0, 1.0},
        {0.0, 1.0, 1.0, 1.0},
        {1.0, 0.0, 0.5, 1.0},
        {0.0, 0.0, 1.0, 1.0},
        {0.8, 1.0, 0.5, 1.0},
        {1.0, 1.0, 1.0, 1.0},
        {0.5, 0.9, 0.0, 1.0},
        {0.0, 1.0, 0.2, 1.0},
        {0.0, 1.0, 0.5, 1.0},
        {0.3, 0.7, 0.7, 1.0},
        {0.5, 1.0, 0.0, 1.0},
        {0.0, 0.0, 0.0, 1.0},
    };
    
    
    
    glColor3fv(colors[colorNumber]);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    
    //---------------------------
    
    glFrontFace(GL_CCW);
    
    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[11]);
    
    for(int q=0; q<128; q++)
    {
        for(int w=0; w<128; w++)
        {
            if(f[q][w]!=0)
            {
                glPushMatrix();
                
                glColor3fv(colors[colorNumber2]);
                
                glTranslatef(q+.5, f[q][w]-.5, w+.5);
                
                glCallList(App::GetSingleton().graphicsTask->cubelist1);
                glPopMatrix();
            }
        }
    }
    
    glFrontFace(GL_CW);
    
    //-----------------------------
    
    int sx = sizeX-1;
    int sz = sizeZ-1;
    
    int lastY=0;
    int lastBY=0;
    
    
    static bool flash=0;
    
    flash=!flash;
    
    
    if(levelNumber==50 || levelNumber==56 || levelNumber==57 || levelNumber==58 )
    {
        glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[15]);
    }
    else
        if(levelNumber==48 || levelNumber==70 || levelNumber==69)
            glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[10]);
        else
            glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[11]);
    
    
    glNormal3f(0,1,0);
    
    float Y=0;
    int lz=0;
    int n=0;
    
    for(int jx=0; jx<sx; jx++)
    {
        Y=t[jx][0];
        
        for(int jz=0; jz<sz; jz++)
        {
            n++;
            if(Y!=t[jx][jz] || jz==sz-1)
            {
                if(levelNumber==48 || levelNumber==70 || levelNumber==69)
                {
                    if(Y==0)
                        glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[11]);
                    else
                        glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[10]);
                }
                
                if(Y==0)
                    glColor3fv(colors[colorNumber2]);
                else
                    glColor3fv(colors[colorNumber]);
                
                
                
                glBegin(GL_QUADS);
                
                if(Y<25)
                {
                    if(jz==sz-1)
                    {
                        
                        glTexCoord2i(n+1, 1);
                        glVertex3i(jx,Y,jz-n);
                        
                        glTexCoord2i(n+1, 0);
                        glVertex3i(jx+1,Y,jz-n);
                        
                        glTexCoord2i(0, 0);
                        glVertex3i(jx+1,Y,sz);
                        
                        glTexCoord2i(0, 1);
                        glVertex3i(jx,Y,sz);
                    }
                    else
                    {
                        if(Y>=0)
                        {
                            glTexCoord2i(n, 1);
                            glVertex3i(jx,Y,jz-n);
                            
                            glTexCoord2i(n, 0);
                            glVertex3i(jx+1,Y,jz-n);
                            
                            glTexCoord2i(0, 0);
                            glVertex3i(jx+1,Y,jz);
                            
                            glTexCoord2i(0, 1);
                            glVertex3i(jx,Y,jz);
                        }
                        
                    }
                }
                
                glEnd();
                n=0;
            }
            
            Y=t[jx][jz];
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[11]);
    
    for(int ix=0; ix<=sx; ix++)
    {
        
        for(int iz=0; iz<=sz; iz++)
        {
            
            if(t[ix][iz]!=lastY && iz!=sz)
            {
                if(ix!=0)
                {
                    for(int iy=lastY; iy<=t[ix][iz]; iy++)
                    {
                        if(iy==t[ix][iz] )
                        {
                            glBegin(GL_QUADS);
                            
                            glColor3fv(colors[colorNumber2]);
                            
                            if(lastY<0)
                            {
                                glColor3fv(colors[colorNumber+1]);
                            }
                            
                            glTexCoord2f(0, 0);
                            glVertex3f((float)ix,(float)lastY,(float)iz);
                            
                            glTexCoord2f(1, 0);
                            glVertex3f((float)ix+1,(float)lastY,(float)iz);
                            
                            
                            
                            if(lastY<0)
                            {
                                glColor3fv(colors[colorNumber]);
                            }
                            
                            glColor3fv(colors[colorNumber]);
                            
                            glTexCoord2f(1, iy-lastY);
                            glVertex3f((float)ix+1,(float)iy,(float)iz);
                            
                            glTexCoord2f(0, iy-lastY);
                            glVertex3f((float)ix,(float)iy,(float)iz);
                            
                            glEnd();
                        }
                        
                    }
                }
                
            }
            
            lastY=t[ix][iz];
        }
    }
    
    //glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[10]);
    
    for(int ix=sx; ix>0; ix--)
    {
        
        for(int iz=sz-1; iz>0; iz--)
        {
            if(t[ix][iz]!=lastBY )//Nope
            {
                
                if(ix!=sx)
                {
                    for(int iy=lastBY; iy<=t[ix][iz]; iy++)
                    {
                        if(iy==t[ix][iz])
                        {
                            glBegin(GL_QUADS);
                            
                            glTexCoord2i(0, 0);
                            glVertex3i(ix,iy,iz+1);
                            
                            glTexCoord2i(1, 0);
                            glVertex3i(ix+1,iy,iz+1);
                            
                            glColor3fv(colors[colorNumber2]);
                            
                            if(lastBY<0)
                            {
                                glColor3fv(colors[colorNumber+1]);
                            }
                            
                            glTexCoord2i(1, iy-lastBY);
                            glVertex3i(ix+1,lastBY,iz+1);
                            
                            glTexCoord2i(0, iy-lastBY);
                            glVertex3i(ix,lastBY,iz+1);
                            
                            glColor3fv(colors[colorNumber]);
                            
                            if(lastBY<0)
                            {
                                glColor3fv(colors[colorNumber]);
                            }
                            
                            glEnd();
                            
                        }
                    }
                }
            }
            
            
            
            //glBindTexture(GL_TEXTURE_2D, texture[0]);
            
            lastBY=t[ix][iz];
        }
        
    }
    
    //-------------------------------------------
    
    lastY=0;
    lastBY=0;
    
    for(int iz=0; iz<sz; iz++)
    {
        for(int ix=0; ix<=sx; ix++)
        {
            if(t[ix][iz]!=lastY && ix!=sx)
            {
                
                
                if(iz!=0)
                {
                    for(int iy=lastY; iy<=t[ix][iz]; iy++)
                    {
                        if(iy==t[ix][iz])
                        {
                            glBegin(GL_QUADS);
                            
                            glTexCoord2i(0, 0);
                            glVertex3i(ix,iy,iz);
                            
                            glTexCoord2i(1, 0);
                            glVertex3i(ix,iy,iz+1);
                            
                            glColor3fv(colors[colorNumber2]);
                            
                            if(lastY<0)
                            {
                                glColor3fv(colors[colorNumber+1]);
                            }
                            
                            glTexCoord2i(1, iy-lastY);
                            glVertex3i(ix,lastY,iz+1);
                            
                            glTexCoord2i(0, iy-lastY);
                            glVertex3i(ix,lastY,iz);
                            
                            glColor3fv(colors[colorNumber]);
                            
                            if(lastY<0)
                            {
                                glColor3fv(colors[colorNumber]);
                            }
                            glEnd();
                        }
                    }
                }
            }
            lastY=t[ix][iz];
        }
    }
    
    for(int iz=sz; iz>0; iz--)
    {
        for(int ix=sx; ix>=0; ix--)
        {
            
            if(t[ix][iz]!=lastBY && ix!=0)
            {
                
                if(ix!=sz)
                {
                    for(int iy=lastBY; iy<=t[ix][iz]; iy++)
                    {
                        if(iy==t[ix][iz])
                        {
                            
                            //glColor3f(0,1,0);
                            
                            glBegin(GL_QUADS);
                            
                            if(lastBY<0)
                            {
                                glColor3fv(colors[colorNumber+1]);
                            }
                            
                            glColor3fv(colors[colorNumber2]);
                            
                            glTexCoord2i(0, 0);
                            glVertex3f((float)ix+1,(float)lastBY,(float)iz);
                            
                            glTexCoord2i(1, 0);
                            glVertex3f((float)ix+1,(float)lastBY,(float)iz+1);
                            
                            glColor3fv(colors[colorNumber]);
                            
                            if(lastBY<0)
                            {
                                glColor3fv(colors[colorNumber]);
                            }
                            
                            glTexCoord2i(1, iy-lastBY);
                            glVertex3f((float)ix+1,(float)iy,(float)iz+1);
                            
                            glTexCoord2i(0, iy-lastBY);
                            glVertex3f((float)ix+1,(float)iy,(float)iz);
                            glEnd();
                        }
                    }
                }
            }
            
            //glBindTexture(GL_TEXTURE_2D, texture[0]);
            
            lastBY=t[ix][iz];
        }
        
    }
    
    
    if(levelNumber!=48)
    {
        glBegin(GL_QUADS);
        
        glTexCoord2f(sx-1, sz);
        glVertex3f((float)1,(float)26,(float)1);
        
        glTexCoord2f(0, sz);
        glVertex3f((float)1,(float)26,(float)sz);
        
        glTexCoord2f(0, 0);
        glVertex3f((float)sz,(float)26,(float)sz);
        
        glTexCoord2f(sx-1, 0);
        glVertex3f((float)sz,(float)26,(float)1);
        
        
        glEnd();
    }
    
    
    
    
    if(levelNumber==48)//drawWalls)
    {
        glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[10]);
        glTranslatef(0, -30, 0);
        glFrontFace(GL_CCW);
    }
    
    glBegin(GL_QUADS);
    
    glColor3fv(colors[colorNumber2]);
    
    glTexCoord2f(0, 30);
    glVertex3f((float)1,(float)30,(float)1);
    
    glTexCoord2f(sx-1, 30);
    glVertex3f((float)sx,(float)30,(float)1);
    
    glColor3fv(colors[colorNumber2]);
				
    glTexCoord2f(sx-1, 0);
    glVertex3f((float)sx,(float)0,(float)1);
    
    glTexCoord2f(0, 0);
    glVertex3f((float)1,(float)0,(float)1);
    glEnd();
    
    glBegin(GL_QUADS);
    
    glColor3fv(colors[colorNumber]);
    glTexCoord2f(sx-1, 30);
    glVertex3f((float)sx,(float)30,(float)sz);
    
    glTexCoord2f(0, 30);
    glVertex3f((float)1,(float)30,(float)sz);
    
    glColor3fv(colors[colorNumber2]);
    
    glTexCoord2f(0, 0);
    glVertex3f((float)1,(float)0,(float)sz);
    
    glTexCoord2f(sx-1, 0);
    glVertex3f((float)sx,(float)0,(float)sz);
    
    
    glEnd();
    
    
    
    glBegin(GL_QUADS);
    
    glColor3fv(colors[colorNumber]);
    
    
    glTexCoord2f(0, 30);
    glVertex3f((float)sz,(float)30,(float)1);
    
    glTexCoord2f(sx-1, 30);
    glVertex3f((float)sx,(float)30,(float)sz);
    
    glColor3fv(colors[colorNumber2]);
    
    glTexCoord2f(sx-1, 0);
    glVertex3f((float)sx,(float)0,(float)sz);
				
    glTexCoord2f(0, 0);
    glVertex3f((float)sz,(float)0,(float)1);
    
    
    glEnd();
    
    glBegin(GL_QUADS);
    
    glColor3fv(colors[colorNumber]);
    
    glTexCoord2f(0, 30);
    glVertex3f((float)1,(float)30,(float)sz);
    
    glTexCoord2f(sx-1, 30);
    glVertex3f((float)1,(float)30,(float)1);
    
    glColor3fv(colors[colorNumber2]);
				
    glTexCoord2f(sx-1, 0);
    glVertex3f((float)1,(float)0,(float)1);
    
    glTexCoord2f(0, 0);
    glVertex3f((float)1,(float)0,(float)sz);
    glEnd();
    
    if(levelNumber==48)
        glTranslatef(0,30,0);
    
    
    
    
    //-----BLENDY SHIT---//
    
    
    
    sx = sizeX-1;
    sz = sizeZ-1;
    
    lastY=0;
    lastBY=0;
    
    
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    glDepthMask(GL_FALSE);
    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[12]);
    
    glColor4fv(colors[colorNumber+1]);
    
    for(int ix=0; ix<=sx; ix++)
    {
        for(int iz=0; iz<=sz; iz++)
        {
            if(t[ix][iz]!=lastY && iz!=sz)
            {
                if(t[ix][iz]<0)
                {
                    glBegin(GL_QUADS);
                    
                    //glColor4f(0,1,0,0);
                    
                    glTexCoord2i(0, 1);
                    glVertex3f((float)ix,(float)lastY+2,(float)iz);
                    glTexCoord2i(1, 1);
                    glVertex3f((float)ix+1,(float)lastY+2,(float)iz);
                    
                    //glColor4f(0,1,0,1);
                    
                    glTexCoord2i(1, 0);
                    glVertex3f((float)ix+1,(float)lastY,(float)iz);
                    glTexCoord2i(0, 0);
                    glVertex3f((float)ix,(float)lastY,(float)iz);
                    
                    glEnd();
                }
            }
            lastY=t[ix][iz];
        }
    }
    
    for(int ix=sx; ix>0; ix--)
    {
        for(int iz=sz-1; iz>0; iz--)
        {
            if(t[ix][iz]!=lastBY )
            {
                if(t[ix][iz]<0)
                {
                    glBegin(GL_QUADS);
                    
                    //glColor4f(0,10,0,0);
                    
                    glTexCoord2i(0, 1);
                    glVertex3f((float)ix,(float)lastBY+2,(float)iz+1);
                    glTexCoord2i(1, 1);
                    glVertex3f((float)ix+1,(float)lastBY+2,(float)iz+1);
                    
                    //glColor4f(0,10,0,1);
                    
                    glTexCoord2i(1, 0);
                    glVertex3f((float)ix+1,(float)lastBY,(float)iz+1);
                    glTexCoord2i(0, 0);
                    glVertex3f((float)ix,(float)lastBY,(float)iz+1);
                    
                    glEnd();
                    
                }
            }
            
            
            lastBY=t[ix][iz];
        }
        
    }
    
    
    lastY=0;
    lastBY=0;
    
    for(int iz=0; iz<sz; iz++)
    {
        for(int ix=0; ix<=sx; ix++)
        {
            if(t[ix][iz]!=lastY && ix!=sx)
            {
                if(t[ix][iz]<0)
                {
                    glBegin(GL_QUADS);
                    
                    //glColor4f(0,10,0,0);
                    
                    glTexCoord2i(0, 1);
                    glVertex3f((float)ix,(float)lastY+2,(float)iz);
                    glTexCoord2i(1, 1);
                    glVertex3f((float)ix,(float)lastY+2,(float)iz+1);
                    
                    //glColor4f(0,10,0,1);
                    
                    glTexCoord2i(1, 0);
                    glVertex3f((float)ix,(float)lastY,(float)iz+1);
                    glTexCoord2i(0, 0);
                    glVertex3f((float)ix,(float)lastY,(float)iz);
                    glEnd();
                    
                }
            }
            lastY=t[ix][iz];
        }
    }
    
    for(int iz=sz; iz>0; iz--)
    {
        for(int ix=sx; ix>=0; ix--)
        {
            
            if(t[ix][iz]!=lastBY && ix!=0)
            {
                
                if(t[ix][iz]<0)
                {
                    glBegin(GL_QUADS);
                    
                    //glColor4f(0,10,0,0);
                    
                    glTexCoord2i(0, 1);
                    glVertex3f((float)ix+1,(float)lastBY+2,(float)iz);
                    glTexCoord2i(1, 1);
                    glVertex3f((float)ix+1,(float)lastBY+2,(float)iz+1);
                    
                    //glColor4f(0,10,0,1);
                    
                    glTexCoord2i(1, 0);
                    glVertex3f((float)ix+1,(float)lastBY,(float)iz+1);
                    glTexCoord2i(0, 0);
                    glVertex3f((float)ix+1,(float)lastBY,(float)iz);
                    glEnd();
                }
            }
            
            lastBY=t[ix][iz];
        }
    }
    glColor3fv(colors[colorNumber]);
    
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    
    
    glEnable(GL_CULL_FACE);
    
    glPopMatrix();
    
}


//############################################################

void LevelHandler::DrawTerrain_OLD()
{
    if(levelNumber==48)
        glColor3f(1.0, 1.0, 1.0);
    else if(levelNumber==49)
        glColor3f(0.1, 1.0, 0.1);
    else if(levelNumber==50)
        glColor3f(1.0, 0.1, .1);
    
    glEnable(GL_DEPTH_TEST);
    
    glPushMatrix();
    
    int sx = sizeX-1;
    int sz = sizeZ-1;
    
    int lastY=0;
    int lastBY=0;
    
    glEnable(GL_TEXTURE_2D);
    static bool flash=0;
    
    flash=!flash;
    
    
    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[flash+10]);
    
    glNormal3f(0,1,0);
    
    for(int ix=0; ix<=sx; ix++)
    {
        
        for(int iz=0; iz<=sz; iz++)
        {
            
            if(t[ix][iz]!=lastY && iz!=sz)//Not here
            {
                
                
                if(ix!=0)
                {
                    
                    
                    for(int iy=lastY; iy<t[ix][iz]; iy++)
                    {
                        glBegin(GL_QUADS);
                        
                        glTexCoord2f(0, 0);
                        glVertex3f((float)ix,(float)iy,(float)iz);
                        
                        glTexCoord2f(1, 0);
                        glVertex3f((float)ix+1,(float)iy,(float)iz);
                        
                        glTexCoord2f(1, 1);
                        glVertex3f((float)ix+1,(float)iy+1,(float)iz);
                        
                        glTexCoord2f(0, 1);
                        glVertex3f((float)ix,(float)iy+1,(float)iz);
                        
                        glEnd();
                    }
                }
                
            }
            
            
            
            
            
            //glDisable(GL_TEXTURE_2D);
            
            //glBindTexture(GL_TEXTURE_2D, VideoTask::textureArray[1]);
            
            if(t[ix][iz]!=0 && t[ix][iz]>0)
            {
                
                glBegin(GL_QUADS);
                glTexCoord2i(1, 1);
                glVertex3f((float)ix,(float)t[ix][iz],(float)iz);
                
                glTexCoord2i(1, 0);
                glVertex3f((float)ix+1,(float)t[ix][iz],(float)iz);
                
                glTexCoord2i(0, 0);
                glVertex3f((float)ix+1,(float)t[ix][iz],(float)iz+1);
                
                glTexCoord2i(0, 1);
                glVertex3f((float)ix,(float)t[ix][iz],(float)iz+1);
                glEnd();
                
                
            }
            
            //glBindTexture(GL_TEXTURE_2D, VideoTask::textureArray[1]);
            
            if(drawFloor && t[ix][iz]==0)
            {
                
                
                //	if(TankHandler::GetSingleton().player.ry>0 && TankHandler::GetSingleton().player.ry<90)
                //	{
                //	if(TankHandler::GetSingleton().player.x < ix)
                //	{
                
                //Draw flat surface.
                glBegin(GL_QUADS);
                glTexCoord2i(1, 1);
                glVertex3f((float)ix,(float)t[ix][iz],(float)iz);
                
                glTexCoord2i(1, 0);
                glVertex3f((float)(ix+1),(float)t[ix][iz],(float)iz);
                
                glTexCoord2i(0, 0);
                glVertex3f((float)(ix+1),(float)t[ix][iz],(float)(iz+1));
                
                glTexCoord2i(0, 1);
                glVertex3f((float)ix,(float)t[ix][iz],(float)(iz+1));
                
            }
            lastY=t[ix][iz];
        }
    }
    
    for(int ix=sx; ix>0; ix--)
    {
        
        for(int iz=sz-1; iz>0; iz--)
        {
            if(t[ix][iz]!=lastBY )//Nope
            {
                
                if(ix!=sx)
                {
                    for(int iy=lastBY; iy<t[ix][iz]; iy++)
                    {
                        
                        
                        glBegin(GL_QUADS);
                        
                        glTexCoord2i(0, 1);
                        glVertex3f((float)ix,(float)iy+1,(float)iz+1);
                        
                        glTexCoord2i(0, 0);
                        glVertex3f((float)ix+1,(float)iy+1,(float)iz+1);
                        
                        glTexCoord2i(1, 0);
                        glVertex3f((float)ix+1,(float)iy,(float)iz+1);
                        
                        glTexCoord2i(1, 1);
                        glVertex3f((float)ix,(float)iy,(float)iz+1);
                        glEnd();
                    }
                }
            }
            
            
            lastBY=t[ix][iz];
        }
        
    }
    
    //-------------------------------------------
    
    lastY=0;
    lastBY=0;
    
    for(int iz=0; iz<sz; iz++)
    {
        for(int ix=0; ix<=sx; ix++)
        {
            if(t[ix][iz]!=lastY && ix!=sx)
            {
                
                
                if(iz!=0)
                {
                    for(int iy=lastY; iy<t[ix][iz]; iy++)
                    {
                        
                        
                        glBegin(GL_QUADS);
                        
                        glTexCoord2i(0, 1);
                        glVertex3f((float)ix,(float)iy+1,(float)iz);
                        
                        glTexCoord2i(0, 0);
                        glVertex3f((float)ix,(float)iy+1,(float)iz+1);
                        
                        glTexCoord2i(1, 0);
                        glVertex3f((float)ix,(float)iy,(float)iz+1);
                        
                        glTexCoord2i(1, 1);
                        glVertex3f((float)ix,(float)iy,(float)iz);
                        glEnd();
                    }
                }
            }
            lastY=t[ix][iz];
        }
    }
    
    for(int iz=sz; iz>0; iz--)
    {
        for(int ix=sx; ix>=0; ix--)
        {
            
            if(t[ix][iz]!=lastBY && ix!=0)
            {
                
                if(ix!=sz)
                {
                    for(int iy=lastBY; iy<t[ix][iz]; iy++)
                    {
                        
                        //glBindTexture(GL_TEXTURE_2D, texture[0]);d
                        
                        glColor3f(0,1,0);
                        
                        glBegin(GL_QUADS);
                        
                        glTexCoord2i(0, 0);
                        glVertex3f((float)ix+1,(float)iy,(float)iz);
                        
                        glTexCoord2i(1, 0);
                        glVertex3f((float)ix+1,(float)iy,(float)iz+1);
                        
                        
                        glTexCoord2i(1, 1);
                        glVertex3f((float)ix+1,(float)iy+1,(float)iz+1);
                        
                        glTexCoord2i(0, 1);
                        glVertex3f((float)ix+1,(float)iy+1,(float)iz);
                        
                        
                        
                        
                        glEnd();
                    }
                }
            }
            
            //glBindTexture(GL_TEXTURE_2D, texture[0]);
            
            lastBY=t[ix][iz];
        }
        
    }
    
    if(levelNumber==48)
    {
        glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[10]);
    }
    
    if(drawWalls)
    {
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 30);
        glVertex3f((float)1,(float)30,(float)1);
        
        glTexCoord2f(sx-1, 30);
        glVertex3f((float)sx,(float)30,(float)1);
        
        glTexCoord2f(sx-1, 0);
        glVertex3f((float)sx,(float)0,(float)1);
        
        glTexCoord2f(0, 0);
        glVertex3f((float)1,(float)0,(float)1);
        glEnd();
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 30);
        glVertex3f((float)1,(float)30,(float)sz);
        
        glTexCoord2f(sx-1, 30);
        glVertex3f((float)sx,(float)30,(float)sz);
        
        glTexCoord2f(sx-1, 0);
        glVertex3f((float)sx,(float)0,(float)sz);
        
        glTexCoord2f(0, 0);
        glVertex3f((float)1,(float)0,(float)sz);
        glEnd();
        
        //glBindTexture(GL_TEXTURE_2D, CApplication::GetSingleton().graphicsTask->textureArray[15]);
        
        glBegin(GL_QUADS);
        glTexCoord2f(sx-1, 30);
        glVertex3f((float)sx,(float)30,(float)sz);
        
        glTexCoord2f(0, 30);
        glVertex3f((float)sz,(float)30,(float)1);
        
        glTexCoord2f(0, 0);
        glVertex3f((float)sz,(float)0,(float)1);
        
        glTexCoord2f(sx-1, 0);
        glVertex3f((float)sx,(float)0,(float)sz);
        glEnd();
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 30);
        glVertex3f((float)1,(float)30,(float)sz);
        
        glTexCoord2f(sx-1, 30);
        glVertex3f((float)1,(float)30,(float)1);
        
        glTexCoord2f(sx-1, 0);
        glVertex3f((float)1,(float)0,(float)1);
        
        glTexCoord2f(0, 0);
        glVertex3f((float)1,(float)0,(float)sz);
        glEnd();
    }
    if(drawTop)
    {
        glBegin(GL_QUADS);
        glTexCoord2f(0, sz);
        glVertex3f((float)1,(float)26,(float)sz);
        
        glTexCoord2f(sx-1, sz);
        glVertex3f((float)1,(float)26,(float)1);
        
        glTexCoord2f(sx-1, 0);
        glVertex3f((float)sz,(float)26,(float)1);
        
        glTexCoord2f(0, 0);
        glVertex3f((float)sz,(float)26,(float)sz);
        glEnd();
    }
    
    
    //-----BLENDY SHIT---//
    
    
    
    sx = sizeX-1;
    sz = sizeZ-1;
    
    lastY=0;
    lastBY=0;
    
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    glDepthMask(GL_FALSE);
    glBindTexture(GL_TEXTURE_2D, App::GetSingleton().graphicsTask->textureArray[12]);
    
    if(levelNumber==48) glColor4f(0,1,0,1);
    else if(levelNumber==49) glColor4f(1,0,0,1);
    
    for(int ix=0; ix<=sx; ix++)
    {
        for(int iz=0; iz<=sz; iz++)
        {
            if(t[ix][iz]!=lastY && iz!=sz)
            {
                if(t[ix][iz]<0)
                {
                    glBegin(GL_QUADS);
                    
                    //glColor4f(0,1,0,0);
                    
                    glTexCoord2i(0, 1);
                    glVertex3f((float)ix,(float)lastY+1,(float)iz);
                    glTexCoord2i(1, 1);
                    glVertex3f((float)ix+1,(float)lastY+1,(float)iz);
                    
                    //glColor4f(0,1,0,1);
                    
                    glTexCoord2i(1, 0);
                    glVertex3f((float)ix+1,(float)lastY,(float)iz);
                    glTexCoord2i(0, 0);
                    glVertex3f((float)ix,(float)lastY,(float)iz);
                    
                    glEnd();
                }
            }
            lastY=t[ix][iz];
        }
    }
    
    for(int ix=sx; ix>0; ix--)
    {
        for(int iz=sz-1; iz>0; iz--)
        {
            if(t[ix][iz]!=lastBY )
            {
                if(t[ix][iz]<0)
                {
                    glBegin(GL_QUADS);
                    
                    //glColor4f(0,10,0,0);
                    
                    glTexCoord2i(0, 1);
                    glVertex3f((float)ix,(float)lastBY+1,(float)iz+1);
                    glTexCoord2i(1, 1);
                    glVertex3f((float)ix+1,(float)lastBY+1,(float)iz+1);
                    
                    //glColor4f(0,10,0,1);
                    
                    glTexCoord2i(1, 0);
                    glVertex3f((float)ix+1,(float)lastBY,(float)iz+1);
                    glTexCoord2i(0, 0);
                    glVertex3f((float)ix,(float)lastBY,(float)iz+1);
                    
                    glEnd();
                    
                }
            }
            
            
            lastBY=t[ix][iz];
        }
        
    }
    
    
    lastY=0;
    lastBY=0;
    
    for(int iz=0; iz<sz; iz++)
    {
        for(int ix=0; ix<=sx; ix++)
        {
            if(t[ix][iz]!=lastY && ix!=sx)
            {
                if(t[ix][iz]<0)
                {
                    glBegin(GL_QUADS);
                    
                    //glColor4f(0,10,0,0);
                    
                    glTexCoord2i(0, 1);
                    glVertex3f((float)ix,(float)lastY+1,(float)iz);
                    glTexCoord2i(1, 1);
                    glVertex3f((float)ix,(float)lastY+1,(float)iz+1);
                    
                    //glColor4f(0,10,0,1);
                    
                    glTexCoord2i(1, 0);
                    glVertex3f((float)ix,(float)lastY,(float)iz+1);
                    glTexCoord2i(0, 0);
                    glVertex3f((float)ix,(float)lastY,(float)iz);
                    glEnd();
                    
                }
            }
            lastY=t[ix][iz];
        }
    }
    
    for(int iz=sz; iz>0; iz--)
    {
        for(int ix=sx; ix>=0; ix--)
        {
            
            if(t[ix][iz]!=lastBY && ix!=0)
            {
                
                if(t[ix][iz]<0)
                {
                    glBegin(GL_QUADS);
                    
                    //glColor4f(0,10,0,0);
                    
                    glTexCoord2i(0, 1);
                    glVertex3f((float)ix+1,(float)lastBY+1,(float)iz);
                    glTexCoord2i(1, 1);
                    glVertex3f((float)ix+1,(float)lastBY+1,(float)iz+1);
                    
                    //glColor4f(0,10,0,1);
                    
                    glTexCoord2i(1, 0);
                    glVertex3f((float)ix+1,(float)lastBY,(float)iz+1);
                    glTexCoord2i(0, 0);
                    glVertex3f((float)ix+1,(float)lastBY,(float)iz);
                    glEnd();
                }
            }
            
            lastBY=t[ix][iz];
        }
    }
    
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    
    
    glEnable(GL_CULL_FACE);
    
    glPopMatrix();
    
}
