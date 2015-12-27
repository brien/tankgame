#ifndef ITEM_H_INCLUDED
#define ITEM_H_INCLUDED

class Item
{
public:

	Item();
   Item(float nx, float ny, float nz, int ntype);

   bool alive;
	float x, y, z;
   float rx, ry, rz;

   float r, g, b;

   int type;


	void Draw();



};


#endif