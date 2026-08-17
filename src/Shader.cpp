#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
static std::string readFile(const std::string&p){ std::ifstream f(p); if(!f) throw std::runtime_error("Cannot open "+p); std::stringstream s;s<<f.rdbuf();return s.str(); }
static GLuint compile(GLenum type,const std::string&src){ GLuint s=glCreateShader(type); const char*c=src.c_str();glShaderSource(s,1,&c,nullptr);glCompileShader(s);GLint ok;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);if(!ok){char log[4096];glGetShaderInfoLog(s,4096,nullptr,log);throw std::runtime_error(log);}return s; }
Shader::Shader(const std::string&vs,const std::string&fs){ GLuint v=compile(GL_VERTEX_SHADER,readFile(vs)), f=compile(GL_FRAGMENT_SHADER,readFile(fs)); id=glCreateProgram();glAttachShader(id,v);glAttachShader(id,f);glLinkProgram(id);glDeleteShader(v);glDeleteShader(f);GLint ok;glGetProgramiv(id,GL_LINK_STATUS,&ok);if(!ok){char log[4096];glGetProgramInfoLog(id,4096,nullptr,log);throw std::runtime_error(log);} }
Shader::~Shader(){ if(id)glDeleteProgram(id); }
void Shader::setInt(const char*n,int v)const{ glUniform1i(glGetUniformLocation(id,n),v); }
void Shader::setFloat(const char*n,float v)const{ glUniform1f(glGetUniformLocation(id,n),v); }
void Shader::setVec3(const char*n,Vec3 v)const{ glUniform3f(glGetUniformLocation(id,n),v.x,v.y,v.z); }
void Shader::setMat4(const char*n,const Mat4&m)const{ glUniformMatrix4fv(glGetUniformLocation(id,n),1,GL_FALSE,m.m); }

