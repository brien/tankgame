#include "Item.h"
#include "App.h"
#include <OpenGL/gl.h>

Item::Item()
{
    x=y=z=0;
    rx=ry=rz=0;
    alive=true;
}

Item::Item(float nx, float ny, float nz, TankType ntype)
{
    x=nx;
    y=ny;
    z=nz;
    rx=ry=rz=0;
    type=ntype;
    alive=true;
    
    if(ntype==0)
    {
        r=.5;
        g=.5;
        b=.5;
    }
    else if(ntype==1)
    {
        r=1;
        g=0;
        b=0;
    }
    else if(ntype==2)
    {
        r=0;
        g=0;
        b=1;
    }
    else if(ntype==3)
    {
        r=1;
        g=1;
        b=0;
    }
    else if(ntype==4)
    {
        r=1;
        g=0;
        b=1;
    }
    
}

void Item::Draw()
{
    ry++;
    
    glPushMatrix();
    
    glTranslatef(x, y, z);
    
    glRotatef(-ry, 0, 1, 0);
    glRotatef(rz, 0, 0, 1);
    glRotatef(90, 0, 0, 1);
    
    glColor3f(r, g, b);
    
    glFrontFace(GL_CCW);
    
    glCallList(App::GetSingleton().graphicsTask->itemlist);
    
    glFrontFace(GL_CW);
    
    glPopMatrix();
    
}