////////////////////////////////////////////////////////////////////////////////
//File: igtl_qmesh.cpp - implementation File
//Programmer: Izcalli64 - www.gocaco.com
//Last Update: 2/22/2005
//Bullet Description: A Generalized 3D simple mesh class
////////////////////////////////////////////////////////////////////////////////

#ifndef IGTL_3D_GENERIC_QUICK_MESH_CPP
#define IGTL_3D_GENERIC_QUICK_MESH_CPP

#ifdef _WIN32
    // If building in windows:
    #include <windows.h>
    #include <GL/gl.h>
#elif __APPLE__
    // If building on macOS:
    #include <OpenGL/gl.h>
#else
    // If building on Linux:
    #include <GL/gl.h>
    #include <cstdio>
    #include <string.h>
#endif

#include <iostream>
#include <fstream>
#include <sstream>

#include "igtl_qmesh.h"

igtl_QGLMesh::igtl_QGLMesh(){
    
}

igtl_QGLMesh::~igtl_QGLMesh(){
    
    Clear();
}

igtl_QGLMesh::igtl_QGLMesh(const igtl_QGLMesh &old){
    
    //Prevent self assignment
    if(this!=&old){
        
        *this = old;
    }
}

igtl_QGLMesh & igtl_QGLMesh::operator = (const igtl_QGLMesh &old){
    
    //Prevent self assignment
    if(this!=&old){
        
        Clear();
        
        m_verticies = old.m_verticies;
        m_edges = old.m_edges;
        m_triangles = old.m_triangles;
        
    }
    return *this;
}


void igtl_QGLMesh::AddVertex(igtl_QGLVertex &v){
    
    m_verticies.push_back(v);
}

void igtl_QGLMesh::AddEdge(igtl_QGLEdge &e){
    
    m_edges.push_back(e);
}

void igtl_QGLMesh::AddTriangle(igtl_QGLTriangle &t){
    
    m_triangles.push_back(t);
}


igtl_QGLVertex igtl_QGLMesh::GetVertex(unsigned int n){
    
    if( n < m_verticies.size() ){
        
        return m_verticies[n];
    }
    
    return m_dv;
}

igtl_QGLEdge igtl_QGLMesh::GetEdge(unsigned int n){
    
    if( n < m_edges.size() ){
        
        return m_edges[n];
    }
    
    return m_de;
}

igtl_QGLTriangle igtl_QGLMesh::GetTriangle(unsigned int n){
    
    if( n < m_triangles.size() ){
        
        return m_triangles[n];
    }
    
    return m_dt;
}

//Return the number of verticies in this qmesh
unsigned long igtl_QGLMesh::GetNumVerticies(){
    
    return m_verticies.size();
}

//Return the number of triangles in this qmesh
unsigned long igtl_QGLMesh::GetNumEdges(){
    
    return m_edges.size();
}

//Return the number of triangles in this qmesh
unsigned long igtl_QGLMesh::GetNumTriangles(){
    
    return m_triangles.size();
}

//Set the nth vertex to the passed vertex
bool igtl_QGLMesh::SetVertex(unsigned int n, igtl_QGLVertex &v){
    
    if( n < m_verticies.size() ){
        
        m_verticies[n] = v;
        
        return true;
    }
    return false;
}

//Set the nth triangle to the passed triangle
bool igtl_QGLMesh::SetEdge(unsigned int n, igtl_QGLEdge &e){
    
    if( n < m_edges.size() ){
        
        m_edges[n] = e;
        
        return true;
    }
    return false;
}

//Set the nth triangle to the passed triangle
bool igtl_QGLMesh::SetTriangle(unsigned int n, igtl_QGLTriangle &t){
    
    if( n < m_triangles.size() ){
        
        m_triangles[n] = t;
        
        return true;
    }
    return false;
}

bool igtl_QGLMesh::FlipTriangleWinding(unsigned int n){
    
    if( n < m_triangles.size() ){
        
        //Change the winding (Swap 2 & 3)
        //
        unsigned int tv = m_triangles[n].m_v2;
        m_triangles[n].m_v2 = m_triangles[n].m_v3;
        m_triangles[n].m_v3 = tv;
        
        return true;
    }
    return false;
}

bool igtl_QGLMesh::SetEdgeColor(unsigned int n, float r, float g, float b){
    
    if( n < m_edges.size() ){
        
        m_edges[n].m_r = r;
        m_edges[n].m_g = g;
        m_edges[n].m_b = b;
        
        return true;
    }
    return false;
}

//Set the n'th triangles color
bool igtl_QGLMesh::SetTriangleColor(unsigned int n, float r, float g, float b){
    
    if( n < m_triangles.size() ){
        
        m_triangles[n].m_r = r;
        m_triangles[n].m_g = g;
        m_triangles[n].m_b = b;
        
        return true;
    }
    return false;
}

//Set the n'th triangles color
bool igtl_QGLMesh::SetTriangleColor(float r, float g, float b){
    
    for( size_t n = 0; n < m_triangles.size(); n++)
    {
        m_triangles[n].m_r = r;
        m_triangles[n].m_g = g;
        m_triangles[n].m_b = b;
    }
    return true;
}

bool igtl_QGLMesh::SetEdgeFlag(unsigned int n, bool f){//Changes this edge on or off
    
    if( n <m_edges.size() ){
        
        m_edges[n].m_flag = f;
        
        return true;
    }
    return false;
}

//Remove the nth edge
bool igtl_QGLMesh::RemoveEdge(unsigned int n){
    
    if( n < m_edges.size() ){
        
        m_edges[n] = m_edges[m_edges.size() - 1];
        
        m_edges.pop_back();
        return true;
    }
    return false;
}

//Remove the nth triangle
bool igtl_QGLMesh::RemoveTriangle(unsigned int n){
    
    if( n < m_triangles.size() ){
        
        m_triangles[n] = m_triangles[m_triangles.size() - 1];
        
        m_triangles.pop_back();
        return true;
    }
    return false;
}

//Remove all model information
void igtl_QGLMesh::Clear(){
    
    m_verticies.clear();
    m_edges.clear();
    m_triangles.clear();
}

//Save to file
//
bool igtl_QGLMesh::SaveGSM(FILE * chunk){
    
    if( chunk != NULL ){
        
        //Write header
        fputs("IGTL_GSMV1.0\n", chunk);
        
        unsigned int t = 0;
        t = m_verticies.size();
        fwrite(&t, sizeof(unsigned int), 1, chunk);//Number of verticies
        
        float speedup[8];
        
        unsigned int i;
        
        //Write all verticies
        i = 0;
        while( i < t){
            
            //Save vertex
            speedup[0] = m_verticies[i].m_x;
            speedup[1] = m_verticies[i].m_y;
            speedup[2] = m_verticies[i].m_z;
            speedup[3] = m_verticies[i].m_nx;
            speedup[4] = m_verticies[i].m_ny;
            speedup[5] = m_verticies[i].m_nz;
            speedup[6] = m_verticies[i].m_u;
            speedup[7] = m_verticies[i].m_v;
            
            fwrite(speedup, sizeof(float), 8, chunk);
            
            //fwrite(&m_verticies[i].m_x, sizeof(float), 1, chunk);
            //fwrite(&m_verticies[i].m_y, sizeof(float), 1, chunk);
            //fwrite(&m_verticies[i].m_z, sizeof(float), 1, chunk);
            //fwrite(&m_verticies[i].m_nx, sizeof(float), 1, chunk);
            //fwrite(&m_verticies[i].m_ny, sizeof(float), 1, chunk);
            //fwrite(&m_verticies[i].m_nz, sizeof(float), 1, chunk);
            //fwrite(&m_verticies[i].m_u, sizeof(float), 1, chunk);
            //fwrite(&m_verticies[i].m_v, sizeof(float), 1, chunk);
            
            i++;
        }
        
        t = m_triangles.size();
        fwrite(&t, sizeof(unsigned int), 1, chunk);//Number of triangles
        
        //Write all triangles
        i = 0;
        while( i < t){
            
            //Save triangle
            fwrite(&m_triangles[i].m_v1, sizeof(unsigned int), 1, chunk);
            fwrite(&m_triangles[i].m_v2, sizeof(unsigned int), 1, chunk);
            fwrite(&m_triangles[i].m_v3, sizeof(unsigned int), 1, chunk);
            
            speedup[0] = m_triangles[i].m_fx;
            speedup[1] = m_triangles[i].m_fy;
            speedup[2] = m_triangles[i].m_fz;
            speedup[3] = m_triangles[i].m_r;
            speedup[4] = m_triangles[i].m_g;
            speedup[5] = m_triangles[i].m_b;
            
            fwrite(speedup, sizeof(float), 6, chunk);
            
            //fwrite(&m_triangles[i].m_fx, sizeof(float), 1, chunk);
            //fwrite(&m_triangles[i].m_fy, sizeof(float), 1, chunk);
            //fwrite(&m_triangles[i].m_fz, sizeof(float), 1, chunk);
            //fwrite(&m_triangles[i].m_r, sizeof(float), 1, chunk);
            //fwrite(&m_triangles[i].m_g, sizeof(float), 1, chunk);
            //fwrite(&m_triangles[i].m_b, sizeof(float), 1, chunk);
            
            fwrite(&m_triangles[i].m_mat, sizeof(unsigned int), 1, chunk);
            
            i++;
        }
        
        t = m_edges.size();
        fwrite(&t, sizeof(unsigned int), 1, chunk);//Number of edges
        
        //Write all edges
        i = 0;
        while( i < t){
            
            //Save edge
            fwrite(&m_edges[i].m_v1, sizeof(unsigned int), 1, chunk);
            fwrite(&m_edges[i].m_v2, sizeof(unsigned int), 1, chunk);
            
            speedup[0] = m_edges[i].m_nx;
            speedup[1] = m_edges[i].m_ny;
            speedup[2] = m_edges[i].m_nz;
            speedup[3] = m_edges[i].m_r;
            speedup[4] = m_edges[i].m_g;
            speedup[5] = m_edges[i].m_b;
            
            fwrite(speedup, sizeof(float), 6, chunk);
            
            fwrite(&m_edges[i].m_flag, sizeof(unsigned int), 1, chunk);
            
            i++;
        }
        
        return true;
    }
    return false;
}

//Load from file
//
bool igtl_QGLMesh::LoadGSM(FILE * chunk){
    
    if( chunk != NULL ){
        
        char oneline[255];
        
        //Get Tile DB summary
        fgets(oneline, 255, chunk);
        
        //Make sure this is the right file
        if(!strcmp("IGTL_GSMV1.0",oneline)){
            
            return false;
        }
        
        unsigned int t = 0;
        fread(&t, sizeof(unsigned int), 1, chunk);//Number of verticies
        m_verticies.resize(t);
        
        float speedup[8];
        
        //Write all verticies
        unsigned int i = 0;
        while( i < t){
            
            //Read vertex
            fread(speedup, sizeof(float), 8, chunk);
            
            //fread(&m_dv.m_x, sizeof(float), 1, chunk);
            //fread(&m_dv.m_y, sizeof(float), 1, chunk);
            //fread(&m_dv.m_z, sizeof(float), 1, chunk);
            //fread(&m_dv.m_nx, sizeof(float), 1, chunk);
            //fread(&m_dv.m_ny, sizeof(float), 1, chunk);
            //fread(&m_dv.m_nz, sizeof(float), 1, chunk);
            //fread(&m_dv.m_u, sizeof(float), 1, chunk);
            //fread(&m_dv.m_v, sizeof(float), 1, chunk);
            
            m_dv.m_x = speedup[0];
            m_dv.m_y = speedup[1];
            m_dv.m_z = speedup[2];
            m_dv.m_nx = speedup[3];
            m_dv.m_ny = speedup[4];
            m_dv.m_nz = speedup[5];
            m_dv.m_u = speedup[6];
            m_dv.m_v = speedup[7];
            
            if( m_verticies.size() > i ){
                
                m_verticies[i] = m_dv;
            }else{
                
                m_verticies.push_back( m_dv );
            }
            
            i++;
        }
        
        fread(&t, sizeof(unsigned int), 1, chunk);//Number of triangles
        m_triangles.resize(t);
        
        //Read all triangles
        i = 0;
        while( i < t){
            
            //Read triangle
            fread(&m_dt.m_v1, sizeof(unsigned int), 1, chunk);
            fread(&m_dt.m_v2, sizeof(unsigned int), 1, chunk);
            fread(&m_dt.m_v3, sizeof(unsigned int), 1, chunk);
            
            fread(speedup, sizeof(float), 6, chunk);
            
            m_dt.m_fx = speedup[0];
            m_dt.m_fy = speedup[1];
            m_dt.m_fz = speedup[2];
            m_dt.m_r = speedup[3];
            m_dt.m_g = speedup[4];
            m_dt.m_b = speedup[5];
            
            //fread(&m_dt.m_fx, sizeof(float), 1, chunk);
            //fread(&m_dt.m_fy, sizeof(float), 1, chunk);
            //fread(&m_dt.m_fz, sizeof(float), 1, chunk);
            //fread(&m_dt.m_r, sizeof(float), 1, chunk);
            //fread(&m_dt.m_g, sizeof(float), 1, chunk);
            //fread(&m_dt.m_b, sizeof(float), 1, chunk);
            
            fread(&m_dt.m_mat, sizeof(unsigned int), 1, chunk);
            
            if( m_triangles.size() > i ){
                
                m_triangles[i] = m_dt;
            }else{
                
                m_triangles.push_back( m_dt );
            }
            
            i++;
        }
        
        if( feof(chunk) ){
            return true;
        }
        
        fread(&t, sizeof(unsigned int), 1, chunk);//Number of edges
        m_edges.resize(t);
        
        if( feof(chunk) ){
            return true;
        }
        
        //Read all edges
        i = 0;
        while( i < t){
            
            //Save edge
            fread(&m_de.m_v1, sizeof(unsigned int), 1, chunk);
            fread(&m_de.m_v2, sizeof(unsigned int), 1, chunk);
            
            fread(speedup, sizeof(float), 6, chunk);
            
            m_de.m_nx = speedup[0];
            m_de.m_ny = speedup[1];
            m_de.m_nz = speedup[2];
            m_de.m_r = speedup[3];
            m_de.m_g = speedup[4];
            m_de.m_b = speedup[5];
            
            fread(&m_de.m_flag, sizeof(unsigned int), 1, chunk);
            
            if( m_edges.size() > i ){
                
                m_edges[i] = m_de;
            }else{
                
                m_edges.push_back( m_de );
            }
            
            i++;
        }
        
        return true;
    }
    return false;
}

//Regenerates facets, with winding specifiec as v1,v2,v3
//
void igtl_QGLMesh::GenerateFacets(){
    
    float x1,y1,z1,x2,y2,z2;
    
    unsigned int i = 0;
    while( i < m_triangles.size()){
        
        //(V1 -> V2) x (V1 -> V3)
        x1 = m_verticies[m_triangles[i].m_v2].m_x - m_verticies[m_triangles[i].m_v1].m_x;
        y1 = m_verticies[m_triangles[i].m_v2].m_y - m_verticies[m_triangles[i].m_v1].m_y;
        z1 = m_verticies[m_triangles[i].m_v2].m_z - m_verticies[m_triangles[i].m_v1].m_z;
        
        x2 = m_verticies[m_triangles[i].m_v3].m_x - m_verticies[m_triangles[i].m_v1].m_x;
        y2 = m_verticies[m_triangles[i].m_v3].m_y - m_verticies[m_triangles[i].m_v1].m_y;
        z2 = m_verticies[m_triangles[i].m_v3].m_z - m_verticies[m_triangles[i].m_v1].m_z;
        
        m_triangles[i].m_fx = (y1 * z2) - (z1 * y2);
        m_triangles[i].m_fy = (z1 * x2) - (x1 * z2);
        m_triangles[i].m_fz = (x1 * y2) - (y1 * x2);
        
        //Normalize facet
        x1 = m_triangles[i].m_fx*m_triangles[i].m_fx;
        y1 = m_triangles[i].m_fy*m_triangles[i].m_fy;
        z1 = m_triangles[i].m_fz*m_triangles[i].m_fz;
        
        x2 = sqrt(x1 + y1 + z1);
        
        if( x2 != 0 ){
            
            m_triangles[i].m_fx /= x2;
            m_triangles[i].m_fy /= x2;
            m_triangles[i].m_fz /= x2;
        }
        
        i++;
    }
}

//Scales this model down (Permanent change)
//
void igtl_QGLMesh::ScaleVerticies(float x, float y, float z){
    
    unsigned int i = 0;
    while( i < m_verticies.size()){
        
        //Scale the verticies
        m_verticies[i].m_x *= x;
        m_verticies[i].m_y *= y;
        m_verticies[i].m_z *= z;
        i++;
    }
}

//Scales this model down to fit a n x n x n cube (Preserves shape)
//
void igtl_QGLMesh::Unitize(float n){
    
    //First calculate dimesions
    float minx,maxx;
    float miny,maxy;
    float minz,maxz;
    
    unsigned int i = 0;
    if( i < m_verticies.size()){
        
        minx = m_verticies[i].m_x;
        maxx = m_verticies[i].m_x;
        miny = m_verticies[i].m_y;
        maxy = m_verticies[i].m_y;
        minz = m_verticies[i].m_z;
        maxz = m_verticies[i].m_z;
        i++;
    }
    
    while( i < m_verticies.size()){
        
        //Scale the verticies
        if( m_verticies[i].m_x < minx ){
            
            minx = m_verticies[i].m_x;
        }else if( m_verticies[i].m_x > maxx ){
            
            maxx = m_verticies[i].m_x;
        }
        
        if( m_verticies[i].m_y < miny ){
            
            miny = m_verticies[i].m_y;
        }else if( m_verticies[i].m_y > maxy ){
            
            maxy = m_verticies[i].m_y;
        }
        
        if( m_verticies[i].m_z < minz ){
            
            minz = m_verticies[i].m_z;
        }else if( m_verticies[i].m_z > maxz ){
            
            maxz = m_verticies[i].m_z;
        }
        i++;
    }
    
    //Now pick the Largest dimension anc scale everything down by that factor (thus preserving the shape)
    minx = maxx - minx;
    miny = maxy - miny;
    minz = maxz - minz;
    if( miny > minx ){
        minx = miny;
    }
    if( minz > minx ){
        minx = minz;
    }
    
    //Now scale
    if( minx != 0 ){
        
        ScaleVerticies((n/minx),(n/minx),(n/minx));
    }
}

void igtl_QGLMesh::GenerateEdges(){
    
    m_de.m_flag = true;
    
    if(m_edges.size()>0)
    {
        m_edges.clear();
    }
    
    
    
    //Based upon loaded triangles, we create a edge for each unique seto of two verticies
    unsigned int i = 0;
    unsigned int j = 0;
    float mag;
    
    bool rem = false;
    
    while( i < m_triangles.size()){
        
        m_de.m_v1 = m_triangles[i].m_v1;
        m_de.m_v2 = m_triangles[i].m_v2;
        
        m_de.m_nx = m_triangles[i].m_fx;
        m_de.m_ny = m_triangles[i].m_fy;
        m_de.m_nz = m_triangles[i].m_fz;
        
        m_de.m_r = m_triangles[i].m_r;
        m_de.m_g = m_triangles[i].m_g;
        m_de.m_b = m_triangles[i].m_b;
        
        m_de.m_flag = true;
        
        m_edges.push_back(m_de);
        
        m_de.m_v1 = m_triangles[i].m_v2;
        m_de.m_v2 = m_triangles[i].m_v3;
        
        m_edges.push_back(m_de);
        
        m_de.m_v1 = m_triangles[i].m_v3;
        m_de.m_v2 = m_triangles[i].m_v1;
        
        m_edges.push_back(m_de);
        
        i++;
    }
    
    //Now cull like edges
    i = 0;
    while( i < m_edges.size()){
        
        if( m_edges[i].m_flag ){
            
            j = 0;
            while( j < m_edges.size()){
                
                if( i != j && m_edges[j].m_flag ){
                    
                    rem = false;
                    
                    m_de = m_edges[j];
                    
                    //This should work, unless point singularity is violated. (Damn!)
                    if( (m_edges[i].m_v1 == m_edges[j].m_v1) && (m_edges[i].m_v2 == m_edges[j].m_v2) ){
                        
                        rem = true;
                    }
                    if( (m_edges[i].m_v1 == m_edges[j].m_v2) && (m_edges[i].m_v2 == m_edges[j].m_v1) ){
                        
                        rem = true;
                    }
                    
                    if( !rem ){
                        
                        //The deeper test always works, but still why?
                        if( (	m_verticies[m_edges[i].m_v1].m_x == m_verticies[m_edges[j].m_v1].m_x &&
                             m_verticies[m_edges[i].m_v1].m_y == m_verticies[m_edges[j].m_v1].m_y &&
                             m_verticies[m_edges[i].m_v1].m_z == m_verticies[m_edges[j].m_v1].m_z)
                           &&
                           (m_verticies[m_edges[i].m_v2].m_x == m_verticies[m_edges[j].m_v2].m_x &&
                            m_verticies[m_edges[i].m_v2].m_y == m_verticies[m_edges[j].m_v2].m_y &&
                            m_verticies[m_edges[i].m_v2].m_z == m_verticies[m_edges[j].m_v2].m_z) ){
                               
                               rem = true;
                           }
                        
                        if( (	m_verticies[m_edges[i].m_v1].m_x == m_verticies[m_edges[j].m_v2].m_x &&
                             m_verticies[m_edges[i].m_v1].m_y == m_verticies[m_edges[j].m_v2].m_y &&
                             m_verticies[m_edges[i].m_v1].m_z == m_verticies[m_edges[j].m_v2].m_z)
                           &&
                           (m_verticies[m_edges[i].m_v2].m_x == m_verticies[m_edges[j].m_v1].m_x &&
                            m_verticies[m_edges[i].m_v2].m_y == m_verticies[m_edges[j].m_v1].m_y &&
                            m_verticies[m_edges[i].m_v2].m_z == m_verticies[m_edges[j].m_v1].m_z) ){
                               
                               rem = true;
                           }
                    }
                    
                    if( rem == true ){
                        
                        //Both edges are the same. Destroy one edge (after adding normals together)
                        
                        m_edges[i].m_nx += m_edges[j].m_nx;
                        m_edges[i].m_ny += m_edges[j].m_ny;
                        m_edges[i].m_nz += m_edges[j].m_nz;
                        
                        mag = sqrt(m_edges[i].m_nx*m_edges[i].m_nx + m_edges[i].m_ny*m_edges[i].m_ny + m_edges[i].m_nz*m_edges[i].m_nz);
                        
                        if( mag != 0 ){
                            
                            m_edges[i].m_nx /= mag;
                            m_edges[i].m_ny /= mag;
                            m_edges[i].m_nz /= mag;
                        }
                        
                        m_edges[j].m_flag = false;
                    }
                }
                
                j++;
            }
        }
        
        i++;
    }
    
    i = 0;
    while( i < m_edges.size()){
        
        if( !m_edges[i].m_flag ){
            
            RemoveEdge(i);
            i--;
        }
        
        i++;
    }
    
}

//Automatically finds and removes triangles/edges with references to nonexistant points.
void igtl_QGLMesh::SafetyCheck(){
    
    unsigned int i = 0;
    while( i < m_triangles.size()){
        
        if( m_triangles[i].m_v1 >= m_verticies.size() ){
            
            RemoveTriangle(i);
            i--;
        }else{
            
            if( m_triangles[i].m_v2 >= m_verticies.size() ){
                
                RemoveTriangle(i);
                i--;
            }else{
                
                if( m_triangles[i].m_v3 >= m_verticies.size() ){
                    
                    RemoveTriangle(i);
                    i--;
                }
            }
        }
        i++;
    }
    
    i = 0;
    while( i < m_edges.size()){
        
        if( m_edges[i].m_v1 >= m_verticies.size() ){
            
            RemoveEdge(i);
            i--;
        }else{
            
            if( m_edges[i].m_v2 >= m_verticies.size() ){
                
                RemoveEdge(i);
                i--;
            }
        }
        i++;
    }
}

void igtl_QGLMesh::FixWinding(){
    
    //To fix the winding, take the order of verticies
    //V1 -> V2 -> V3
    //And make sure that V3 is on the CCW side of the plane generated by V1 & V2.
    //This is done by first calculating the facet normal, since it is still
    //in the plane I chose.
    //After I have the facet normal, I can cross the normal with the vector generated
    //From V2 - V1, which gives me the plane I want.
    //A Theorum in calculus lets me determine the winding rule for CCW, since the cross product
    //is automatically ? I can say ?
    //
    /*
     igtl_QGLVertex a,b,c;
     float x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,D,Dm;
     unsigned int tv = 0;
     
     unsigned int i = 0;
     while( i < m_triangles.size()){
     
     //Convenience
     a = m_verticies[m_triangles[i].m_v1];
     b = m_verticies[m_triangles[i].m_v2];
     c = m_verticies[m_triangles[i].m_v3];
     
     //Generate testing plane
     //
     x1 = b.m_x - a.m_x;
     y1 = b.m_y - a.m_y;
     z1 = b.m_z - a.m_z;
     //Set Y Vector (Up for plane)
     //
     x2 = -y1;
     y2 = x1;
     z2 = z1;
     //Get temp vector
     //
     x3= c.m_x - a.m_x;
     y3= c.m_y - a.m_y;
     z3= c.m_z - a.m_z;
     
     x4 = (y1 * z3) - (z1 * y3);
     y4 = (z1 * x3) - (x1 * z3);
     z4 = (x1 * y3) - (y1 * x3);
     
     Dm = sqrt(x2*x2 + y2*y2 + z2*z2);
     
     if( Dm != 0 ){
     
     x2 /= Dm;
     y2 /= Dm;
     z2 /= Dm;
     }
     
     Dm = sqrt(x4*x4 + y4*y4 + z4*z4);
     
     if( Dm != 0 ){
     
     x4 /= Dm;
     y4 /= Dm;
     z4 /= Dm;
     }
     
     if( (x2*x4 + y2*y4 + z2*z4) < 0 ){
     
     //Fix the winding (Swap 2 & 3)
     //
     tv = m_triangles[i].m_v2;
     m_triangles[i].m_v2 = m_triangles[i].m_v3;
     m_triangles[i].m_v3 = tv;
     }
     
     i++;
     }
     */
}

void igtl_QGLMesh::MergeVerticies(){
    
    unsigned int i=0;
    unsigned int j=0;
    i = 0;
    while( i < m_verticies.size()){
        
        j = 0;
        while( j < m_verticies.size()){
            
            if( i != j ){
                
                if( m_verticies[i].m_x == m_verticies[j].m_x &&
                   m_verticies[i].m_y == m_verticies[j].m_y &&
                   m_verticies[i].m_z == m_verticies[j].m_z){
                    
                    //If two verticies share the same reference, we need to update all references
                    //greater than the lowest reference and remove 1 vertex.
                    
                    unsigned int lowestr = 0;
                    unsigned int replaceme = 0;
                    unsigned int k = 0;
                    if( i < j ){
                        
                        //Use i as lowest
                        lowestr = i;
                        replaceme = j;
                    }else{
                        
                        //Use j as lowest
                        lowestr = j;
                        replaceme = i;
                    }
                    
                    //Fix normals
                    m_verticies[lowestr].m_nx += m_verticies[replaceme].m_nx;
                    m_verticies[lowestr].m_ny += m_verticies[replaceme].m_ny;
                    m_verticies[lowestr].m_nz += m_verticies[replaceme].m_nz;
                    
                    float mag = sqrt(m_verticies[lowestr].m_nx*m_verticies[lowestr].m_nx + m_verticies[lowestr].m_ny*m_verticies[lowestr].m_ny + m_verticies[lowestr].m_nz*m_verticies[lowestr].m_nz);
                    
                    if( mag != 0 ){
                        
                        m_verticies[lowestr].m_nx /= mag;
                        m_verticies[lowestr].m_ny /= mag;
                        m_verticies[lowestr].m_nz /= mag;
                    }
                    
                    //Go through all triangles and edges, if you find replaceme switch it with lowestr.
                    //	Next if any of the references are greater than replaceme, subtract 1
                    k = 0;
                    while( k < m_triangles.size()){
                        
                        if( m_triangles[k].m_v1 == replaceme ){
                            
                            m_triangles[k].m_v1 = lowestr;
                        }
                        if( m_triangles[k].m_v2 == replaceme ){
                            
                            m_triangles[k].m_v2 = lowestr;
                        }
                        if( m_triangles[k].m_v3 == replaceme ){
                            
                            m_triangles[k].m_v3 = lowestr;
                        }
                        
                        if( m_triangles[k].m_v1 > replaceme ){
                            
                            m_triangles[k].m_v1 --;
                        }
                        if( m_triangles[k].m_v2 > replaceme ){
                            
                            m_triangles[k].m_v2 --;
                        }
                        if( m_triangles[k].m_v3 > replaceme ){
                            
                            m_triangles[k].m_v3 --;
                        }
                        k++;
                    }
                    k = 0;
                    while( k < m_edges.size()){
                        
                        if( m_edges[k].m_v1 == replaceme ){
                            
                            m_edges[k].m_v1 = lowestr;
                        }
                        if( m_edges[k].m_v2 == replaceme ){
                            
                            m_edges[k].m_v2 = lowestr;
                        }
                        
                        if( m_edges[k].m_v1 > replaceme ){
                            
                            m_edges[k].m_v1 --;
                        }
                        if( m_edges[k].m_v2 > replaceme ){
                            
                            m_edges[k].m_v2 --;
                        }
                        
                        k++;
                    }
                    
                    //Remove the vertex from the STL vector. Manually
                    k = replaceme+1;
                    while( k < m_verticies.size()){
                        
                        m_verticies[k-1] = m_verticies[k];
                        
                        k++;
                    }
                    
                    //Done!
                    m_verticies.pop_back();
                    
                    j--;
                    i--;
                    
                    //RemoveVertex(n);	//Complex call (update all edges and triangles, remove ANY with reference.) (N^3)
                }
            }
            
            j++;
        }
        
        i++;
    }
}

void igtl_QGLMesh::DrawTriangles(){
    
    //Draw a triangle here
    glBegin(GL_TRIANGLES);
    
    unsigned int i = 0;
    
    while( i < m_triangles.size() ){
        
        //glColor3f( m_triangles[i].m_r,m_triangles[i].m_g,m_triangles[i].m_b );
        
        //Vertex 0
        glNormal3f( m_verticies[m_triangles[i].m_v1].m_nx,
                   m_verticies[m_triangles[i].m_v1].m_ny,
                   m_verticies[m_triangles[i].m_v1].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v1].m_u,
                     m_verticies[m_triangles[i].m_v1].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v1].m_x,
                   m_verticies[m_triangles[i].m_v1].m_y,
                   m_verticies[m_triangles[i].m_v1].m_z);
        
        //Vertex 1
        glNormal3f( m_verticies[m_triangles[i].m_v2].m_nx,
                   m_verticies[m_triangles[i].m_v2].m_ny,
                   m_verticies[m_triangles[i].m_v2].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v2].m_u,
                     m_verticies[m_triangles[i].m_v2].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v2].m_x,
                   m_verticies[m_triangles[i].m_v2].m_y,
                   m_verticies[m_triangles[i].m_v2].m_z);
        
        //Vertex 2
        glNormal3f( m_verticies[m_triangles[i].m_v3].m_nx,
                   m_verticies[m_triangles[i].m_v3].m_ny,
                   m_verticies[m_triangles[i].m_v3].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v3].m_u,
                     m_verticies[m_triangles[i].m_v3].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v3].m_x,
                   m_verticies[m_triangles[i].m_v3].m_y,
                   m_verticies[m_triangles[i].m_v3].m_z);
        
        //Go to next vertex
        i++;
    }
    
    glEnd();
}

void igtl_QGLMesh::DrawTrianglesExtruded(float f){
    
    glBegin(GL_TRIANGLES);
    
    unsigned int i = 0;
    
    while( i < m_triangles.size() ){
        
        //glColor3f( m_triangles[i].m_r,m_triangles[i].m_g,m_triangles[i].m_b );
        
        //Vertex 0
        glNormal3f( m_verticies[m_triangles[i].m_v1].m_nx,
                   m_verticies[m_triangles[i].m_v1].m_ny,
                   m_verticies[m_triangles[i].m_v1].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v1].m_u,
                     m_verticies[m_triangles[i].m_v1].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v1].m_x + f*m_triangles[i].m_fx,
                   m_verticies[m_triangles[i].m_v1].m_y + f*m_triangles[i].m_fy,
                   m_verticies[m_triangles[i].m_v1].m_z + f*m_triangles[i].m_fz);
        
        //Vertex 1
        glNormal3f( m_verticies[m_triangles[i].m_v2].m_nx,
                   m_verticies[m_triangles[i].m_v2].m_ny,
                   m_verticies[m_triangles[i].m_v2].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v2].m_u,
                     m_verticies[m_triangles[i].m_v2].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v2].m_x + f*m_triangles[i].m_fx,
                   m_verticies[m_triangles[i].m_v2].m_y + f*m_triangles[i].m_fy,
                   m_verticies[m_triangles[i].m_v2].m_z + f*m_triangles[i].m_fz);
        
        //Vertex 2
        glNormal3f( m_verticies[m_triangles[i].m_v3].m_nx,
                   m_verticies[m_triangles[i].m_v3].m_ny,
                   m_verticies[m_triangles[i].m_v3].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v3].m_u,
                     m_verticies[m_triangles[i].m_v3].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v3].m_x + f*m_triangles[i].m_fx,
                   m_verticies[m_triangles[i].m_v3].m_y + f*m_triangles[i].m_fy,
                   m_verticies[m_triangles[i].m_v3].m_z + f*m_triangles[i].m_fz);
        
        //Go to next vertex
        i++;
    }
    
    glEnd();
    
}

void igtl_QGLMesh::DrawTrianglesColored(){
    
    //Draw a triangle here
    glBegin(GL_TRIANGLES);
    
    unsigned int i = 0;
    
    while( i < m_triangles.size() ){
        
        glColor3f( m_triangles[i].m_r,m_triangles[i].m_g,m_triangles[i].m_b );
        
        //Vertex 0
        glNormal3f( m_verticies[m_triangles[i].m_v1].m_nx,
                   m_verticies[m_triangles[i].m_v1].m_ny,
                   m_verticies[m_triangles[i].m_v1].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v1].m_u,
                     m_verticies[m_triangles[i].m_v1].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v1].m_x,
                   m_verticies[m_triangles[i].m_v1].m_y,
                   m_verticies[m_triangles[i].m_v1].m_z);
        
        //Vertex 1
        glNormal3f( m_verticies[m_triangles[i].m_v2].m_nx,
                   m_verticies[m_triangles[i].m_v2].m_ny,
                   m_verticies[m_triangles[i].m_v2].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v2].m_u,
                     m_verticies[m_triangles[i].m_v2].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v2].m_x,
                   m_verticies[m_triangles[i].m_v2].m_y,
                   m_verticies[m_triangles[i].m_v2].m_z);
        
        //Vertex 2
        glNormal3f( m_verticies[m_triangles[i].m_v3].m_nx,
                   m_verticies[m_triangles[i].m_v3].m_ny,
                   m_verticies[m_triangles[i].m_v3].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v3].m_u,
                     m_verticies[m_triangles[i].m_v3].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v3].m_x,
                   m_verticies[m_triangles[i].m_v3].m_y,
                   m_verticies[m_triangles[i].m_v3].m_z);
        
        //Go to next vertex
        i++;
    }
    
    glEnd();
}

void igtl_QGLMesh::DrawTrianglesColoredExtruded(float f){
    
    glBegin(GL_TRIANGLES);
    
    unsigned int i = 0;
    
    while( i < m_triangles.size() ){
        
        glColor3f( m_triangles[i].m_r,m_triangles[i].m_g,m_triangles[i].m_b );
        
        //Vertex 0
        glNormal3f( m_verticies[m_triangles[i].m_v1].m_nx,
                   m_verticies[m_triangles[i].m_v1].m_ny,
                   m_verticies[m_triangles[i].m_v1].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v1].m_u,
                     m_verticies[m_triangles[i].m_v1].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v1].m_x + f*m_triangles[i].m_fx,
                   m_verticies[m_triangles[i].m_v1].m_y + f*m_triangles[i].m_fy,
                   m_verticies[m_triangles[i].m_v1].m_z + f*m_triangles[i].m_fz);
        
        //Vertex 1
        glNormal3f( m_verticies[m_triangles[i].m_v2].m_nx,
                   m_verticies[m_triangles[i].m_v2].m_ny,
                   m_verticies[m_triangles[i].m_v2].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v2].m_u,
                     m_verticies[m_triangles[i].m_v2].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v2].m_x + f*m_triangles[i].m_fx,
                   m_verticies[m_triangles[i].m_v2].m_y + f*m_triangles[i].m_fy,
                   m_verticies[m_triangles[i].m_v2].m_z + f*m_triangles[i].m_fz);
        
        //Vertex 2
        glNormal3f( m_verticies[m_triangles[i].m_v3].m_nx,
                   m_verticies[m_triangles[i].m_v3].m_ny,
                   m_verticies[m_triangles[i].m_v3].m_nz);
        glTexCoord2f( m_verticies[m_triangles[i].m_v3].m_u,
                     m_verticies[m_triangles[i].m_v3].m_v);
        glVertex3f( m_verticies[m_triangles[i].m_v3].m_x + f*m_triangles[i].m_fx,
                   m_verticies[m_triangles[i].m_v3].m_y + f*m_triangles[i].m_fy,
                   m_verticies[m_triangles[i].m_v3].m_z + f*m_triangles[i].m_fz);
        
        //Go to next vertex
        i++;
    }
    
    glEnd();
    
}

void igtl_QGLMesh::DrawEdges(){
    
    //Draw a edge here
    glBegin(GL_LINES);
    
    unsigned int i = 0;
    
    while( i < m_edges.size() ){
        
        //glColor3f( m_edges[i].m_r,m_edges[i].m_g,m_edges[i].m_b );
        
        if( m_edges[i].m_flag ){
            
            //Vertex 0
            glNormal3f( m_verticies[m_edges[i].m_v1].m_nx,
                       m_verticies[m_edges[i].m_v1].m_ny,
                       m_verticies[m_edges[i].m_v1].m_nz);
            glTexCoord2f( m_verticies[m_edges[i].m_v1].m_u,
                         m_verticies[m_edges[i].m_v1].m_v);
            glVertex3f( m_verticies[m_edges[i].m_v1].m_x,
                       m_verticies[m_edges[i].m_v1].m_y,
                       m_verticies[m_edges[i].m_v1].m_z);
            
            //Vertex 1
            glNormal3f( m_verticies[m_edges[i].m_v2].m_nx,
                       m_verticies[m_edges[i].m_v2].m_ny,
                       m_verticies[m_edges[i].m_v2].m_nz);
            glTexCoord2f( m_verticies[m_edges[i].m_v2].m_u,
                         m_verticies[m_edges[i].m_v2].m_v);
            glVertex3f( m_verticies[m_edges[i].m_v2].m_x,
                       m_verticies[m_edges[i].m_v2].m_y,
                       m_verticies[m_edges[i].m_v2].m_z);
        }
        //Go to next vertex
        i++;
    }
    
    glEnd();
}

void igtl_QGLMesh::DrawEdgesExtruded(float f){
    
    //Draw a edge here
    glBegin(GL_LINES);
    
    unsigned int i = 0;
    
    while( i < m_edges.size() ){
        
        //glColor3f( m_edges[i].m_r,m_edges[i].m_g,m_edges[i].m_b );
        
        if( m_edges[i].m_flag ){
            
            //Vertex 0
            glNormal3f( m_verticies[m_edges[i].m_v1].m_nx,
                       m_verticies[m_edges[i].m_v1].m_ny,
                       m_verticies[m_edges[i].m_v1].m_nz);
            glTexCoord2f( m_verticies[m_edges[i].m_v1].m_u,
                         m_verticies[m_edges[i].m_v1].m_v);
            glVertex3f( m_verticies[m_edges[i].m_v1].m_x + f*m_edges[i].m_nx,
                       m_verticies[m_edges[i].m_v1].m_y + f*m_edges[i].m_ny,
                       m_verticies[m_edges[i].m_v1].m_z + f*m_edges[i].m_nz);
            
            //Vertex 1
            glNormal3f( m_verticies[m_edges[i].m_v2].m_nx,
                       m_verticies[m_edges[i].m_v2].m_ny,
                       m_verticies[m_edges[i].m_v2].m_nz);
            glTexCoord2f( m_verticies[m_edges[i].m_v2].m_u,
                         m_verticies[m_edges[i].m_v2].m_v);
            glVertex3f( m_verticies[m_edges[i].m_v2].m_x + f*m_edges[i].m_nx,
                       m_verticies[m_edges[i].m_v2].m_y + f*m_edges[i].m_ny,
                       m_verticies[m_edges[i].m_v2].m_z + f*m_edges[i].m_nz);
        }
        //Go to next vertex
        i++;
    }
    
    glEnd();
}

void igtl_QGLMesh::DrawEdgesColored(){
    
    //Draw a edge here
    glBegin(GL_LINES);
    
    unsigned int i = 0;
    
    while( i < m_edges.size() ){
        
        glColor3f( m_edges[i].m_r,m_edges[i].m_g,m_edges[i].m_b );
        
        if( m_edges[i].m_flag ){
            
            //Vertex 0
            glNormal3f( m_verticies[m_edges[i].m_v1].m_nx,
                       m_verticies[m_edges[i].m_v1].m_ny,
                       m_verticies[m_edges[i].m_v1].m_nz);
            glTexCoord2f( m_verticies[m_edges[i].m_v1].m_u,
                         m_verticies[m_edges[i].m_v1].m_v);
            glVertex3f( m_verticies[m_edges[i].m_v1].m_x,
                       m_verticies[m_edges[i].m_v1].m_y,
                       m_verticies[m_edges[i].m_v1].m_z);
            
            //Vertex 1
            glNormal3f( m_verticies[m_edges[i].m_v2].m_nx,
                       m_verticies[m_edges[i].m_v2].m_ny,
                       m_verticies[m_edges[i].m_v2].m_nz);
            glTexCoord2f( m_verticies[m_edges[i].m_v2].m_u,
                         m_verticies[m_edges[i].m_v2].m_v);
            glVertex3f( m_verticies[m_edges[i].m_v2].m_x,
                       m_verticies[m_edges[i].m_v2].m_y,
                       m_verticies[m_edges[i].m_v2].m_z);
        }
        //Go to next vertex
        i++;
    }
    
    glEnd();
}

void igtl_QGLMesh::DrawEdgesColoredExtruded(float f){
    
    //Draw a edge here
    glBegin(GL_LINES);
    
    unsigned int i = 0;
    
    while( i < m_edges.size() ){
        
        glColor3f( m_edges[i].m_r,m_edges[i].m_g,m_edges[i].m_b );
        
        if( m_edges[i].m_flag ){
            
            //Vertex 0
            glNormal3f( m_verticies[m_edges[i].m_v1].m_nx,
                       m_verticies[m_edges[i].m_v1].m_ny,
                       m_verticies[m_edges[i].m_v1].m_nz);
            glTexCoord2f( m_verticies[m_edges[i].m_v1].m_u,
                         m_verticies[m_edges[i].m_v1].m_v);
            glVertex3f( m_verticies[m_edges[i].m_v1].m_x + f*m_edges[i].m_nx,
                       m_verticies[m_edges[i].m_v1].m_y + f*m_edges[i].m_ny,
                       m_verticies[m_edges[i].m_v1].m_z + f*m_edges[i].m_nz);
            
            //Vertex 1
            glNormal3f( m_verticies[m_edges[i].m_v2].m_nx,
                       m_verticies[m_edges[i].m_v2].m_ny,
                       m_verticies[m_edges[i].m_v2].m_nz);
            glTexCoord2f( m_verticies[m_edges[i].m_v2].m_u,
                         m_verticies[m_edges[i].m_v2].m_v);
            glVertex3f( m_verticies[m_edges[i].m_v2].m_x + f*m_edges[i].m_nx,
                       m_verticies[m_edges[i].m_v2].m_y + f*m_edges[i].m_ny,
                       m_verticies[m_edges[i].m_v2].m_z + f*m_edges[i].m_nz);
        }
        //Go to next vertex
        i++;
    }
    
    glEnd();
}

bool igtl_QGLMesh::SaveOBJ(const std::string& objFile) {
    std::ofstream outFile(objFile);
    if (!outFile) {
        std::cerr << "Error: Could not create " << objFile << "\n";
        return false;
    }

    // Write OBJ header
    outFile << "# Converted from GSM to OBJ\n";

    // Write vertices, normals, and texture coordinates
    for (size_t i = 0; i < GetNumVerticies(); ++i) {
        igtl_QGLVertex v = GetVertex(i);

        // Write vertex position
        outFile << "v " << v.m_x << " " << v.m_y << " " << v.m_z << "\n";

        // Write vertex normal
        outFile << "vn " << v.m_nx << " " << v.m_ny << " " << v.m_nz << "\n";

        // Write texture coordinates
        outFile << "vt " << v.m_u << " " << v.m_v << "\n";
    }

    // Write faces
    for (size_t i = 0; i < GetNumTriangles(); ++i) {
        igtl_QGLTriangle t = GetTriangle(i);

        // Write face with vertex, texture, and normal indices
        outFile << "f "
                << (t.m_v1 + 1) << "/" << (t.m_v1 + 1) << "/" << (t.m_v1 + 1) << " "
                << (t.m_v2 + 1) << "/" << (t.m_v2 + 1) << "/" << (t.m_v2 + 1) << " "
                << (t.m_v3 + 1) << "/" << (t.m_v3 + 1) << "/" << (t.m_v3 + 1) << "\n";
    }

    outFile.close();
    std::cout << "Successfully converted to " << objFile << "\n";
    return true;
}

bool igtl_QGLMesh::LoadOBJ(const std::string& objFile) {
    std::ifstream inFile(objFile);
    if (!inFile) {
        std::cerr << "Error: Could not open " << objFile << "\n";
        return false;
    }

    // Clear existing mesh data
    Clear();

    std::vector<Vec3> tempVertices;  // Temporary storage for vertices
    std::vector<Vec3> tempNormals;   // Temporary storage for normals
    std::vector<Vec2> tempTexCoords; // Temporary storage for texture coordinates

    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            // Vertex position
            Vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            tempVertices.push_back(vertex);
        } else if (prefix == "vn") {
            // Vertex normal
            Vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            tempNormals.push_back(normal);
        } else if (prefix == "vt") {
            // Vertex texture coordinate
            Vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            tempTexCoords.push_back(texCoord);
        } else if (prefix == "f") {
            // Face definition
            std::vector<int> vertexIndices, normalIndices, texCoordIndices;
            std::string vertexData;
            while (iss >> vertexData) {
                std::istringstream vertexStream(vertexData);
                std::string v, vt, vn;
                std::getline(vertexStream, v, '/');
                std::getline(vertexStream, vt, '/');
                std::getline(vertexStream, vn, '/');

                vertexIndices.push_back(std::stoi(v) - 1);
                if (!vt.empty()) texCoordIndices.push_back(std::stoi(vt) - 1);
                if (!vn.empty()) normalIndices.push_back(std::stoi(vn) - 1);
            }

            // Create triangles from face data
            for (size_t i = 1; i < vertexIndices.size() - 1; ++i) {
                igtl_QGLTriangle triangle;
                triangle.m_v1 = vertexIndices[0];
                triangle.m_v2 = vertexIndices[i];
                triangle.m_v3 = vertexIndices[i + 1];
                m_triangles.push_back(triangle);
            }
        }
    }

    // Map temporary data to GSM mesh structure
    for (size_t i = 0; i < tempVertices.size(); ++i) {
        igtl_QGLVertex vertex;
        vertex.m_x = tempVertices[i].x;
        vertex.m_y = tempVertices[i].y;
        vertex.m_z = tempVertices[i].z;

        if (i < tempNormals.size()) {
            vertex.m_nx = tempNormals[i].x;
            vertex.m_ny = tempNormals[i].y;
            vertex.m_nz = tempNormals[i].z;
        } else {
            vertex.m_nx = vertex.m_ny = vertex.m_nz = 0.0f;
        }

        if (i < tempTexCoords.size()) {
            vertex.m_u = tempTexCoords[i].x;
            vertex.m_v = tempTexCoords[i].y;
        } else {
            vertex.m_u = vertex.m_v = 0.0f;
        }

        m_verticies.push_back(vertex);
    }

    inFile.close();
    return true;
}

bool igtl_QGLMesh::SaveAsGSM(const std::string& gsmFile) {
    std::ofstream outFile(gsmFile, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Could not create " << gsmFile << "\n";
        return false;
    }

    // Write GSM header (example header, adjust as needed)
    uint32_t numVertices = static_cast<uint32_t>(GetNumVerticies());
    uint32_t numEdges = static_cast<uint32_t>(GetNumEdges());
    outFile.write(reinterpret_cast<const char*>(&numVertices), sizeof(numVertices));
    outFile.write(reinterpret_cast<const char*>(&numEdges), sizeof(numEdges));

    // Write vertices
    for (size_t i = 0; i < GetNumVerticies(); ++i) {
        igtl_QGLVertex v = GetVertex(i);
        outFile.write(reinterpret_cast<const char*>(&v.m_x), sizeof(v.m_x));
        outFile.write(reinterpret_cast<const char*>(&v.m_y), sizeof(v.m_y));
        outFile.write(reinterpret_cast<const char*>(&v.m_z), sizeof(v.m_z));
        outFile.write(reinterpret_cast<const char*>(&v.m_nx), sizeof(v.m_nx));
        outFile.write(reinterpret_cast<const char*>(&v.m_ny), sizeof(v.m_ny));
        outFile.write(reinterpret_cast<const char*>(&v.m_nz), sizeof(v.m_nz));
        outFile.write(reinterpret_cast<const char*>(&v.m_u), sizeof(v.m_u));
        outFile.write(reinterpret_cast<const char*>(&v.m_v), sizeof(v.m_v));
    }

    // Write edges
    for (size_t i = 0; i < GetNumEdges(); ++i) {
        igtl_QGLEdge e = GetEdge(i);
        outFile.write(reinterpret_cast<const char*>(&e.m_v1), sizeof(e.m_v1));
        outFile.write(reinterpret_cast<const char*>(&e.m_v2), sizeof(e.m_v2));
        outFile.write(reinterpret_cast<const char*>(&e.m_nx), sizeof(e.m_nx));
        outFile.write(reinterpret_cast<const char*>(&e.m_ny), sizeof(e.m_ny));
        outFile.write(reinterpret_cast<const char*>(&e.m_nz), sizeof(e.m_nz));
    }

    outFile.close();
    return true;
}

bool igtl_QGLMesh::SaveGSM_CPP(const std::string& filePath) {
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Could not create " << filePath << "\n";
        return false;
    }

    // Write header
    const std::string header = "IGTL_GSMV1.0\n";
    outFile.write(header.c_str(), header.size());

    // Write number of vertices
    unsigned int vertexCount = static_cast<unsigned int>(m_verticies.size());
    outFile.write(reinterpret_cast<const char*>(&vertexCount), sizeof(unsigned int));

    // Write all vertices
    for (const auto& vertex : m_verticies) {
        outFile.write(reinterpret_cast<const char*>(&vertex.m_x), sizeof(vertex.m_x));
        outFile.write(reinterpret_cast<const char*>(&vertex.m_y), sizeof(vertex.m_y));
        outFile.write(reinterpret_cast<const char*>(&vertex.m_z), sizeof(vertex.m_z));
        outFile.write(reinterpret_cast<const char*>(&vertex.m_nx), sizeof(vertex.m_nx));
        outFile.write(reinterpret_cast<const char*>(&vertex.m_ny), sizeof(vertex.m_ny));
        outFile.write(reinterpret_cast<const char*>(&vertex.m_nz), sizeof(vertex.m_nz));
        outFile.write(reinterpret_cast<const char*>(&vertex.m_u), sizeof(vertex.m_u));
        outFile.write(reinterpret_cast<const char*>(&vertex.m_v), sizeof(vertex.m_v));
    }

    // Write number of edges
    unsigned int edgeCount = static_cast<unsigned int>(m_edges.size());
    outFile.write(reinterpret_cast<const char*>(&edgeCount), sizeof(unsigned int));

    // Write all edges
    for (const auto& edge : m_edges) {
        outFile.write(reinterpret_cast<const char*>(&edge.m_v1), sizeof(edge.m_v1));
        outFile.write(reinterpret_cast<const char*>(&edge.m_v2), sizeof(edge.m_v2));
        outFile.write(reinterpret_cast<const char*>(&edge.m_nx), sizeof(edge.m_nx));
        outFile.write(reinterpret_cast<const char*>(&edge.m_ny), sizeof(edge.m_ny));
        outFile.write(reinterpret_cast<const char*>(&edge.m_nz), sizeof(edge.m_nz));
    }

    // Write number of triangles
    unsigned int triangleCount = static_cast<unsigned int>(m_triangles.size());
    outFile.write(reinterpret_cast<const char*>(&triangleCount), sizeof(unsigned int));

    // Write all triangles
    for (const auto& triangle : m_triangles) {
        outFile.write(reinterpret_cast<const char*>(&triangle.m_v1), sizeof(triangle.m_v1));
        outFile.write(reinterpret_cast<const char*>(&triangle.m_v2), sizeof(triangle.m_v2));
        outFile.write(reinterpret_cast<const char*>(&triangle.m_v3), sizeof(triangle.m_v3));
    }

    outFile.close();
    return true;
}

#endif //IGTL_3D_GENERIC_QUICK_MESH_CPP
