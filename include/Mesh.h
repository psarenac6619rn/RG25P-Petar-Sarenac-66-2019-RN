#pragma once
#include <vector>
#include <GL/glew.h>
#include "OutpostMath.h"
struct Vertex{ Vec3 p,n; Vec2 uv; };
class Mesh{
    GLuint vao=0,vbo=0,ebo=0; GLsizei count=0;
    Mesh(const std::vector<Vertex>&v,const std::vector<unsigned>&i);
public:
    Mesh()=default; ~Mesh(); Mesh(const Mesh&)=delete; Mesh& operator=(const Mesh&)=delete;
    Mesh(Mesh&&o) noexcept; Mesh& operator=(Mesh&&o) noexcept;
    static Mesh cube(); static Mesh plane(int resolution,float size); static Mesh quad();
    void draw()const;
};
