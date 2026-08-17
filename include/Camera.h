#pragma once
#include "OutpostMath.h"
class Camera{
public:
 Vec3 pos{0,2.2f,7}; float yaw=-90,pitch=-10; float speed=5;
 Mat4 view()const; Vec3 front()const; void move(float forward,float right,float dt); void look(float dx,float dy);
};

