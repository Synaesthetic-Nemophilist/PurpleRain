//
// Created by Jerry Kougianos on 08/05/2018.
//

#include <cerrno>
#include <iostream>
#include "AudioManager.h"

using namespace std;


AudioManager::AudioManager() {}

AudioManager::~AudioManager() {
    destroy();
}

void AudioManager::init() {
    if (m_isInitialized)
        throw runtime_error("Audio manager already initialised");

    // init audio engine
    if (Mix_Init(MIX_INIT_OGG || MIX_INIT_MP3) == -1)
        throw runtime_error("SDL_mixer failed to initialise: " + string(SDL_GetError()));

    // set audio specs
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
        throw runtime_error("Mix_OpenAudio failed: " + string(SDL_GetError()));

    m_isInitialized = true;
}

void AudioManager::destroy() {
    if (m_isInitialized) {
        m_isInitialized = false;

        // Release audio resources
        for (auto &iter : m_effectList)
            Mix_FreeChunk(iter.second);
        for (auto &iter : m_musicList)
            Mix_FreeMusic(iter.second);
        Mix_CloseAudio();
        Mix_Quit();
    }
}

SoundEffect AudioManager::LoadSoundEffect(const std::string &filepath) {
    SoundEffect effect;

    // search for effect in cache b4 loading
    auto iter = m_effectList.find(filepath);

    // if not found, load...
    if (iter == m_effectList.end()) {
        Mix_Chunk *chunk = Mix_LoadWAV(filepath.c_str());

        if (chunk == nullptr)
            throw runtime_error("Mix_LoadWAV failed: " + string(SDL_GetError()));

        effect.m_chunk = chunk;
        m_effectList[filepath] = chunk;
    } else {  // found in cache
        effect.m_chunk = iter->second;
    }
    return effect;
}

Music AudioManager::LoadMusic(const std::string &filepath) {
    Music music;

    // search for effect in cache b4 loading
    auto iter = m_musicList.find(filepath);

    // if not found, load...
    if (iter == m_musicList.end()) {
        Mix_Music *chunk = Mix_LoadMUS(filepath.c_str());

        if (chunk == nullptr)
            throw runtime_error("Mix_LoadMUS failed: " + string(SDL_GetError()));

        music.mix_music = chunk;
        m_musicList[filepath] = chunk;
    } else {  // found in cache
        music.mix_music = iter->second;
    }
    return music;
}

void SoundEffect::play(int numOfLoops) {
    if (Mix_PlayChannel(-1, m_chunk, numOfLoops) == -1)
        throw runtime_error("Mix_PlayChannel error: " + string(SDL_GetError()));
}

void Music::play(int numOfLoops) {
    if (Mix_PlayMusic(mix_music, numOfLoops) == -1)
        throw runtime_error("Mix_PlayMusic error: " + string(SDL_GetError()));
}
