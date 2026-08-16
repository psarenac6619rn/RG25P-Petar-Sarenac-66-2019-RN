#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;

uniform mat4 model, view, projection;
uniform float time;
uniform int dmtMode;

out vec3 ViewPos;
out vec3 ViewNormal;
out vec2 UV;

void main(){
    vec3 localPos = aPos;

    // DMT Visual mode: blaga, glatka deformacija geometrije.
    // Namerno je mala da scena ostane citljiva i da efekat moze da se objasni na odbrani.
    if(dmtMode == 1){
        vec4 baseWorld = model * vec4(aPos, 1.0);
        float wave1 = sin(baseWorld.x * 1.35 + time * 1.8);
        float wave2 = cos(baseWorld.z * 1.15 - time * 1.35);
        float pulse = sin(time * 1.25 + baseWorld.y * 1.8);
        localPos += aNormal * (0.055 * wave1 + 0.040 * wave2 + 0.025 * pulse);
        localPos.x += 0.020 * sin(time * 1.6 + baseWorld.y * 2.2);
        localPos.z += 0.018 * cos(time * 1.4 + baseWorld.x * 1.7);
    }

    vec4 wp = model * vec4(localPos, 1.0);
    vec4 vp = view * wp;
    ViewPos = vp.xyz;
    ViewNormal = mat3(transpose(inverse(view * model))) * aNormal;
    UV = aUV;
    gl_Position = projection * vp;
}
