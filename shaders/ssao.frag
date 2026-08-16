#version 330 core
out float FragColor; in vec2 UV;
uniform sampler2D gPosition,gNormal,noiseTex; uniform vec3 samples[32]; uniform mat4 projection; uniform vec2 noiseScale;
void main(){vec3 p=texture(gPosition,UV).xyz,n=normalize(texture(gNormal,UV).xyz),rnd=texture(noiseTex,UV*noiseScale).xyz;vec3 t=normalize(rnd-n*dot(rnd,n));vec3 b=cross(n,t);mat3 TBN=mat3(t,b,n);float occ=0.0;for(int i=0;i<32;i++){vec3 sp=p+TBN*samples[i]*0.7;vec4 off=projection*vec4(sp,1);off.xyz/=off.w;off.xyz=off.xyz*.5+.5;float sampleDepth=texture(gPosition,off.xy).z;float range=smoothstep(0,1,0.7/abs(p.z-sampleDepth));occ+=(sampleDepth>=sp.z+0.025?1.0:0.0)*range;}FragColor=1.0-occ/32.0;}
