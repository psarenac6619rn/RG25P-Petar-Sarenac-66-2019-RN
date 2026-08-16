#pragma once
#include <cmath>
#include <algorithm>

constexpr float PI_F = 3.14159265358979323846f;
inline float radians(float deg){ return deg * PI_F / 180.0f; }

struct Vec2 { float x=0, y=0; };
struct Vec3 {
    float x=0, y=0, z=0;
    Vec3()=default; Vec3(float X,float Y,float Z):x(X),y(Y),z(Z){}
};
inline Vec3 operator+(Vec3 a, Vec3 b){ return {a.x+b.x,a.y+b.y,a.z+b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b){ return {a.x-b.x,a.y-b.y,a.z-b.z}; }
inline Vec3 operator*(Vec3 a, float s){ return {a.x*s,a.y*s,a.z*s}; }
inline Vec3 operator/(Vec3 a, float s){ return {a.x/s,a.y/s,a.z/s}; }
inline float dot(Vec3 a,Vec3 b){ return a.x*b.x+a.y*b.y+a.z*b.z; }
inline Vec3 cross(Vec3 a,Vec3 b){ return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
inline float length(Vec3 v){ return std::sqrt(dot(v,v)); }
inline Vec3 normalize(Vec3 v){ float l=length(v); return l>1e-6f?v/l:Vec3{}; }

struct Mat4 { float m[16]{}; };
inline Mat4 identity(){ Mat4 r{}; r.m[0]=r.m[5]=r.m[10]=r.m[15]=1; return r; }
inline Mat4 multiply(const Mat4&a,const Mat4&b){
    Mat4 r{};
    for(int c=0;c<4;c++) for(int rr=0;rr<4;rr++)
        for(int k=0;k<4;k++) r.m[c*4+rr]+=a.m[k*4+rr]*b.m[c*4+k];
    return r;
}
inline Mat4 translate(Vec3 v){ Mat4 r=identity(); r.m[12]=v.x;r.m[13]=v.y;r.m[14]=v.z; return r; }
inline Mat4 scale(Vec3 v){ Mat4 r=identity();r.m[0]=v.x;r.m[5]=v.y;r.m[10]=v.z;return r; }
inline Mat4 rotateY(float a){ Mat4 r=identity(); float c=std::cos(a),s=std::sin(a); r.m[0]=c;r.m[8]=s;r.m[2]=-s;r.m[10]=c;return r; }
inline Mat4 rotateZ(float a){ Mat4 r=identity(); float c=std::cos(a),s=std::sin(a); r.m[0]=c;r.m[4]=-s;r.m[1]=s;r.m[5]=c;return r; }
inline Mat4 perspective(float fovy,float aspect,float n,float f){
    Mat4 r{}; float q=1.0f/std::tan(fovy/2.0f); r.m[0]=q/aspect;r.m[5]=q;r.m[10]=(f+n)/(n-f);r.m[11]=-1;r.m[14]=(2*f*n)/(n-f);return r;
}
inline Mat4 lookAt(Vec3 eye,Vec3 center,Vec3 up){
    Vec3 f=normalize(center-eye), s=normalize(cross(f,up)), u=cross(s,f);
    Mat4 r=identity();
    r.m[0]=s.x;r.m[4]=s.y;r.m[8]=s.z;
    r.m[1]=u.x;r.m[5]=u.y;r.m[9]=u.z;
    r.m[2]=-f.x;r.m[6]=-f.y;r.m[10]=-f.z;
    r.m[12]=-dot(s,eye);r.m[13]=-dot(u,eye);r.m[14]=dot(f,eye);return r;
}

inline Vec3 transformPoint(const Mat4&m,Vec3 v){
    float x=m.m[0]*v.x+m.m[4]*v.y+m.m[8]*v.z+m.m[12];
    float y=m.m[1]*v.x+m.m[5]*v.y+m.m[9]*v.z+m.m[13];
    float z=m.m[2]*v.x+m.m[6]*v.y+m.m[10]*v.z+m.m[14];
    return {x,y,z};
}
