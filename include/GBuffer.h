#pragma once
#include <GL/glew.h>
class GBuffer{public:GLuint fbo=0,position=0,normal=0,albedo=0,depth=0;int w=0,h=0;GBuffer(int W,int H);~GBuffer();void bind()const;};
