#version 330 core
layout(location=0) out vec3 gPosition; layout(location=1) out vec3 gNormal; layout(location=2) out vec4 gAlbedo;
in vec3 ViewPos; in vec3 ViewNormal; in vec2 UV;
uniform sampler2D normalMap; uniform vec3 baseColor; uniform float useNormalMap;
void main(){
 vec3 n=normalize(ViewNormal); vec3 mapN=texture(normalMap,UV).xyz*2.0-1.0;
 n=normalize(mix(n, normalize(n + vec3(mapN.x,0.0,mapN.y)*0.38), useNormalMap));
 gPosition=ViewPos; gNormal=n; float grid=smoothstep(.46,.5,abs(fract(UV.x)-.5))+smoothstep(.46,.5,abs(fract(UV.y)-.5));
 gAlbedo=vec4(baseColor*(0.82+0.18*grid),0.42); // alpha is material specular strength
}
