#pragma once
#include <vector>
#include <GL/glew.h>
#include "OutpostMath.h"
class SSAO{public:GLuint fbo=0,texture=0,noise=0;std::vector<Vec3> kernel;int w,h;SSAO(int W,int H);~SSAO();};
