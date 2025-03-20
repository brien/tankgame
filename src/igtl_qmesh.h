////////////////////////////////////////////////////////////////////////////////
//File: igtl_qmesh.h - Header File
//Programmer: Izcalli64 - www.gocaco.com
//Last Update: 2/22/2005
//Bullet Description: A Generalized 3D simple mesh class
////////////////////////////////////////////////////////////////////////////////

#ifndef IGTL_3D_GENERIC_QUICK_MESH_H
#define IGTL_3D_GENERIC_QUICK_MESH_H

//Define this to enable automatic simple OpenGL drawing (Debug utility)
//#define IGTL_3D_GENERIC_QUICK_MESH_USE_OPENGL

//#ifdef IGTL_3D_GENERIC_QUICK_MESH_USE_OPENGL

//#endif //IGTL_3D_GENERIC_QUICK_MESH_USE_OPENGL


#include "math.h"			//Needed for sqrt, sin, cos

#include <cstdio>
#include <string>
#include <vector>			//Vectors are very nice to use
using std::vector;//but std is a pain.

struct Vec3 {
    float x, y, z;
};

struct Vec2 {
    float x, y;
};

//Vertex (Point in space)
class igtl_QGLVertex{
public:
    float m_x,m_y,m_z; 
    float m_nx,m_ny,m_nz;
    float m_u,m_v;
};

//Edge (2 connected verticies)
class igtl_QGLEdge{
public:
    float m_v1,m_v2;
    float m_nx,m_ny,m_nz;
    float m_r,m_g,m_b;
    unsigned int m_flag;
};

//Triangle (3 connected verticies)
class igtl_QGLTriangle{
public:
    unsigned int m_v1,m_v2,m_v3;
    float m_fx,m_fy,m_fz;
    float m_r,m_g,m_b;
    unsigned int m_mat;
};

class igtl_QGLMesh{
    
public:
    
    igtl_QGLMesh();
    ~igtl_QGLMesh();
    igtl_QGLMesh(const igtl_QGLMesh &old);
    igtl_QGLMesh & operator = (const igtl_QGLMesh &old);
    
    //Get Functions
    //
    igtl_QGLVertex GetVertex(unsigned int n);//Returns a copy of the requested vertex
    igtl_QGLEdge GetEdge(unsigned int n);//Returns a copy of the requested edge
    igtl_QGLTriangle GetTriangle(unsigned int n);//Returns a copy of the requested triangle
    
    unsigned long GetNumVerticies();//Returns the amount of verticies
    unsigned long GetNumEdges();//Returns the amount of verticies
    unsigned long GetNumTriangles();//Returns the amount of triangles
    
    //Set Functions
    //
    bool SetVertex(unsigned int n, igtl_QGLVertex &v);//Sets the nth vertex to the passed vertex
    bool SetEdge(unsigned int n, igtl_QGLEdge &e);//Sets the nth edge to the passed edge
    bool SetTriangle(unsigned int n, igtl_QGLTriangle &t);//Sets the nth triangle to the passed triangle
    
    bool FlipTriangleWinding(unsigned int n);//Toggles CCW and CW winding
    bool SetEdgeColor(unsigned int n, float r, float g, float b);//Changes the color of the nth edge
    bool SetTriangleColor(unsigned int n, float r, float g, float b);//Changes the color of the nth triangle
    bool SetTriangleColor(float r, float g, float b);//Changes the color of the all triangle
    
    //bool SetTriangleMaterial(unsigned int n, float v);//Your own decoding setup.
    bool SetEdgeFlag(unsigned int n, bool f);//Changes this edge on or off
    
    //Add Functions
    //
    void AddVertex(igtl_QGLVertex &v);//Add the vertex to this model
    void AddEdge(igtl_QGLEdge &e);//Add the edge to this model
    void AddTriangle(igtl_QGLTriangle &t);//Add the triangle to this model
    
    //Remove Functions
    //
    //bool RemoveVertex();//This is not safe to do since it destroys all triangle references. MASSIVE repair needed.
    bool RemoveEdge(unsigned int n);//Removes the n'th edge. This is safe to do since order of edges is not important
    bool RemoveTriangle(unsigned int n);//Removes the n'th triangle. This is safe to do since order of triangles is not important
    void SafetyCheck();	//Since this class is not safe, you can optionally check for triangles that reference nonexistant points. They are remove automatically.
    
    //Utility Functions (Model editing)
    //
    void GenerateEdges();//Automatically generates all possible edges (Slow, N^2)
    void GenerateFacets();//Regenerates facets, with winding specifiec as v1,v2,v3
    void ScaleVerticies(float x, float y, float z);	//Scales this model down relative to it's center (Permanent change)
    void Unitize(float n);//Scales this model down to fit a n x n x n cube (Preserves shape)
    
    void MergeVerticies();//N^3 call that eliminates overlapping points within distance d (zero for int stuff)
    //void SubdivideTriangles(float df)//Breaks all triangles into 3 triangles with an additional point extruded by a factor of df from the center.
    void FixWinding();//Rewinds the verticies so that they are all counter-clockwise
    //void SimulateAmbientLight(float r,float g,float b,float ldx,float ldy,float ldz);//Multiplies the colors on the faces by the light direction magnitude.
    
    //File functions
    //
    bool SaveGSM(FILE * chunk);//Save to a gsm file
    bool LoadGSM(FILE * chunk);//Load from a gsm file

    bool SaveOBJ(const std::string& objFile);
    bool LoadOBJ(const std::string& objFile);

    bool SaveAsGSM(const std::string& gsmFile);
    bool SaveGSM_CPP(const std::string& filePath);
    
    //Define this to enable automatic simple OpenGLdrawing
    //#ifdef IGTL_3D_GENERIC_QUICK_MESH_USE_OPENGL
    
    void DrawTriangles();//Just draws the triangles. That's it. No coloring.
    void DrawTrianglesExtruded(float f);//DrawTriangles(), but extruded by factor f. No coloring.
    void DrawTrianglesColored();//Just draws the triangles. That's it. No coloring.
    void DrawTrianglesColoredExtruded(float f);//DrawTriangles(), but extruded by factor f. No coloring.
    void DrawEdges();//Draws all edges (With colors)
    void DrawEdgesExtruded(float f);//Draws all edges (With colors)
    void DrawEdgesColored();//Draws all edges (With colors)
    void DrawEdgesColoredExtruded(float f);//Draws all edges (With colors)
    
    //#endif //IGTL_3D_GENERIC_QUICK_MESH_USE_OPENGL
    
    void Clear();//Remove all Data

    //This is a convenience feature that way you don't need to contruct these as temporary variables.
    //In fact, they play no part in this class except as temps.
    //IE use Get Vertex with class.m_dv = GetVertex(4);
    igtl_QGLVertex m_dv;
    igtl_QGLEdge m_de;
    igtl_QGLTriangle m_dt;
    
private:
    
    vector<igtl_QGLVertex> m_verticies;
    vector<igtl_QGLEdge> m_edges;
    vector<igtl_QGLTriangle> m_triangles;
};

//Yes, I have to place the implementation here to make C++ happy.
//Yes, I know this is ugly and not code friendly, but 
//NO, You can never get this to work otherwise. So deal with it.
//#ifdef IGTL_3D_GENERIC_QUICK_MESH_USE_OPENGL


//#endif //IGTL_3D_GENERIC_QUICK_MESH_USE_OPENGL

#endif //IGTL_3D_GENERIC_QUICK_MESH_H
