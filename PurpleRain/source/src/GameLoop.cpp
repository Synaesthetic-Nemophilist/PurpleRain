//
// Created by Jerry Kougianos on 05/05/2018.
//

#include <stdexcept>
#include "GameLoop.h"

using namespace std;

//Constructor, set up the game loop
//on this object's creation
GameLoop::GameLoop() {

    //Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    //Create an application window with the following settings:
    window = SDL_CreateWindow("Purple Rain",             // window title
                              SDL_WINDOWPOS_UNDEFINED,       // initial x position
                              SDL_WINDOWPOS_UNDEFINED,       // initial y position
                              1280,                          // width, in pixels
                              720,                           // height, in pixels
                              0                              // flags
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_ShowCursor(0);

    //load mouse cursors
    const char *cursorNames[8] = {
            "./img/cursor_back.png",
            "./img/cursor_down.png",
            "./img/cursor_fwd.png",
            "./img/cursor_item.png",
            "./img/cursor_left.png",
            "./img/cursor_normal.png",
            "./img/cursor_right.png",
            "./img/cursor_up.png"
    };
    for (int i = 0; i < 8; i++) {
        SDL_Surface *surf = IMG_Load(cursorNames[i]);

        if (!surf)
            throw runtime_error("Error finding mouse cursors");

        mouseCursors[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
    mouseCursor = mouseCursors[CursorNormal];

    // Font init
    TTF_Init();
    font = TTF_OpenFont("./fonts/Crazy-Pixel.ttf", 48);
    if (!font)
        throw runtime_error("Font not found");

    // AudioManager & Bg music init
    m_audioManager.init();
    m_bgMusic = m_audioManager.LoadMusic("./audio/music/Micromelancolié - work-life balance.mp3");
    m_bgMusic.play();

    loadScene("./scenes/scene1.scn");

}

//Destructor
//Shutdown game
GameLoop::~GameLoop() {
    // Close and destroy the window
    SDL_DestroyWindow(window);
    // Destroy the renderer
    SDL_DestroyRenderer(renderer);
    // Destroy mouse textures
    for (auto &mouseCursor : mouseCursors)
        SDL_DestroyTexture(mouseCursor);

    // Closed used font
    TTF_CloseFont(font);

    // Stop bg music
    Music::Stop();

    // Destroy Audio manager
    m_audioManager.destroy();

    // Shut-down ttf engine
    TTF_Quit();

    // Clean up
    SDL_Quit();
}

void GameLoop::loadScene(const string &scn) {
    scene = make_shared<Scene>(renderer, font, scn);
}

void GameLoop::transitionScene(const string &scn) {
    incomingScene = make_shared<Scene>(renderer, font, scn);
    transitionfade = 0;
}

void GameLoop::setCursor(int cursor) {
    mouseCursor = mouseCursors[cursor];
}

int GameLoop::processEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_WINDOWEVENT_CLOSE: {
                if (window) {
                    SDL_DestroyWindow(window);
                    window = nullptr;
                    done = true;
                }
            }
                break;
            case SDL_MOUSEBUTTONDOWN: {
                if (!incomingScene) {  // disallow clicks when transitioning scenes
                    scene->mousePressed(this, event.button.x, event.button.y);
                }
            }
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        done = true;
                        break;
                }
            }
                break;
            case SDL_QUIT:
                //quit out of the game
                done = true;
                break;
        }
    }

    //if we wanted to handle keyboard input, this is where we'd do it
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_LEFT]) {
        //...
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        //...
    }
    if (state[SDL_SCANCODE_UP]) {
        //...
    }
    if (state[SDL_SCANCODE_DOWN]) {
        //...
    }

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    scene->process(this, mouseX, mouseY);

    return done;

}

void GameLoop::updateLogic() {

    // if transition exists, here is the update logic for cross-fade
    if (incomingScene) {
        transitionfade += 0.02f;
        if (transitionfade > 1.0f) {
            transitionfade = 1.0f;
            // destruct incoming scene ptr
            scene = incomingScene;
            incomingScene = nullptr;
        }
    }
}

void GameLoop::doRender() {
    //SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    //SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    scene->render(renderer);

    // Render cross-fade between incoming and current scene
    if (incomingScene) {
        incomingScene->setOpacity(transitionfade);
        incomingScene->render(renderer);
    }

    // Draw cursor
    SDL_Rect cursorRect = {mouseX, mouseY, 64, 64};
    SDL_RenderCopy(renderer, mouseCursor, NULL, &cursorRect);

    //We are done drawing, "present" or show to the screen what we've drawn
    SDL_RenderPresent(renderer);
}

//Run the game event loop
void GameLoop::run() {
    //Event loop
    while (!done) {
        //Check for events
        done = processEvents();

        //Update logic
        updateLogic();

        //Render display
        doRender();

        //don't burn up the CPU
        SDL_Delay(10);
    }
}
