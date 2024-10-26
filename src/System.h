#pragma once

#include <SDL.h>
#include <string>
#include <map>
#include <functional>

class System
{
public:
	enum class ErrorCode;
	enum class Renderer;
private:
	static System* _instance;
	std::string _errorMessage;
	SDL_Window* _window = NULL;
	SDL_Surface* _screenSurface = NULL;
	int _width, _height;
	Renderer _renderer;
	std::map<int, std::function<void(int)>> _keyBindDown;
	std::map<int, std::function<void(int)>> _keyBindUp;
	std::map<int, bool> _isPressed;
	bool _running;
public:
	System(int width, int height);
	ErrorCode Init(std::string windowName, Renderer renderer);
	ErrorCode Release();
	void Render();
	void ProcessInput();
private:
	System::ErrorCode InitSDL_Surface();
	System::ErrorCode InitHardwareRenderer();
};

