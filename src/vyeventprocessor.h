#pragma once

#include <SDL.h>
#include <string>

class VyEventProcessor {
public:
	virtual void HandleEvent(const SDL_Event& e) = 0;
};

class VyDrawable {
public:
	virtual void Draw(SDL_Renderer* g) = 0;
};

class VyUpdatable {
public:
	virtual void Update() = 0;
};

class VyInterface {
public:
	virtual const std::string& GetName() const = 0;
	virtual void SetName(std::string name) = 0;
	virtual VyEventProcessor* AsEventProcessor() = 0;
	virtual VyDrawable* AsDrawable() = 0;
	virtual VyUpdatable* AsUpdatable() = 0;
};
