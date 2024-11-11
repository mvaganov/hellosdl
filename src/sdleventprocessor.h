#pragma once

#include <SDL.h>
#include <string>

class SdlEventProcessor {
public:
	virtual void ProcessInput(const SDL_Event& e) = 0;
};

class SdlDrawable {
public:
	virtual void Draw(SDL_Renderer* g) = 0;
};

class SdlUpdatable {
public:
	virtual void Update() = 0;
};

class SdlNamed {
public:
	virtual std::string GetName() = 0;
	virtual void SetName(std::string name) = 0;
};
