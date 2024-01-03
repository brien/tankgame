#ifndef MATH_H_INCLUDED
#define MATH_H_INCLUDED

const float PI=3.141592f;
const float DTR=0.017453f;

#include <list>
#include <cmath>

class Vec2d
{
public:
    float x,y;
    Vec2d(){x=y=0;}
    Vec2d(float cx,float cy){x=cx;y=cy;}
    Vec2d(const Vec2d &s){x=s.x;y=s.y;}
    
    Vec2d operator +(Vec2d &v){return Vec2d(x+v.x,y+v.y);}
    Vec2d operator -(Vec2d &v){return Vec2d(x-v.x,y-v.y);}
    Vec2d operator +(float f){return Vec2d(x+f, y+f);}
    Vec2d operator -(float f){return Vec2d(x-f, y-f);}
    Vec2d operator *(float f){return Vec2d(x*f, y*f);}
    Vec2d operator /(float f){return Vec2d(x/f, y/f);}
    
    Vec2d operator -(){return Vec2d(-x,-y);}
    
    Vec2d &operator +=(Vec2d &v){x+=v.x;y+=v.y;return (*this);}
    Vec2d &operator -=(Vec2d &v){x-=v.x;y-=v.y;return (*this);}
    Vec2d &operator +=(float f){x+=f;y+=f;return (*this);}
    Vec2d &operator -=(float f){x-=f;y-=f;return (*this);}
    Vec2d &operator *=(float f){x*=f;y*=f;return (*this);}
    Vec2d &operator /=(float f){x/=f;y/=f;return (*this);}
    
    float Dot(Vec2d &v){return x*v.x+y*v.y;}
    float Magsq(){return x*x+y*y;}
    float Magnitude(){return (float)sqrt(Magsq());}
    Vec2d &Normalize(){*this/=Magnitude(); return *this;}
    
    bool operator == (Vec2d &v) {if((x==v.x)&&(y==v.y))return true; return false;}
    bool operator != (Vec2d &v) {return !(*this==v);}
    
};

class Vec3d
{
public:
    float x,y,z;
    Vec3d(){x=y=z=0;}
    Vec3d(float cx,float cy, float cz){x=cx;y=cy;z=cz;}
    Vec3d(const Vec3d &s){x=s.x;y=s.y;z=s.z;}
    
    Vec3d operator +(Vec3d &v){return Vec3d(x+v.x,y+v.y,z+v.z);}
    Vec3d operator -(Vec3d &v){return Vec3d(x-v.x,y-v.y,z-v.z);}
    Vec3d operator +(float f){return Vec3d(x+f, y+f, z+f);}
    Vec3d operator -(float f){return Vec3d(x-f, y-f, z-f);}
    Vec3d operator *(float f){return Vec3d(x*f, y*f, z*f);}
    Vec3d operator /(float f){return Vec3d(x/f, y/f, z/f);}
    
    Vec3d operator -(){return Vec3d(-x,-y,-z);}
    
    Vec3d &operator +=(Vec3d &v){x+=v.x;y+=v.y;z+=v.z;return (*this);}
    Vec3d &operator -=(Vec3d &v){x-=v.x;y-=v.y;z-=v.z;return (*this);}
    Vec3d &operator +=(float f){x+=f;y+=f;z+=f;return (*this);}
    Vec3d &operator -=(float f){x-=f;y-=f;z-=f;return (*this);}
    Vec3d &operator *=(float f){x*=f;y*=f;z*=f;return (*this);}
    Vec3d &operator /=(float f){x/=f;y/=f;z/=f;return (*this);}
    
    float Dot(Vec3d &v){return x*v.x+y*v.y+z*v.z;}
    float Magsq(){return x*x+y*y+z*z;}
    float Magnitude(){return (float)sqrt(Magsq());}
    Vec3d Cross(Vec3d &v){return Vec3d(y*v.z-z*v.y,z*v.x-x*v.z,x*v.y-y*v.x);}
    Vec3d &Normalize(){*this/=Magnitude(); return *this;}
    
    bool operator == (Vec3d &v) {if((x==v.x)&&(y==v.y)&&(z==v.z))return true; return false;}
    bool operator != (Vec3d &v) {return !(*this==v);}
    
};
/*
 class Plane
 {
 public:
	float d;
	Vec3d n;
	Plane(){a=b=c=d=0.0f;}
	void CalcPlane(Vec3d &p, Vec3d &r, Vec3d &q)
	{
 n.Cross(r - p, q - p);
 n.Normalize();
 d = -q.Dot(n);
	}
	bool IsFront(Vec3d &pt)
	{
 return ((pt.x*a)+(pt.y*b)+(pt.z*c)>=d);
	}
	bool IsBack(Vec3d &pt){return !IsFront(pt);}
	void Normalize(){ float m=Vec3d(a,b,c).Magnitude(); a/=m; b/=m; c/=m; d/=m;}
 };*/

float inline toRadians(float val)
{
    return val*(PI/180);
}

float inline toDegrees(float val)
{
    return val/(PI/180);
}

#endif
