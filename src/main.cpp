#include "SDL.h"
#include "glwindow.h"

// In order to make cross-platform development and deployment easy, SDL implements its own main
// function, and instead calls out to our code at this SDL_main, however on linux this is not
// needed (since the entrypoint in linux is already called main) so to keep things portable
// we need to check our environment at compile-time
#ifdef __linux__
int main(int argc, char** argv)
#else
int SDL_main(int argc, char** argv)
#endif
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to initialize SDL", 0);
        return 1;
    }

    OpenGLWindow window;
    window.initGL();
    
    float beta = 0.0f, alpha = 0.0f, theta = 0.0f, phi = 0.0f;
    float betaIncrement = 4.0f, alphaIncrement = 1.0f;
    bool running = true, pause = true;
    float zoom = 150.0f, x = 0.0f, y = 0.0f;
    while(running)
    {
        // Check for a quit event before passing to the GLWindow
        
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
            {
                running = false;
            }
            else if(e.type == SDL_KEYDOWN)
            {
                
                switch(e.key.keysym.sym)
                {
                    case SDLK_UP:
                        alphaIncrement += 1.0f;
                        break;
                    case SDLK_DOWN:
                        alphaIncrement -= 1.0f; 
                        break;
                    case SDLK_LEFT:
                        betaIncrement -= 2.0f;
                        break;
                    case SDLK_RIGHT:
                        betaIncrement += 2.0f; 
                        break;
                    case SDLK_SPACE: 
                        pause = !pause;
                        break;
                    case SDLK_s: 
                        pause = true;
                        break;
                    case SDLK_r: 
                        pause = false;
                        break;
                    case SDLK_y: 
                        theta += 1.0f;
                        break;
                    case SDLK_z: 
                        phi += 1.0f;
                        break;
                }
            }
            else if (e.type == SDL_MOUSEWHEEL)
            {
                // Check the scroll direction
                zoom += e.wheel.y;                
            }            
            else if(!window.handleEvent(e))
            {
                running = false;
            }
            
        }
        alphaIncrement = alphaIncrement < 1.0f ? 1.0f : alphaIncrement;
        betaIncrement = betaIncrement <= alphaIncrement ? alphaIncrement + 1.0f : betaIncrement;
        window.render(alpha, beta, theta, phi, zoom);
        if(!pause || alpha == 0.0f) // Only update alpha and beta if animation is running
        {                        
            alpha += alphaIncrement;
            beta += betaIncrement;
        }
        

        // We sleep for 10ms here so as to prevent excessive CPU usage
        SDL_Delay(10);
    }

    window.cleanup();
    SDL_Quit();
    return 0;
}

