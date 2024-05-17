#include <iostream>
#include <stdio.h>
#include <glm/gtc/type_ptr.hpp>
#include "SDL.h"
#include "stb_image.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glwindow.h"
#include "geometry.h"
#include <math.h>

using namespace std;

GeometryData geometry;

const char* glGetErrorString(GLenum error)
{
    switch(error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "UNRECOGNIZED";
    }
}

void glPrintError(const char* label="Unlabelled Error Checkpoint", bool alwaysPrint=false)
{
    GLenum error = glGetError();
    if(alwaysPrint || (error != GL_NO_ERROR))
    {
        printf("%s: OpenGL error flag is %s\n", label, glGetErrorString(error));
    }
}

GLuint loadShader(const char* shaderFilename, GLenum shaderType)
{
    FILE* shaderFile = fopen(shaderFilename, "r");
    if(!shaderFile)
    {
        return 0;
    }

    fseek(shaderFile, 0, SEEK_END);
    long shaderSize = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    char* shaderText = new char[shaderSize+1];
    size_t readCount = fread(shaderText, 1, shaderSize, shaderFile);
    shaderText[readCount] = '\0';
    fclose(shaderFile);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char**)&shaderText, NULL);
    glCompileShader(shader);

    delete[] shaderText;

    return shader;
}

GLuint loadShaderProgram(const char* vertShaderFilename,
                       const char* fragShaderFilename)
{
    GLuint vertShader = loadShader(vertShaderFilename, GL_VERTEX_SHADER);
    GLuint fragShader = loadShader(fragShaderFilename, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &logLength, message);
        cout << "Shader load error: " << message << endl;
        return 0;
    }

    return program;
}

OpenGLWindow::OpenGLWindow()
{
}


void OpenGLWindow::initGL()
{
    // We need to first specify what type of OpenGL context we need before we can create the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sdlWin = SDL_CreateWindow("OpenGL Prac 1",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 480, SDL_WINDOW_OPENGL);
    if(!sdlWin)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
    }
    SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
    SDL_GL_MakeCurrent(sdlWin, glc);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = true;
    GLenum glewInitResult = glewInit();
    glGetError(); // Consume the error erroneously set by glewInit()
    if(glewInitResult != GLEW_OK)
    {
        const GLubyte* errorString = glewGetErrorString(glewInitResult);
        cout << "Unable to initialize glew: " << errorString;
    }

    int glMajorVersion;
    int glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
    cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
    cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
    cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
    cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0,0,0,1);

    // Load the model that we want to use and buffer the vertex attributes
    geometry.loadFromOBJFile("sphere-fixed.obj");    

    
}

void OpenGLWindow::render(float a, float b, float theta, float phi, float zoom)
{

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);    

    shader = loadShaderProgram("simple.vert", "simple.frag");
    glUseProgram(shader);

    // Calculate the view matrix for the camera
    glm::vec3 cameraPosition = glm::vec3(10.0f * cos(glm::radians(theta)) * sin(glm::radians(phi)), 10.0f * sin(glm::radians(theta))* sin(glm::radians(phi)), 10.0f * cos(glm::radians(phi)));  // Adjust the position based on your preference
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f);  // Target towards the center of the scene
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);       // Up direction for the camera
    glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);
    GLint viewLoc = glGetUniformLocation(shader, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Calculate the projection matrix (perspective projection)
    float fov = glm::radians(zoom);
    float aspectRatio = 4.0f/3.0f; 
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    glm::mat4 projectionMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
    GLint projectionLoc = glGetUniformLocation(shader, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Lighting and material properties
    glm::vec3 lightPos = glm::vec3(1.2f * cos(glm::radians(theta)), 1.0f, 2.0f * sin(glm::radians(theta))); // Moving light
    glm::vec3 lightAmbient(0.2f, 0.2f, 0.2f);
    glm::vec3 lightDiffuse(0.5f, 0.5f, 0.5f);
    glm::vec3 lightSpecular(1.0f, 1.0f, 1.0f);
    
    glm::vec3 materialAmbient(1.0f, 0.5f, 0.31f);
    glm::vec3 materialDiffuse(1.0f, 0.5f, 0.31f);
    glm::vec3 materialSpecular(0.5f, 0.5f, 0.5f);
    float materialShininess = 32.0f;

    glUniform3fv(glGetUniformLocation(shader, "light.position"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(shader, "light.ambient"), 1, glm::value_ptr(lightAmbient));
    glUniform3fv(glGetUniformLocation(shader, "light.diffuse"), 1, glm::value_ptr(lightDiffuse));
    glUniform3fv(glGetUniformLocation(shader, "light.specular"), 1, glm::value_ptr(lightSpecular));

    // Pass material information to shaders
    glUniform3fv(glGetUniformLocation(shader, "material.ambient"), 1, glm::value_ptr(materialAmbient));
    glUniform3fv(glGetUniformLocation(shader, "material.diffuse"), 1, glm::value_ptr(materialDiffuse));
    glUniform3fv(glGetUniformLocation(shader, "material.specular"), 1, glm::value_ptr(materialSpecular));
    glUniform1f(glGetUniformLocation(shader, "material.shininess"), materialShininess);

    // Pass view position to shaders
    glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(cameraPosition));
    
    vertexCount = geometry.vertexCount();

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    string images[3] = {"sun_texture.png", "earth_diffuse.png", "moon_diffuse.png"};
    std::vector<GLuint> textures(3);
    glGenTextures(3, textures.data()); 
    for (int i = 0; i < 3; i++){
               
        glBindTexture(GL_TEXTURE_2D, textures[i]);  

        int widthImg, heightImg, numColCh;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* bytes = stbi_load(images[i].c_str(), &widthImg, &heightImg, &numColCh, 4);        
        if (bytes)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(bytes);

        // Set the texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        
    }

    auto vertices = static_cast<float*>(geometry.vertexData());  
    auto texCoords = static_cast<float*>(geometry.textureCoordData());
    auto normals = static_cast<float*>(geometry.normalData());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int widthImg, heightImg, numColCh;       
    
    for (int i = 0; i < 3; ++i) {
        glm::vec3 position, scale;
        if (i == 0) {
            position = glm::vec3(0.0f, 0.0f, -3.0f);
            scale = glm::vec3(1.0f);
        } else if (i == 1) {
            position = glm::vec3(4.0f * cos(glm::radians(a)), 4.0f * sin(glm::radians(a)), -3.0f);
            scale = glm::vec3(0.3f);
        } else if (i == 2) {
            position = glm::vec3(4.0f * cos(glm::radians(a)) + 1.5f * cos(glm::radians(b)), 4.0f * sin(glm::radians(a)) + 1.5f * sin(glm::radians(b)), -3.0f);
            scale = glm::vec3(0.1f);
        }        

        std::vector<float> combinedData;

        for (int j = 0; j < vertexCount; ++j) {
            combinedData.push_back(vertices[j * 3]);
            combinedData.push_back(vertices[j * 3 + 1]);
            combinedData.push_back(vertices[j * 3 + 2]);
            combinedData.push_back(texCoords[j * 2]);
            combinedData.push_back(texCoords[j * 2 + 1]);
            combinedData.push_back(normals[j * 3]);
            combinedData.push_back(normals[j * 3 + 1]);
            combinedData.push_back(normals[j * 3 + 2]);
        }
        
        // Calculate the view model matrix for each instance
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
        modelMatrix = glm::scale(modelMatrix, scale);
        GLint modelLoc = glGetUniformLocation(shader, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));      
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * combinedData.size(), combinedData.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);    
        
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        
    }   
    // glPrintError("Setup complete", true);

    // Swap the front and back buffers on the window, effectively putting what we just "drew"
    // onto the screen (whereas previously it only existed in memory)
    SDL_GL_SwapWindow(sdlWin);
}

// The program will exit if this function returns false
bool OpenGLWindow::handleEvent(SDL_Event e)
{
    // A list of keycode constants is available here: https://wiki.libsdl.org/SDL_Keycode
    // Note that SDL provides both Scancodes (which correspond to physical positions on the keyboard)
    // and Keycodes (which correspond to symbols on the keyboard, and might differ across layouts)
    if(e.type == SDL_KEYDOWN)
    {
        if(e.key.keysym.sym == SDLK_ESCAPE)
        {
            return false;
        }
    }
    return true;
}

void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
    SDL_DestroyWindow(sdlWin);
}
