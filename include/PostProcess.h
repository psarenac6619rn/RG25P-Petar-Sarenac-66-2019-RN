#pragma once
#include <GL/glew.h>
class PostProcess{public:GLuint fbo=0,color=0,depth=0;int w,h;PostProcess(int W,int H);~PostProcess();void bind()const;};

