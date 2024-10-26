#include "system.h"
#include "stringstuff.h"
#include <map>

/// Reimplements C#'s nameof operator
#define NAMEOF(thing)	#thing

System * System::_instance = NULL;

enum class System::ErrorCode { Success = 0, Failure = -1, NotImplemented = -2, InitializationFailure = 1, WindowCreationFailure = 2 };
enum class System::Renderer{ None = 0, SDL_Surface = 1, HardwareRenderer = 2 };

System::System(int width, int height) : _window(NULL), _screenSurface(NULL), _width(width), _height(height) {
	if (_instance) {
		_instance = this;
	}
}

System::ErrorCode System::Release()
{
	if (_window != NULL)
	{
		SDL_DestroyWindow(_window);
	}
	SDL_Quit();
	return System::ErrorCode::Success;
}

System::ErrorCode System::Init(std::string windowName, Renderer renderer)
{
	_running = false;
	_renderer = Renderer::None;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		_errorMessage = string_format("SDL could not initialize! SDL_Error: %s", SDL_GetError());
		return System::ErrorCode::InitializationFailure;
	}
	_window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width, _height, SDL_WINDOW_SHOWN);
	if (_window == NULL)
	{
		_errorMessage = string_format("Window could not be created! SDL_Error: %s", SDL_GetError());
		return System::ErrorCode::WindowCreationFailure;
	}

	_renderer = renderer;
	System::ErrorCode errorCode = ErrorCode::NotImplemented;
	switch (renderer) {
	case Renderer::SDL_Surface:
		errorCode = InitSDL_Surface();
		break;
	case Renderer::HardwareRenderer:
		errorCode = InitSDL_Surface();
		break;
	default:
		_errorMessage = string_format("Renderer %d not implemented!", renderer);
		errorCode = ErrorCode::NotImplemented;
		break;
	}
	if (errorCode != ErrorCode::Success) {
		return errorCode;
	}
}

void System::Render() {
	switch (_renderer) {
	case Renderer::SDL_Surface:
		SDL_UpdateWindowSurface(_window);
		break;
	case Renderer::HardwareRenderer:
		break;
	}
}

void System::ProcessInput() {
	//Hack to get _window to stay up
	SDL_Event e;
	std::map<int, std::function<void(int)>>::iterator found;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			_running = false;
			break;
		case SDL_KEYDOWN:
			_isPressed[e.key.keysym.sym] = true;
			found = _keyBindDown.find(e.key.keysym.sym);
			if (found != _keyBindDown.end()) {
				found->second(e.key.keysym.sym);
			}
			break;
		case SDL_KEYUP:
			_isPressed[e.key.keysym.sym] = false;
			found = _keyBindUp.find(e.key.keysym.sym);
			if (found != _keyBindUp.end()) {
				found->second(e.key.keysym.sym);
			}
			break;
		}
	}
}

System::ErrorCode System::InitSDL_Surface() {
	_screenSurface = SDL_GetWindowSurface(_window);
	if (_screenSurface == NULL)
	{
		_errorMessage = string_format("Window surface could not be retrieved! SDL_Error: %s", SDL_GetError());
		return System::ErrorCode::WindowCreationFailure;
	}
	SDL_FillRect(_screenSurface, NULL, SDL_MapRGB(_screenSurface->format, 0xFF, 0xFF, 0xFF));
	return System::ErrorCode::Success;
}

System::ErrorCode System::InitHardwareRenderer() {
	_errorMessage = string_format("Need to implement " NAMEOF(InitHardwareRenderer), SDL_GetError());
	return System::ErrorCode::NotImplemented;
}

#undef NAMEOF
