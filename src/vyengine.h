#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <map>
#include <functional>
#include <vector>
#include "coord.h"
#include "rect.h"
#include "sdlhelper.h"
#include "sdleventprocessor.h"

class VyEngine
{
public:
	enum class ErrorCode {
		Success = 0,
		Failure = -1,
		NotImplemented = -2,
		InitializationFailure = 1,
		WindowCreationFailure = 2,
		InputError = 3,
		CapabilityLoadFailed = 4,
		UnsupportedFormat = 5,
		MissingResource = 6,
	};
	enum class Renderer { None = 0, SDL_Surface = 1, SDL_Renderer = 2 };

	typedef std::function<void(SDL_Event)> EventDelegate;
	typedef std::map<size_t, EventDelegate> EventDelegateKeyedList;
	typedef std::map<int, EventDelegateKeyedList> EventDelegateListMap;
	typedef std::function<void()> TriggeredEvent;
	typedef std::map<size_t, TriggeredEvent> EventKeyedList;
	static VyEngine* GetInstance() { return _instance; }
private:
	TTF_Font* _currentFont;
	std::string _currentFontId;
	std::string _currentFontName;
	int _currentFontSize;
	static VyEngine* _instance;
	SDL_Window* _window = NULL;
	SDL_Surface* _screenSurface = NULL;
	SDL_Renderer* _renderer = NULL;
	int _width, _height;
	Renderer _rendererKind;
	EventDelegateListMap _keyBindDown;
	EventDelegateListMap _keyBindUp;
	EventDelegateListMap _mouseBindDown;
	EventDelegateListMap _mouseBindUp;
	int _isPressedKeyMask[8];
	int _isPressedKeyMaskScancode[16];
	int _isMousePressed[1];
	bool _running;
	bool _initialized;
	std::vector<SDL_Surface*> _managedSurfaces;
	std::vector<size_t> _managedTextures;
	std::map<std::string, TTF_Font*> _fonts;
	std::vector<SdlEventProcessor*> _eventProcessors;
	std::vector<SdlDrawable*> _drawables;
	std::vector<SdlUpdatable*> _updatable;
	class DelegateNextFrame {
	public:
		std::string src;
		TriggeredEvent action;
	};
	typedef std::vector<DelegateNextFrame> DelegateList;
	typedef std::shared_ptr<std::vector<DelegateNextFrame>> DelegateListPtr;
	DelegateListPtr _todo;
	DelegateListPtr _todoNow;
public:
	std::string ErrorMessage;
	Coord MousePosition;
	int MouseClickState;
	VyEngine(int width, int height);
	~VyEngine();
	void FailFast();
	ErrorCode Init(std::string windowName, Renderer renderer);
	ErrorCode Release();
	bool IsRunning();
	SDL_Surface* GetScreenSurface();
	SDL_Renderer* GetRenderer();
	TTF_Font* GetFont();
	std::string GetFontName();
	std::string GetFontId();
	int GetFontSize();
	VyEngine::ErrorCode SetFont(std::string fontName, int size);
	void ClearGraphics();
	void Render();
	void ProcessInput();
	void Update();
	VyEngine::ErrorCode IsPressed(int sdlk, bool& out_pressed);
	VyEngine::ErrorCode LoadSdlSurfaceBasic(std::string path, SDL_Surface*& out_surface);
	VyEngine::ErrorCode LoadSdlTextBasic(std::string text, SDL_Surface*& out_surface);
	VyEngine::ErrorCode LoadSdlSurface(std::string path, SDL_Surface*& out_surface);
	VyEngine::ErrorCode LoadSdlTexture(std::string path, SDL_Texture*& out_texture);
	VyEngine::ErrorCode LoadSdlTexture(SDL_Surface* loadedSurface, SDL_Texture*& out_texture);
	VyEngine::ErrorCode CreateText(std::string text, SDL_Texture*& out_texture);
	void ReleaseSdlTexture(SDL_Texture* texture);
	Coord GetTextureSize(SDL_Texture* texture);
	/// <summary>
	/// this is set by <see cref="VyEngine::ProcessInput"/>
	/// </summary>
	/// <param name="sdlk"></param>
	/// <param name="pressed"></param>
	/// <returns></returns>
	VyEngine::ErrorCode SetPressed(int sdlk, bool pressed);
	std::function<void(int, int)> OnMouseMove;
	void RegisterMouseDown(int button, size_t owner, EventDelegate eventDeletage);
	void RegisterMouseUp(int button, size_t owner, EventDelegate eventDeletage);
	void RegisterKeyDown(int key, size_t owner, EventDelegate eventDelegate);
	void RegisterKeyUp(int key, size_t owner, EventDelegate eventDelegate);
	void UnregisterMouseDown(int button, size_t owner);
	void UnregisterMouseUp(int button, size_t owner);
	void UnregisterKeyDown(int button, size_t owner);
	void UnregisterKeyUp(int button, size_t owner);
	void RegisterProcessor(SdlEventProcessor* eventProcessor);
	void UnregisterProcessor(SdlEventProcessor* eventProcessor);
	void RegisterDrawable(SdlDrawable* drawable);
	void UnregisterDrawable(SdlDrawable* drawable);
	void RegisterUpdatable(SdlUpdatable* updatable);
	void UnregisterUpdatable(SdlUpdatable* updatable);
	void ProcessEvent(const SDL_Event& e);
	void ServiceQueue();
	void Queue(TriggeredEvent action, std::string src);
	static void ProcessDelegates(VyEngine::EventDelegateListMap& delegates, int id, const SDL_Event& e);
	static void ProcessDelegates(VyEngine::EventDelegateKeyedList& delegates, const SDL_Event& e);
	static void ProcessDelegates(VyEngine::EventKeyedList& delegates);
	static void ProcessDelegates(std::vector<SdlEventProcessor*> eventProcessors, const SDL_Event& e);
private:
	VyEngine::ErrorCode InitSDL_Surface();
	VyEngine::ErrorCode InitSDL_Renderer();
};
