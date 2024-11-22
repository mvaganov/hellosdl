#include "vyengine.h"
#include "stringstuff.h"
#include <map>
#include <cctype>
#include <SDL_image.h>
#include <algorithm>
#include "helper.h"

#define CLEAR_ARRAY(arr) memset(arr, 0, sizeof(arr))

VyEngine * VyEngine::_instance = NULL;

void VyEngine::FailFast() {
	if (ErrorMessage != "") {
		printf(ErrorMessage.c_str());
		exit((int)VyEngine::ErrorCode::Failure);
	}
}

VyEngine::VyEngine(int width, int height) : MouseClickState(0), _window(NULL), _screenSurface(NULL), _width(width), _height(height),
_rendererKind(Renderer::None), _running(false), _initialized(false), _currentFont(NULL),
_isPressedKeyMask(), _isMousePressed(), _isPressedKeyMaskScancode(),
_managedSurfaces(), _fonts(), _eventProcessors(), _todo(NULL), _todoNow(NULL), _currentFontSize(0) {
	ErrorMessage = "";
	if (_instance == NULL) {
		_instance = this;
	} else {
		printf("duplicate VyEngine being created? already have at %016llux", (size_t)_instance);
	}
	CLEAR_ARRAY(_isPressedKeyMask);
	CLEAR_ARRAY(_isPressedKeyMaskScancode);
	CLEAR_ARRAY(_isMousePressed);
	_todo = DelegateListPtr(new std::vector<DelegateNextFrame>());
	_todoNow = DelegateListPtr(new std::vector<DelegateNextFrame>());
}

VyEngine::~VyEngine() {
	Release();
}

VyEngine::ErrorCode VyEngine::Release() {
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
	return VyEngine::ErrorCode::Success;
}

VyEngine::ErrorCode VyEngine::Init(std::string windowName, Renderer renderer)
{
	if (_initialized) {
		ErrorMessage = string_format("Re-initialization is unsupported");
		return VyEngine::ErrorCode::NotImplemented;
	}
	_running = false;
	_rendererKind = Renderer::None;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		ErrorMessage = string_format("SDL could not initialize! SDL_Error: %s", SDL_GetError());
		return VyEngine::ErrorCode::InitializationFailure;
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
		return VyEngine::ErrorCode::WindowCreationFailure;
	}

	_rendererKind = renderer;
	VyEngine::ErrorCode errorCode = ErrorCode::NotImplemented;
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

bool VyEngine::IsRunning() {
	return _running;
}

SDL_Surface* VyEngine::GetScreenSurface() { return this->_screenSurface; }

SDL_Renderer* VyEngine::GetRenderer() { return this->_renderer; }

TTF_Font* VyEngine::GetFont() { return this->_currentFont; }

std::string VyEngine::GetFontName() { return this->_currentFontName; }

std::string VyEngine::GetFontId() { return this->_currentFontId; }

int VyEngine::GetFontSize() { return this->_currentFontSize; }

VyEngine::ErrorCode VyEngine::SetFont(std::string fontName, int size) {
	std::string savedName = string_format("%s%d", fontName.c_str(), size);
	auto iter = _fonts.find(savedName);
	if (iter == _fonts.end()) {
		std::string path = string_format("font/%s.ttf", fontName.c_str());
		TTF_Font* font = TTF_OpenFont(path.c_str(), size);
		if (font == NULL) {
			ErrorMessage = string_format("could not load %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
			VyEngine::ErrorCode::MissingResource;
		}
		_fonts[savedName] = font;
		_currentFont = font;
	} else {
		_currentFont = iter->second;
	}
	_currentFontId = savedName;
	_currentFontName = fontName;
	_currentFontSize = size;
	printf("current font: %s\n", _currentFontId.c_str());
	return VyEngine::ErrorCode::Success;
}

void VyEngine::ClearGraphics() {
	switch (_rendererKind) {
	case Renderer::SDL_Surface:
		SDL_FillRect(_screenSurface, NULL, SDL_MapRGBA(_screenSurface->format, 0xFF, 0xFF, 0xFF, 0x00));
		break;
	case Renderer::SDL_Renderer:
		SDL_RenderClear(_renderer);
		break;
	}
}

void VyEngine::Render() {
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

void VyEngine::ProcessDelegates(std::vector<VyEventProcessor*> eventProcessors, const SDL_Event& e) {
	for (int i = 0; i < eventProcessors.size(); ++i) {
		eventProcessors[i]->HandleEvent(e);
	}
}

void VyEngine::ProcessDelegates(EventDelegateListMap& delegates, int id, const SDL_Event& e) {
	EventDelegateListMap::iterator found = delegates.find(id);
	if (found != delegates.end()) {
		ProcessDelegates(found->second, e);
	}
}

void VyEngine::ProcessDelegates(VyEngine::EventDelegateKeyedList& delegates, const SDL_Event& e) {
	for (auto it = delegates.begin(); it != delegates.end(); it++) {
		it->second(e);
	}
}

void VyEngine::ProcessDelegates(VyEngine::EventKeyedList& delegates){
	for (auto it = delegates.begin(); it != delegates.end(); it++) {
		it->second();
	}
}

void VyEngine::RegisterProcessor(VyEventProcessor* eventProcessor) {
	_eventProcessors.push_back(eventProcessor);
}

void VyEngine::UnregisterProcessor(VyEventProcessor* eventProcessor) {
	auto found = std::find(_eventProcessors.begin(), _eventProcessors.end(), eventProcessor);
	if (found == _eventProcessors.end()) { return; }
	_eventProcessors.erase(found);
}

void VyEngine::RegisterDrawable(SdlDrawable* drawable) {
	_drawables.push_back(drawable);
}

void VyEngine::UnregisterDrawable(SdlDrawable* drawable) {
	auto found = std::find(_drawables.begin(), _drawables.end(), drawable);
	if (found == _drawables.end()) { return; }
	_drawables.erase(found);
}

void VyEngine::RegisterUpdatable(SdlUpdatable* updatable) {
	_updatable.push_back(updatable);
}

void VyEngine::UnregisterUpdatable(SdlUpdatable* updatable) {
	auto found = std::find(_updatable.begin(), _updatable.end(), updatable);
	if (found == _updatable.end()) { return; }
	_updatable.erase(found);
}

void VyEngine::ProcessEvent(const SDL_Event& e)
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

void VyEngine::ServiceQueue() {
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

void VyEngine::Queue(VyEngine::TriggeredEvent action, std::string src) {
	_todo->push_back({ src, action });
}


void VyEngine::ProcessInput() {
	SDL_Event e;
	std::map<int, EventDelegateKeyedList>::iterator found;
	while (SDL_PollEvent(&e)) {
		ProcessEvent(e);
	}
}

void VyEngine::Update() {
	for (int b = 0; b < _updatable.size(); ++b) {
		_updatable[b]->Update();
	}
	ServiceQueue();
}

VyEngine::ErrorCode VyEngine::InitSDL_Surface() {
	_screenSurface = SDL_GetWindowSurface(_window);
	if (_screenSurface == NULL)
	{
		ErrorMessage = string_format("Window surface could not be retrieved! SDL_Error: %s", SDL_GetError());
		return VyEngine::ErrorCode::WindowCreationFailure;
	}
	SDL_FillRect(_screenSurface, NULL, SDL_MapRGB(_screenSurface->format, 0xFF, 0xFF, 0xFF));
	return VyEngine::ErrorCode::Success;
}

VyEngine::ErrorCode VyEngine::InitSDL_Renderer() {
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	if (_renderer == NULL)
	{
		ErrorMessage = string_format("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return VyEngine::ErrorCode::WindowCreationFailure;
	}
	SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	return VyEngine::ErrorCode::Success;
}

VyEngine::ErrorCode VyEngine::SetPressed(int sdlk, bool pressed) {
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

VyEngine::ErrorCode VyEngine::IsPressed(int sdlk, bool& out_pressed) {
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

VyEngine::ErrorCode VyEngine::LoadSdlSurfaceBasic(std::string path, SDL_Surface*& out_surface) {
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

VyEngine::ErrorCode VyEngine::LoadSdlTextBasic(std::string text, SDL_Surface*& out_surface) {
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

VyEngine::ErrorCode VyEngine::LoadSdlSurface(std::string path, SDL_Surface*& out_surface) {
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

VyEngine::ErrorCode VyEngine::LoadSdlTexture(std::string path, SDL_Texture*& out_texture) {
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

VyEngine::ErrorCode VyEngine::LoadSdlTexture(SDL_Surface* loadedSurface, SDL_Texture*& out_texture) {
	out_texture = SDL_CreateTextureFromSurface(_renderer, loadedSurface);
	if (out_texture == NULL) {
		ErrorMessage = string_format("Unable to create texture from SDL_Surface! SDL Error: %s\n", SDL_GetError());
		return ErrorCode::Failure;
	}
	_managedTextures.push_back((size_t)out_texture);
	return ErrorCode::Success;
}

void VyEngine::ReleaseSdlTexture(SDL_Texture* texture) {
	auto end = _managedTextures.end();
	_managedTextures.erase(std::remove(_managedTextures.begin(), end, (size_t)texture), end);
}

Coord VyEngine::GetTextureSize(SDL_Texture* texture) {
	Coord size;
	SDL_QueryTexture(texture, NULL, NULL, &size.x, &size.y);
	return size;
}

VyEngine::ErrorCode VyEngine::CreateText(std::string text, SDL_Texture*& out_texture) {
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

void AddDelegateToList(VyEngine::EventDelegateListMap& map, int button, size_t owner, VyEngine::EventDelegate eventDelegate) {
	VyEngine::EventDelegateKeyedList* list = NULL;
	auto found = map.find(button);
	if (found != map.end()) {
		list = &found->second;
	} else {
		map[button] = VyEngine::EventDelegateKeyedList();
		auto found = map.find(button);
		list = &found->second;
	}
	(*list)[owner] = eventDelegate;
}

void RemoveDelegateFromList(VyEngine::EventDelegateListMap& map, int button, size_t owner) {
	auto found = map.find(button);
	if (found == map.end()) { return; }
	found->second.erase(owner);
}

void VyEngine::RegisterMouseDown(int button, size_t owner, VyEngine::EventDelegate eventDelegate) {
	button &= ~SDL_MOUSEMOTION;
	AddDelegateToList(_mouseBindDown, button, owner, eventDelegate);
}

void VyEngine::RegisterMouseUp(int button, size_t owner, VyEngine::EventDelegate eventDelegate) {
	button &= ~SDL_MOUSEMOTION;
	AddDelegateToList(_mouseBindUp, button, owner, eventDelegate);
}

void VyEngine::RegisterKeyDown(int button, size_t owner, VyEngine::EventDelegate eventDelegate) {
	AddDelegateToList(_keyBindDown, button, owner, eventDelegate);
}

void VyEngine::RegisterKeyUp(int button, size_t owner, VyEngine::EventDelegate eventDelegate) {
	AddDelegateToList(_keyBindUp, button, owner, eventDelegate);
}

void VyEngine::UnregisterMouseDown(int button, size_t owner) {
	RemoveDelegateFromList(_mouseBindDown, button, owner);
}

void VyEngine::UnregisterMouseUp(int button, size_t owner) {
	RemoveDelegateFromList(_mouseBindUp, button, owner);
}

void VyEngine::UnregisterKeyDown(int button, size_t owner) {
	RemoveDelegateFromList(_keyBindDown, button, owner);
}

void VyEngine::UnregisterKeyUp(int button, size_t owner) {
	RemoveDelegateFromList(_keyBindUp, button, owner);
}

#undef CLEAR_ARRAY
