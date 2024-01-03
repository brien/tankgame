#ifndef ModelRenderer_h
#define ModelRenderer_h


#include "DisplayList.h"

class ModelRenderer
{
public:
    static DisplayList displayList;
    static const float LETTER_WIDTH;
    static const float LETTER_HEIGHT;
    static const int COLOR_NUM;

    static void init();

    static void close();

    static float getWidth(int n, float s);

private:
    
};

#endif /* ModelRenderer_h */
