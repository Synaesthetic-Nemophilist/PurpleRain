//
// Created by Jerry Kougianos on 05/05/2018.
//

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <fstream>
#include "Scene.h"
#include "GameLoop.h"

using namespace std;

Scene::Scene(SDL_Renderer *renderer, TTF_Font *font, int sceneId, int alternate, int numItems)
        : renderer(renderer), font(font) {
    init(sceneId, alternate, numItems);
}

Scene::Scene(SDL_Renderer *renderer, TTF_Font *font, const string &filename)
        : renderer(renderer), font(font) {
    loadScn(filename);
}

Scene::~Scene() {
    SDL_DestroyTexture(image);
}

void Scene::init(int sceneId, int alternate, int numItems) {
    ostringstream fileName;

    fileName << "./img/scene";
    fileName << sceneId;

    if (alternate) {
        char alt = 'a' + alternate;
        fileName << "_" + alt;
    }

    fileName << ".png";

    SDL_Surface *surf = IMG_Load(fileName.str().c_str());
    if (!surf)
        throw runtime_error("All is lost");

    image = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
}

void Scene::loadScn(const std::string &filepath) {

    ifstream inf(filepath);

    if (inf.is_open()) {
        bool error = false;
        string directive;
        int img = -1;

        while (inf.good()) {
            directive = "";
            inf >> directive;

            if (directive == "IMAGE") {
                // handle image
                inf >> img;
                if (img <= 0) {
                    error = true;
                    break;
                }

            } else if (directive == "NARRATIVE") {
                string text;
                char quote;

                inf >> quote;
                if (quote == '"') {
                    char c = 0;

                    do {
                        c = inf.get();
                        if (c != '"')
                            text += c;
                    } while (c != '"' && inf.good());

                    narrative = text;
                } else {
                    error = true;
                    break;
                }

            } else if (directive == "LINK") {
                // handle link
                string strMouseType, scnPath;
                int mouseType;
                Rect r = {-1};

                inf >> strMouseType;
                inf >> r.x >> r.y >> r.w >> r.h;
                inf >> scnPath;
                if (r.x < 0) {
                    error = true;
                    break;
                }

                // convert captured info from stream to Link struct field types
                if (strMouseType == "up")
                    mouseType = GameLoop::CursorUp;
                else if (strMouseType == "down")
                    mouseType = GameLoop::CursorDown;
                else if (strMouseType == "forward")
                    mouseType = GameLoop::CursorFwd;
                else if (strMouseType == "back")
                    mouseType = GameLoop::CursorBack;
                else if (strMouseType == "right")
                    mouseType = GameLoop::CursorRight;
                else if (strMouseType == "left")
                    mouseType = GameLoop::CursorLeft;

                Link l = {mouseType, r, scnPath};
                links.push_back(l);

            } else if (!directive.empty()) {
                cerr << "Syntax error" << endl;
                error = true;
                break;
            }
        }

        if (error)
            throw runtime_error("Cannot parse scn file");
        else
            init(img, 0, 0);


        inf.close();
    } else {
        throw runtime_error("Cannot find scn file");
    }

    if (!narrative.empty()) {
        textBox = make_shared<TextBox>(narrative, font);
    }

}

void Scene::process(GameLoop *gameLoop, int mx, int my) {
    bool insideAny = false;

    // check links
    for (auto &link : links) {
        // is mouse cursor in the region?
        if (mx > link.region.x && my > link.region.y && mx < link.region.x + link.region.w &&
            my < link.region.y + link.region.h) {
            gameLoop->setCursor(link.mouseCursor);
            insideAny = true;
        }
    }

    if (!insideAny)
        gameLoop->setCursor(GameLoop::CursorNormal);

    if (textBox)
        textBox->update();
}

void Scene::mousePressed(GameLoop *gameLoop, int mx, int my) {
    // check links
    for (auto &link : links) {
        // is mouse cursor in the region?
        if (mx > link.region.x && my > link.region.y && mx < link.region.x + link.region.w &&
            my < link.region.y + link.region.h) {
            // we haved clicked in region
            gameLoop->transitionScene(link.scnFile);
        }
    }
}

void Scene::setOpacity(float op) {
    opacity = op;
}

void Scene::setNarrative(const std::string &str) {
    narrative = str;
}

void Scene::render(SDL_Renderer *renderer) {
    if (opacity < 1.0f) {
        SDL_SetTextureBlendMode(image, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(image, (int) roundf(255 * opacity));
    }

    SDL_RenderCopy(renderer, image, NULL, NULL);

    if (textBox)
        textBox->render(renderer);

    if (opacity < 1.0f) {
        SDL_SetTextureBlendMode(image, SDL_BLENDMODE_NONE);
    }
}
