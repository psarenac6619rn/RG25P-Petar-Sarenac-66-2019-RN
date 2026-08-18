#pragma once
#include <GL/glew.h>
#include <string>
#include "OutpostMath.h"
#include "Shader.h"

class GUI {
public:
    GUI(int screenWidth, int screenHeight);
    ~GUI();

    void resize(int w, int h);
    void begin();
    void end();

    void drawCrosshair();
    void drawText(const std::string& text, float x, float y, float scale, Vec3 color);

private:
    int W, H;
    GLuint vao, vbo;
    GLuint fontTex;
    
    Shader* shader;

    void initFont();
};
