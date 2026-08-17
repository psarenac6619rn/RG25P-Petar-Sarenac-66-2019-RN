#include "Camera.h"
Vec3 Camera::front()const{float y=radians(yaw),p=radians(pitch);return normalize({std::cos(y)*std::cos(p),std::sin(p),std::sin(y)*std::cos(p)});}
Mat4 Camera::view()const{return lookAt(pos,pos+front(),{0,1,0});}
void Camera::move(float f,float r,float dt){Vec3 fw=front();fw.y=0;fw=normalize(fw);Vec3 rt=normalize(cross(fw,{0,1,0}));pos=pos+fw*(f*speed*dt)+rt*(r*speed*dt);}
void Camera::look(float dx,float dy){yaw+=dx*.09f;pitch=std::clamp(pitch+dy*.09f,-89.0f,89.0f);}

