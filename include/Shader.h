#pragma once
#include <string>
#include <GL/glew.h>
#include "OutpostMath.h"
class Shader {
public:
    GLuint id=0;
    Shader()=default;
    Shader(const std::string& vs,const std::string& fs);
    ~Shader();
    Shader(const Shader&)=delete; Shader& operator=(const Shader&)=delete;
    Shader(Shader&& o) noexcept { id=o.id;o.id=0; }
    Shader& operator=(Shader&& o) noexcept { if(this!=&o){ if(id)glDeleteProgram(id); id=o.id;o.id=0;} return *this; }
    void use() const { glUseProgram(id); }
    void setInt(const char*n,int v)const;
    void setFloat(const char*n,float v)const;
    void setVec3(const char*n,Vec3 v)const;
    void setMat4(const char*n,const Mat4& m)const;
};

