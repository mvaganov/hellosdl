#include "sdlengine.h"
#include "stringstuff.h"
#include <map>
#include <cctype>
#include <SDL_image.h>
#include <algorithm>

/// Reimplements C#'s nameof operator
#define nameof(thing)	#thing
#define CLEAR_ARRAY(arr) memset(arr, 0, sizeof(arr))

System * System::_instance = NULL;

void System::FailFast() {
	if (_errorMessage != "") {
		printf(_errorMessage.c_str());
		exit((int)System::ErrorCode::Failure);
	}
}

System::System(int width, int height) : _window(NULL), _screenSurface(NULL), _width(width), _height(height),
_rendererKind(Renderer::None), _running(false), _initialized(false), _isPressedKeyMask(), _isPressedKeyMaskScancode(),
_managedSurfaces() {
	_errorMessage = "";
	if (_instance) {
		_instance = this;
	}
	CLEAR_ARRAY(_isPressedKeyMask);
	CLEAR_ARRAY(_isPressedKeyMaskScancode);
}

System::~System() {
	Release();
}

System::ErrorCode System::Release()
{
	for (int i = 0; i < _managedSurfaces.size(); ++i) {
		SDL_Surface* loadedSurface = _managedSurfaces[i];
		if (loadedSurface == NULL) {
			continue;
		}
		_managedSurfaces[i] = NULL;
		SDL_FreeSurface(loadedSurface);
	}
	for (int i = 0; i < _managedTextures.size(); ++i) {
		SDL_Texture* loadedTexture = _managedTextures[i];
		if (loadedTexture == NULL) {
			continue;
		}
		_managedTextures[i] = NULL;
		SDL_DestroyTexture(loadedTexture);
	}
	switch (_rendererKind) {
	case Renderer::SDL_Renderer:
		if (_gRenderer != NULL) {
			SDL_DestroyRenderer(_gRenderer);
			_gRenderer = NULL;
		}
		break;
	}
	if (_window != NULL)
	{
		SDL_DestroyWindow(_window);
	}
	if (_initialized) {
		IMG_Quit();
		SDL_Quit();
	}
	return System::ErrorCode::Success;
}

System::ErrorCode System::Init(std::string windowName, Renderer renderer)
{
	if (_initialized) {
		// TODO change windowname, or change renderer
		_errorMessage = string_format("Re-initialization is unsupported");
		return System::ErrorCode::NotImplemented;
	}
	_running = false;
	_rendererKind = Renderer::None;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		_errorMessage = string_format("SDL could not initialize! SDL_Error: %s", SDL_GetError());
		return System::ErrorCode::InitializationFailure;
	}
	//Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}

	_initialized = true;
	_window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width, _height, SDL_WINDOW_SHOWN);
	if (_window == NULL)
	{
		_errorMessage = string_format("Window could not be created! SDL_Error: %s", SDL_GetError());
		return System::ErrorCode::WindowCreationFailure;
	}

	_rendererKind = renderer;
	System::ErrorCode errorCode = ErrorCode::NotImplemented;
	switch (renderer) {
	case Renderer::SDL_Surface:
		errorCode = InitSDL_Surface();
		break;
	case Renderer::SDL_Renderer:
		errorCode = InitSDL_Renderer();
		break;
	default:
		_errorMessage = string_format("Renderer %d not implemented!", renderer);
		errorCode = ErrorCode::NotImplemented;
		break;
	}
	// TODO create a separate image module, and be able to query if these image types can be loaded: IMG_INIT_JPG, IMG_INIT_PNG, IMG_INIT_TIF, IMG_INIT_WEBP, IMG_INIT_JXL, IMG_INIT_AVIF
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		_errorMessage = string_format("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		return ErrorCode::CapabilityLoadFailed;
	}
	if (errorCode != ErrorCode::Success) {
		return errorCode;
	}
	_running = true;
	return ErrorCode::Success;
}

bool System::IsRunning() {
	return _running;
}

SDL_Surface* System::GetScreenSurface() { return this->_screenSurface; }

SDL_Renderer* System::GetRenderer() { return this->_gRenderer; }

void System::ClearGraphics() {
	switch (_rendererKind) {
	case Renderer::SDL_Surface:
		SDL_FillRect(_screenSurface, NULL, SDL_MapRGBA(_screenSurface->format, 0xFF, 0xFF, 0xFF, 0x00));
		break;
	case Renderer::SDL_Renderer:
		SDL_RenderClear(_gRenderer);
		break;
	}
}

void System::Render() {
	switch (_rendererKind) {
	case Renderer::SDL_Surface:
		SDL_UpdateWindowSurface(_window);
		break;
	case Renderer::SDL_Renderer:
		SDL_RenderPresent(_gRenderer);
		break;
	}
}

void ExecuteDelegates(System::SdlEventDelegateList& delegates, SDL_Event e) {
	for (int i = 0; i < delegates.size(); ++i) {
		delegates[i](e);
	}
}

void System::ProcessInput() {
	//Hack to get _window to stay up
	SDL_Event e;
	std::map<int, SdlEventDelegateList>::iterator found;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			_running = false;
			break;
		case SDL_KEYDOWN:
			SetPressed(e.key.keysym.sym, true);
			found = _keyBindDown.find(e.key.keysym.sym);
			if (found != _keyBindDown.end()) {
				ExecuteDelegates(found->second, e);
			}
			break;
		case SDL_KEYUP:
			SetPressed(e.key.keysym.sym, false);
			found = _keyBindUp.find(e.key.keysym.sym);
			if (found != _keyBindUp.end()) {
				ExecuteDelegates(found->second, e);
			}
			break;
		case SDL_MOUSEMOTION:
			MousePosition.x = e.motion.x;
			MousePosition.y = e.motion.y;
			printf("mousemotion x%d, y%d, type%d, dx%d, dy%d\n",
				e.motion.x, e.motion.y, e.motion.type, e.motion.xrel, e.motion.yrel);
			break;
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			MousePosition.x = e.button.x;
			MousePosition.y = e.button.y;
			printf("mousemotion x%d, y%d, type%d, clicks%d, which%d, state%d, button%d\n",
				e.button.x, e.button.y, e.button.type, e.button.clicks, e.button.which, e.button.state, e.button.button);
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

System::ErrorCode System::InitSDL_Renderer() {
	_gRenderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	if (_gRenderer == NULL)
	{
		_errorMessage = string_format("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return System::ErrorCode::WindowCreationFailure;
	}
	SDL_SetRenderDrawColor(_gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	return System::ErrorCode::Success;
}

System::ErrorCode System::SetPressed(int sdlk, bool pressed) {
	// TODO do some performance analysis. is int faster than longlong and unsignedchar?
	int* field = NULL;
	if (sdlk >= 0 && sdlk < 256) {
		field = this->_isPressedKeyMask;
	} else if ((sdlk & SDLK_SCANCODE_MASK) != 0) {
		sdlk -= SDLK_SCANCODE_MASK;
		field = this->_isPressedKeyMaskScancode;
#ifndef NDEBUG
		if (sdlk < 0 || sdlk >= SDL_NUM_SCANCODES) {
			_errorMessage = string_format("Unknown scancode %d", sdlk);
			return ErrorCode::InputError;
		}
	} else {
		_errorMessage = string_format("Unknown keycode %d", sdlk);
		return ErrorCode::InputError;
#endif
	}
	int intIndex = sdlk >> 5; // bitwise equivalent of / 32
	int bitIndex = sdlk & ((1 << 5) - 1);
	field += intIndex;
	//printf("\npressed(%d) %d: %d %d\n", pressed, sdlk, intIndex, bitIndex);
	if (pressed) {
		*field |= (1 << bitIndex);
	} else {
		*field &= ~(1 << bitIndex);
	}
	return ErrorCode::Success;
}

System::ErrorCode System::IsPressed(int sdlk, bool& out_pressed) {
	int* field = NULL;
	if (sdlk >= 0 && sdlk < 256) {
		field = this->_isPressedKeyMask;
	} else if ((sdlk & SDLK_SCANCODE_MASK) != 0) {
		sdlk -= SDLK_SCANCODE_MASK;
		field = this->_isPressedKeyMaskScancode;
#ifndef NDEBUG
		if (sdlk < 0 || sdlk >= SDL_NUM_SCANCODES) {
			_errorMessage = string_format("Unknown scancode %d", sdlk);
			return ErrorCode::InputError;
		}
	} else {
		_errorMessage = string_format("Unknown keycode %d", sdlk);
		return ErrorCode::InputError;
#endif
	}
	int intIndex = sdlk >> 5; // bitwise equivalent of / 32
	int bitIndex = sdlk & ((1 << 5) - 1);
	field += intIndex;
	out_pressed = (*field & (1 << bitIndex)) != 0;
	return ErrorCode::Success;
}

static bool ends_with(std::string str, std::string suffix)
{
	return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string str_tolower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), std::tolower);
	// this forces 8bit ascii
	//std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
	return s;
}

System::ErrorCode System::LoadSdlSurfaceBasic(std::string path, SDL_Surface*& out_surface) {
	std::string lowercasePath = str_tolower(path);
	if (ends_with(lowercasePath, "bmp")) {
		out_surface = SDL_LoadBMP(path.c_str());
	}
	else if (ends_with(lowercasePath, "png")) {
		out_surface = IMG_Load(path.c_str());
	}
	else {
		_errorMessage = string_format("Unable to load image format %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		return ErrorCode::UnsupportedFormat;
	}
	if (out_surface == NULL)
	{
		_errorMessage = string_format("Failed to load image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		return ErrorCode::Failure;
	}
	return ErrorCode::Success;
}

System::ErrorCode System::LoadSdlSurface(std::string path, SDL_Surface*& out_surface) {
	SDL_Surface* loadedSurface = NULL;
	ErrorCode err = LoadSdlSurfaceBasic(path, loadedSurface);
	if (err != ErrorCode::Success) { return err; }
	// optimize surface for blitting
	out_surface = SDL_ConvertSurface(loadedSurface, _screenSurface->format, 0);
	if (out_surface == NULL)
	{
		_errorMessage = string_format("Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		return ErrorCode::Failure;
	}
	SDL_FreeSurface(loadedSurface);
	_managedSurfaces.push_back(out_surface);
	return ErrorCode::Success;
}

System::ErrorCode System::LoadSdlTexture(std::string path, SDL_Texture*& out_texture) {
	SDL_Surface* loadedSurface = NULL;
	ErrorCode err = LoadSdlSurfaceBasic(path, loadedSurface);
	if (err != ErrorCode::Success) { return err; }
	//printf("have img %x\n", loadedSurface);
	out_texture = SDL_CreateTextureFromSurface(_gRenderer, loadedSurface);
	if (out_texture == NULL)
	{
		_errorMessage = string_format("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		return ErrorCode::Failure;
	}
	SDL_FreeSurface(loadedSurface);
	_managedTextures.push_back(out_texture);
	return ErrorCode::Success;
}

#undef nameof
#undef CLEAR_ARRAY
