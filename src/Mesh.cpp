#include "Mesh.h"
#include <cstddef>
Mesh::Mesh(const std::vector<Vertex>&v,const std::vector<unsigned>&i){
    glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glGenBuffers(1,&ebo);glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,v.size()*sizeof(Vertex),v.data(),GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);glBufferData(GL_ELEMENT_ARRAY_BUFFER,i.size()*sizeof(unsigned),i.data(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,p));
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,n));
    glEnableVertexAttribArray(2);glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,uv));
    glBindVertexArray(0);count=(GLsizei)i.size();
}
Mesh::~Mesh(){ if(ebo)glDeleteBuffers(1,&ebo);if(vbo)glDeleteBuffers(1,&vbo);if(vao)glDeleteVertexArrays(1,&vao); }
Mesh::Mesh(Mesh&&o) noexcept{vao=o.vao;vbo=o.vbo;ebo=o.ebo;count=o.count;o.vao=o.vbo=o.ebo=0;o.count=0;}
Mesh& Mesh::operator=(Mesh&&o) noexcept{if(this!=&o){if(ebo)glDeleteBuffers(1,&ebo);if(vbo)glDeleteBuffers(1,&vbo);if(vao)glDeleteVertexArrays(1,&vao);vao=o.vao;vbo=o.vbo;ebo=o.ebo;count=o.count;o.vao=o.vbo=o.ebo=0;o.count=0;}return *this;}
void Mesh::draw()const{glBindVertexArray(vao);glDrawElements(GL_TRIANGLES,count,GL_UNSIGNED_INT,nullptr);glBindVertexArray(0);}
Mesh Mesh::quad(){std::vector<Vertex>v={{{-1,-1,0},{0,0,1},{0,0}},{{1,-1,0},{0,0,1},{1,0}},{{1,1,0},{0,0,1},{1,1}},{{-1,1,0},{0,0,1},{0,1}}};return Mesh(v,{0,1,2,0,2,3});}
Mesh Mesh::cube(){
    std::vector<Vertex>v; std::vector<unsigned>i; const Vec3 P[8]={{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}};
    struct F{int a,b,c,d;Vec3 n;}; F fs[]={{4,5,6,7,{0,0,1}},{1,0,3,2,{0,0,-1}},{0,4,7,3,{-1,0,0}},{5,1,2,6,{1,0,0}},{3,7,6,2,{0,1,0}},{0,1,5,4,{0,-1,0}}};
    for(auto&f:fs){unsigned b=(unsigned)v.size();v.push_back({P[f.a],f.n,{0,0}});v.push_back({P[f.b],f.n,{1,0}});v.push_back({P[f.c],f.n,{1,1}});v.push_back({P[f.d],f.n,{0,1}});i.insert(i.end(),{b,b+1,b+2,b,b+2,b+3});}return Mesh(v,i);
}
Mesh Mesh::plane(int r,float s){std::vector<Vertex>v;std::vector<unsigned>i;for(int z=0;z<=r;z++)for(int x=0;x<=r;x++){float u=(float)x/r,w=(float)z/r;float px=(u-.5f)*s,pz=(w-.5f)*s;float y=0.18f*std::sin(px*.65f)*std::cos(pz*.52f)+0.05f*std::sin((px+pz)*1.4f);v.push_back({{px,y,pz},{0,1,0},{u*8,w*8}});}for(int z=0;z<r;z++)for(int x=0;x<r;x++){unsigned a=z*(r+1)+x,b=a+1,c=a+(r+1),d=c+1;i.insert(i.end(),{a,c,b,b,c,d});}return Mesh(v,i);}
