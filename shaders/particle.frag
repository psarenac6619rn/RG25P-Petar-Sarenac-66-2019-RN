#version 330 core
out vec4 FragColor; void main(){float d=length(gl_PointCoord-vec2(.5));if(d>.5)discard;float a=(.5-d)*1.6;FragColor=vec4(0.15,0.75,1.4,a);}

