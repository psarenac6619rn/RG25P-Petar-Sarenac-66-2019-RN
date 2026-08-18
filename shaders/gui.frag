#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D text;
uniform vec3 textColor;
uniform int hasTex;

void main() {
    if (hasTex == 1) {
        float sampled = texture(text, TexCoords).r;
        if (sampled < 0.1) discard; // Don't draw background pixels
        FragColor = vec4(textColor, sampled);
    } else {
        FragColor = vec4(textColor, 1.0);
    }
}
