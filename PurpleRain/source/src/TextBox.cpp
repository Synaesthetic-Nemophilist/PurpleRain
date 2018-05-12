//
// Created by Jerry Kougianos on 06/05/2018.
//

#include <cctype>
#include "TextBox.h"


using namespace std;

TextBox::TextBox(const std::string &text, TTF_Font *font)
        : text(text), font(font) {
    rect = {5, 720 - 150 - 5, 1280 - 10, 150};

    buildLines();
}

void TextBox::buildLines() {
    int pos = 0;
    string line, lineNext;

    while (pos < text.length()) {
        int prePos = pos;

        lineNext = line;
        while (!isspace(text[pos]) && pos < text.length()) {
            lineNext += text[pos];
            pos++;
        }
        if (pos < text.length()) {
            lineNext += text[pos];
            pos++;
        }

        int w, h;
        if (TTF_SizeUTF8(font, lineNext.c_str(), &w, &h) == 0) {
            if (w > rect.w - 15) {
                // spilled over
                lines.push_back(line);  // add result to new line
                line = lineNext = "";
                pos = prePos;
            }
        } else {
            lines.push_back(lineNext);
        }
        line = lineNext;
    }

    lines.push_back(lineNext);
}

void TextBox::update() {
    time++;

    if (time % 3 == 0) {
        textPosition++;
    }
}


void TextBox::render(SDL_Renderer *renderer) {
    SDL_Rect r = {rect.x, rect.y, rect.w, rect.h};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 140);
    SDL_RenderFillRect(renderer, &r);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &r);

    int y = 0, lineLen = 0;
    for (string &line : lines) {

        string str;  // temp placeholder for incremental text hold based on time
        int textPos = textPosition - lineLen;

        if (textPos >= 0) {
            if (textPos < line.length())
                str = line.substr(0, textPos);
            else
                str = line;
        }

        lineLen += line.length();

        if (!str.empty()) {
            SDL_Color color = {255, 255, 255, 255};
            SDL_Surface *surf = TTF_RenderUTF8_Blended(font, str.c_str(), color);

            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect textRect = {rect.x + 10, y + rect.y + 10 - 48 / 2, surf->w, surf->h};
            SDL_RenderCopy(renderer, texture, NULL, &textRect);

            SDL_FreeSurface(surf);
            y += 32;  // line spacing
        }
    }


}
