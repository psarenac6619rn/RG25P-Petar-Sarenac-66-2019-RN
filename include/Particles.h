#pragma once
#include <vector>
#include <GL/glew.h>
#include "OutpostMath.h"
struct Particle{Vec3 p,v;float life;};
class Particles{std::vector<Particle> ps;GLuint vao=0,vbo=0;public:Particles(int count=180);~Particles();void update(float dt,Vec3 emitter);void draw()const;int count()const{return (int)ps.size();}};
