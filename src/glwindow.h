#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>

#include "geometry.h"

class OpenGLWindow
{
public:
    OpenGLWindow();
    void initGL();
    void render(float a, float b, float theta, float phi, float zoom);
    bool handleEvent(SDL_Event e);
    void cleanup();

private:
    SDL_Window* sdlWin;

    GLuint vao;
    GLuint shader;
    GLuint vertexBuffer;
    GLuint elementBuffer;
    GLuint vertexCount;
};

#endif
