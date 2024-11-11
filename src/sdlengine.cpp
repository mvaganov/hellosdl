#include "sdlengine.h"
#include "stringstuff.h"
#include <map>
#include <cctype>
#include <SDL_image.h>
#include <algorithm>

/// Reimplements C#'s nameof operator
#define nameof(thing)	#thing
#define CLEAR_ARRAY(arr) memset(arr, 0, sizeof(arr))

SdlEngine * SdlEngine::_instance = NULL;

void SdlEngine::FailFast() {
	if (ErrorMessage != "") {
		printf(ErrorMessage.c_str());
		exit((int)SdlEngine::ErrorCode::Failure);
	}
}

SdlEngine::SdlEngine(int width, int height) : MouseClickState(0), _window(NULL), _screenSurface(NULL), _width(width), _height(height),
_rendererKind(Renderer::None), _running(false), _initialized(false), _currentFont(NULL),
_isPressedKeyMask(), _isMousePressed(), _isPressedKeyMaskScancode(),
_managedSurfaces(), _fonts(), _eventProcessors(), _todo(NULL), _todoNow(NULL) {
	ErrorMessage = "";
	if (_instance == NULL) {
		_instance = this;
	} else {
		printf("duplicate SdlEngine being created? already have at %016llux", (size_t)_instance);
	}
	CLEAR_ARRAY(_isPressedKeyMask);
	CLEAR_ARRAY(_isPressedKeyMaskScancode);
	CLEAR_ARRAY(_isMousePressed);
	_todo = new std::vector<DelegateNextFrame>();
	_todoNow = new std::vector<DelegateNextFrame>();
}

SdlEngine::~SdlEngine() {
	Release();
}

SdlEngine::ErrorCode SdlEngine::Release()
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
		SDL_Texture* loadedTexture = (SDL_Texture*)_managedTextures[i];
		if (loadedTexture == NULL) {
			continue;
		}
		_managedTextures[i] = NULL;
		SDL_DestroyTexture(loadedTexture);
	}
	switch (_rendererKind) {
	case Renderer::SDL_Renderer:
		if (_renderer != NULL) {
			SDL_DestroyRenderer(_renderer);
			_renderer = NULL;
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
	return SdlEngine::ErrorCode::Success;
}

SdlEngine::ErrorCode SdlEngine::Init(std::string windowName, Renderer renderer)
{
	if (_initialized) {
		// TODO change windowname, or change renderer
		ErrorMessage = string_format("Re-initialization is unsupported");
		return SdlEngine::ErrorCode::NotImplemented;
	}
	_running = false;
	_rendererKind = Renderer::None;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		ErrorMessage = string_format("SDL could not initialize! SDL_Error: %s", SDL_GetError());
		return SdlEngine::ErrorCode::InitializationFailure;
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
		ErrorMessage = string_format("Window could not be created! SDL_Error: %s", SDL_GetError());
		return SdlEngine::ErrorCode::WindowCreationFailure;
	}

	_rendererKind = renderer;
	SdlEngine::ErrorCode errorCode = ErrorCode::NotImplemented;
	switch (renderer) {
	case Renderer::SDL_Surface:
		errorCode = InitSDL_Surface();
		break;
	case Renderer::SDL_Renderer:
		errorCode = InitSDL_Renderer();
		break;
	default:
		ErrorMessage = string_format("Renderer %d not implemented!", renderer);
		errorCode = ErrorCode::NotImplemented;
		break;
	}
	// TODO create a separate image module, and be able to query if these image types can be loaded: IMG_INIT_JPG, IMG_INIT_PNG, IMG_INIT_TIF, IMG_INIT_WEBP, IMG_INIT_JXL, IMG_INIT_AVIF
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		ErrorMessage = string_format("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		return ErrorCode::CapabilityLoadFailed;
	}
	if (TTF_Init() == -1) {
		ErrorMessage = string_format("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
		return ErrorCode::CapabilityLoadFailed;
	}
	if (errorCode != ErrorCode::Success) {
		return errorCode;
	}
	_running = true;
	return ErrorCode::Success;
}

bool SdlEngine::IsRunning() {
	return _running;
}

SDL_Surface* SdlEngine::GetScreenSurface() { return this->_screenSurface; }

SDL_Renderer* SdlEngine::GetRenderer() { return this->_renderer; }

TTF_Font* SdlEngine::GetFont() { return this->_currentFont; }

SdlEngine::ErrorCode SdlEngine::SetFont(std::string fontName, int size) {
	std::string savedName = string_format("%s%d", fontName.c_str(), size);
	auto iter = _fonts.find(savedName);
	if (iter == _fonts.end()) {
		std::string path = string_format("font/%s.ttf", fontName.c_str());
		TTF_Font* font = TTF_OpenFont(path.c_str(), size);
		if (font == NULL) {
			ErrorMessage = string_format("could not load %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
			SdlEngine::ErrorCode::MissingResource;
		}
		_fonts[savedName] = font;
		_currentFont = font;
	} else {
		_currentFont = iter->second;
	}
	return SdlEngine::ErrorCode::Success;
}

void SdlEngine::ClearGraphics() {
	switch (_rendererKind) {
	case Renderer::SDL_Surface:
		SDL_FillRect(_screenSurface, NULL, SDL_MapRGBA(_screenSurface->format, 0xFF, 0xFF, 0xFF, 0x00));
		break;
	case Renderer::SDL_Renderer:
		SDL_RenderClear(_renderer);
		break;
	}
}

void SdlEngine::Render() {
	SDL_Renderer* g = GetRenderer();
	for (int b = 0; b < _drawables.size(); ++b) {
		_drawables[b]->Draw(g);
	}
	switch (_rendererKind) {
	case Renderer::SDL_Surface:
		SDL_UpdateWindowSurface(_window);
		break;
	case Renderer::SDL_Renderer:
		SDL_RenderPresent(_renderer);
		break;
	}
}

void SdlEngine::ProcessDelegates(std::vector<SdlEventProcessor*> eventProcessors, const SDL_Event& e) {
	for (int i = 0; i < eventProcessors.size(); ++i) {
		eventProcessors[i]->ProcessInput(e);
	}
}

void SdlEngine::ProcessDelegates(EventDelegateListMap& delegates, int id, const SDL_Event& e) {
	EventDelegateListMap::iterator found = delegates.find(id);
	if (found != delegates.end()) {
		ProcessDelegates(found->second, e);
	}
}

void SdlEngine::ProcessDelegates(SdlEngine::EventDelegateKeyedList& delegates, const SDL_Event& e) {
	for (auto it = delegates.begin(); it != delegates.end(); it++) {
		it->second(e);
	}
}

void SdlEngine::ProcessDelegates(SdlEngine::EventKeyedList& delegates){
	for (auto it = delegates.begin(); it != delegates.end(); it++) {
		it->second();
	}
}

void SdlEngine::RegisterProcessor(SdlEventProcessor* eventProcessor) {
	_eventProcessors.push_back(eventProcessor);
}

void SdlEngine::UnregisterProcessor(SdlEventProcessor* eventProcessor) {
	auto found = std::find(_eventProcessors.begin(), _eventProcessors.end(), eventProcessor);
	if (found == _eventProcessors.end()) { return; }
	_eventProcessors.erase(found);
}

void SdlEngine::RegisterDrawable(SdlDrawable* drawable) {
	_drawables.push_back(drawable);
}

void SdlEngine::UnregisterDrawable(SdlDrawable* drawable) {
	auto found = std::find(_drawables.begin(), _drawables.end(), drawable);
	if (found == _drawables.end()) { return; }
	_drawables.erase(found);
}

void SdlEngine::RegisterUpdatable(SdlUpdatable* updatable) {
	_updatable.push_back(updatable);
}

void SdlEngine::UnregisterUpdatable(SdlUpdatable* updatable) {
	auto found = std::find(_updatable.begin(), _updatable.end(), updatable);
	if (found == _updatable.end()) { return; }
	_updatable.erase(found);
}

void SdlEngine::ProcessEvent(const SDL_Event& e)
{
	switch (e.type) {
	case SDL_QUIT:
		_running = false;
		break;
	case SDL_KEYDOWN:
		SetPressed(e.key.keysym.sym, true);
		ProcessDelegates(_keyBindDown, e.key.keysym.sym, e);
		break;
	case SDL_KEYUP:
		SetPressed(e.key.keysym.sym, false);
		ProcessDelegates(_keyBindUp, e.key.keysym.sym, e);
		break;
	case SDL_MOUSEMOTION:
		MousePosition.x = e.motion.x;
		MousePosition.y = e.motion.y;
		//printf("mousemotion x%d, y%d, type%d, dx%d, dy%d\n",
		//	e.motion.x, e.motion.y, e.motion.type, e.motion.xrel, e.motion.yrel);
		break;
	case SDL_MOUSEBUTTONUP:
		MousePosition.x = e.button.x;
		MousePosition.y = e.button.y;
		SetPressed(SDL_MOUSEMOTION | e.button.button, false);
		//printf("####################### UP BTN%d   %d\n", e.button.button, e.button.state);
		ProcessDelegates(_mouseBindUp, e.button.button, e);
		//printf("mousemotion x%d, y%d, type%d, clicks%d, which%d, state%d, button%d\n",
		//	e.button.x, e.button.y, e.button.type, e.button.clicks, e.button.which, e.button.state, e.button.button);
		break;
	case SDL_MOUSEBUTTONDOWN:
		MousePosition.x = e.button.x;
		MousePosition.y = e.button.y;
		SetPressed(SDL_MOUSEMOTION | e.button.button, true);
		//printf("####################### DN BTN%d   %d\n", e.button.button, e.button.state);
		ProcessDelegates(_mouseBindDown, e.button.button, e);
		//printf("mousemotion x%d, y%d, type%d, clicks%d, which%d, state%d, button%d\n",
		//	e.button.x, e.button.y, e.button.type, e.button.clicks, e.button.which, e.button.state, e.button.button);
		break;
	}
	ProcessDelegates(this->_eventProcessors, e);
}

void SdlEngine::ServiceQueue() {
	auto temp = _todoNow;
	_todoNow = _todo;
	_todo = temp;
	_todo->clear();
	for (int i = 0; i < _todoNow->size(); ++i) {
		//printf("%s\n", (*_todoNow)[i].src.c_str());
		(*_todoNow)[i].action();
	}
	_todoNow->clear();
}

void SdlEngine::Queue(SdlEngine::TriggeredEvent action, std::string src) {
	_todo->push_back({ src, action });
}


void SdlEngine::ProcessInput() {
	SDL_Event e;
	std::map<int, EventDelegateKeyedList>::iterator found;
	while (SDL_PollEvent(&e)) {
		ProcessEvent(e);
	}
}

void SdlEngine::Update() {
	for (int b = 0; b < _updatable.size(); ++b) {
		_updatable[b]->Update();
	}
	ServiceQueue();
}

SdlEngine::ErrorCode SdlEngine::InitSDL_Surface() {
	_screenSurface = SDL_GetWindowSurface(_window);
	if (_screenSurface == NULL)
	{
		ErrorMessage = string_format("Window surface could not be retrieved! SDL_Error: %s", SDL_GetError());
		return SdlEngine::ErrorCode::WindowCreationFailure;
	}
	SDL_FillRect(_screenSurface, NULL, SDL_MapRGB(_screenSurface->format, 0xFF, 0xFF, 0xFF));
	return SdlEngine::ErrorCode::Success;
}

SdlEngine::ErrorCode SdlEngine::InitSDL_Renderer() {
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	if (_renderer == NULL)
	{
		ErrorMessage = string_format("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return SdlEngine::ErrorCode::WindowCreationFailure;
	}
	SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	return SdlEngine::ErrorCode::Success;
}

SdlEngine::ErrorCode SdlEngine::SetPressed(int sdlk, bool pressed) {
	// TODO do some performance analysis. is int faster than longlong and unsignedchar?
	int* field = NULL;
	if (sdlk >= 0 && sdlk < 256) {
		field = this->_isPressedKeyMask;
	} else if ((sdlk & SDL_MOUSEMOTION) != 0) {
		sdlk -= SDL_MOUSEMOTION;
		field = this->_isMousePressed;
		//printf("~~%d\n", sdlk);
	} else if ((sdlk & SDLK_SCANCODE_MASK) != 0) {
		sdlk -= SDLK_SCANCODE_MASK;
		field = this->_isPressedKeyMaskScancode;
#ifndef NDEBUG
		if (sdlk < 0 || sdlk >= SDL_NUM_SCANCODES) {
			ErrorMessage = string_format("Unknown scancode %d", sdlk);
			return ErrorCode::InputError;
		}
	} else {
		ErrorMessage = string_format("Unknown keycode %d", sdlk);
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

SdlEngine::ErrorCode SdlEngine::IsPressed(int sdlk, bool& out_pressed) {
	int* field = NULL;
	if (sdlk >= 0 && sdlk < 256) {
		field = this->_isPressedKeyMask;
	} else if ((sdlk & SDL_MOUSEMOTION) != 0) {
		sdlk -= SDL_MOUSEMOTION;
		field = this->_isMousePressed;
	} else if ((sdlk & SDLK_SCANCODE_MASK) != 0) {
		sdlk -= SDLK_SCANCODE_MASK;
		field = this->_isPressedKeyMaskScancode;
#ifndef NDEBUG
		if (sdlk < 0 || sdlk >= SDL_NUM_SCANCODES) {
			ErrorMessage = string_format("Unknown scancode %d", sdlk);
			return ErrorCode::InputError;
		}
	} else {
		ErrorMessage = string_format("Unknown keycode %d", sdlk);
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

SdlEngine::ErrorCode SdlEngine::LoadSdlSurfaceBasic(std::string path, SDL_Surface*& out_surface) {
	std::string lowercasePath = str_tolower(path);
	if (ends_with(lowercasePath, "bmp")) {
		out_surface = SDL_LoadBMP(path.c_str());
	} else if (ends_with(lowercasePath, "png")) {
		out_surface = IMG_Load(path.c_str());
	} else {
		ErrorMessage = string_format("Unable to load image format %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		return ErrorCode::UnsupportedFormat;
	}
	if (out_surface == NULL)
	{
		ErrorMessage = string_format("Failed to load image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		return ErrorCode::Failure;
	}
	return ErrorCode::Success;
}

SdlEngine::ErrorCode SdlEngine::LoadSdlTextBasic(std::string text, SDL_Surface*& out_surface) {
	SDL_Color textColor;
	SDL_GetRenderDrawColor(_renderer, &textColor.r, &textColor.g, &textColor.b, &textColor.a);
	out_surface = TTF_RenderText_Solid(_currentFont, text.c_str(), textColor);
	if (out_surface == NULL)
	{
		ErrorMessage = string_format("Failed to create TTF %s! SDL Error: %s\n", text.c_str(), SDL_GetError());
		return ErrorCode::Failure;
	}
	return ErrorCode::Success;
}

SdlEngine::ErrorCode SdlEngine::LoadSdlSurface(std::string path, SDL_Surface*& out_surface) {
	SDL_Surface* loadedSurface = NULL;
	ErrorCode err = LoadSdlSurfaceBasic(path, loadedSurface);
	if (err != ErrorCode::Success) { return err; }
	// optimize surface for blitting
	out_surface = SDL_ConvertSurface(loadedSurface, _screenSurface->format, 0);
	if (out_surface == NULL)
	{
		ErrorMessage = string_format("Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		return ErrorCode::Failure;
	}
	SDL_FreeSurface(loadedSurface);
	_managedSurfaces.push_back(out_surface);
	return ErrorCode::Success;
}

SdlEngine::ErrorCode SdlEngine::LoadSdlTexture(std::string path, SDL_Texture*& out_texture) {
	SDL_Surface* loadedSurface = NULL;
	ErrorCode err = LoadSdlSurfaceBasic(path, loadedSurface);
	if (err != ErrorCode::Success) { return err; }
	err = LoadSdlTexture(loadedSurface, out_texture);
	if (err != ErrorCode::Success) {
		ErrorMessage = string_format("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		return err;
	}
	////printf("have img %x\n", loadedSurface);
	//out_texture = SDL_CreateTextureFromSurface(_renderer, loadedSurface);
	//if (out_texture == NULL) {
	//	ErrorMessage = string_format("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
	//	return ErrorCode::Failure;
	//}
	//_managedTextures.push_back(out_texture);
	SDL_FreeSurface(loadedSurface);
	return ErrorCode::Success;
}

SdlEngine::ErrorCode SdlEngine::LoadSdlTexture(SDL_Surface* loadedSurface, SDL_Texture*& out_texture) {
	out_texture = SDL_CreateTextureFromSurface(_renderer, loadedSurface);
	if (out_texture == NULL) {
		ErrorMessage = string_format("Unable to create texture from SDL_Surface! SDL Error: %s\n", SDL_GetError());
		return ErrorCode::Failure;
	}
	_managedTextures.push_back((size_t)out_texture);
	return ErrorCode::Success;
}

void SdlEngine::ReleaseSdlTexture(SDL_Texture* texture) {
	auto end = _managedTextures.end();
	_managedTextures.erase(std::remove(_managedTextures.begin(), end, (size_t)texture), end);
}

Coord SdlEngine::GetTextureSize(SDL_Texture* texture) {
	Coord size;
	SDL_QueryTexture(texture, NULL, NULL, &size.x, &size.y);
	return size;
}

SdlEngine::ErrorCode SdlEngine::CreateText(std::string text, SDL_Texture*& out_texture) {
	SDL_Surface* loadedSurface = NULL;
	ErrorCode err = LoadSdlTextBasic(text, loadedSurface);
	if (err != ErrorCode::Success) { return err; }
	out_texture = SDL_CreateTextureFromSurface(_renderer, loadedSurface);
	if (out_texture == NULL)
	{
		ErrorMessage = string_format("Unable to create TTF texture for '%s'! SDL Error: %s\n", text.c_str(), SDL_GetError());
		printf("%s", ErrorMessage.c_str());
		return ErrorCode::Failure;
	}
	SDL_FreeSurface(loadedSurface);
	_managedTextures.push_back((size_t)out_texture);
	return ErrorCode::Success;
}

void AddDelegateToList(SdlEngine::EventDelegateListMap& map, int button, size_t owner, SdlEngine::EventDelegate eventDelegate) {
	SdlEngine::EventDelegateKeyedList* list = NULL;
	auto found = map.find(button);
	if (found != map.end()) {
		list = &found->second;
	} else {
		map[button] = SdlEngine::EventDelegateKeyedList();
		auto found = map.find(button);
		list = &found->second;
	}
	(*list)[owner] = eventDelegate;
}

void RemoveDelegateFromList(SdlEngine::EventDelegateListMap& map, int button, size_t owner) {
	auto found = map.find(button);
	if (found == map.end()) { return; }
	found->second.erase(owner);
}

void SdlEngine::RegisterMouseDown(int button, size_t owner, SdlEngine::EventDelegate eventDelegate) {
	button &= ~SDL_MOUSEMOTION;
	AddDelegateToList(_mouseBindDown, button, owner, eventDelegate);
}

void SdlEngine::RegisterMouseUp(int button, size_t owner, SdlEngine::EventDelegate eventDelegate) {
	button &= ~SDL_MOUSEMOTION;
	AddDelegateToList(_mouseBindUp, button, owner, eventDelegate);
}

void SdlEngine::RegisterKeyDown(int button, size_t owner, SdlEngine::EventDelegate eventDelegate) {
	AddDelegateToList(_keyBindDown, button, owner, eventDelegate);
}

void SdlEngine::RegisterKeyUp(int button, size_t owner, SdlEngine::EventDelegate eventDelegate) {
	AddDelegateToList(_keyBindUp, button, owner, eventDelegate);
}

void SdlEngine::UnregisterMouseDown(int button, size_t owner) {
	RemoveDelegateFromList(_mouseBindDown, button, owner);
}

void SdlEngine::UnregisterMouseUp(int button, size_t owner) {
	RemoveDelegateFromList(_mouseBindUp, button, owner);
}

void SdlEngine::UnregisterKeyDown(int button, size_t owner) {
	RemoveDelegateFromList(_keyBindDown, button, owner);
}

void SdlEngine::UnregisterKeyUp(int button, size_t owner) {
	RemoveDelegateFromList(_keyBindUp, button, owner);
}

#undef nameof
#undef CLEAR_ARRAY
