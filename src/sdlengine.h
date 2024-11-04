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

class System
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
	};
	enum class Renderer { None = 0, SDL_Surface = 1, SDL_Renderer = 2 };

	typedef std::function<void(SDL_Event)> SdlEventDelegate;
	typedef std::map<size_t, SdlEventDelegate> SdlEventDelegateList;
	typedef std::map<int, SdlEventDelegateList> SdlEventDelegateListMap;
private:
	// TODO rename _currentFount
	TTF_Font* gFont;
	static System* _instance;
	SDL_Window* _window = NULL;
	SDL_Surface* _screenSurface = NULL;
	SDL_Renderer* _gRenderer = NULL;
	int _width, _height;
	Renderer _rendererKind;
	SdlEventDelegateListMap _keyBindDown;
	SdlEventDelegateListMap _keyBindUp;
	SdlEventDelegateListMap _mouseBindDown;
	SdlEventDelegateListMap _mouseBindUp;
	int _isPressedKeyMask[8];
	int _isPressedKeyMaskScancode[16];
	int _isMousePressed[1];
	bool _running;
	bool _initialized;
	std::vector<SDL_Surface*> _managedSurfaces;
	std::vector<size_t> _managedTextures;
public:
	// TODO rename ErrorMessage
	std::string _errorMessage;
	Coord MousePosition;
	int MouseClickState;
	System(int width, int height);
	~System();
	void FailFast();
	ErrorCode Init(std::string windowName, Renderer renderer);
	ErrorCode Release();
	bool IsRunning();
	SDL_Surface* GetScreenSurface();
	SDL_Renderer* GetRenderer();
	TTF_Font* GetFont();
	void ClearGraphics();
	void Render();
	void ProcessInput();
	System::ErrorCode IsPressed(int sdlk, bool& out_pressed);
	System::ErrorCode LoadSdlSurfaceBasic(std::string path, SDL_Surface*& out_surface);
	System::ErrorCode LoadSdlTextBasic(std::string text, SDL_Surface*& out_surface);
	System::ErrorCode LoadSdlSurface(std::string path, SDL_Surface*& out_surface);
	System::ErrorCode LoadSdlTexture(std::string path, SDL_Texture*& out_texture);
	System::ErrorCode LoadSdlTexture(SDL_Surface* loadedSurface, SDL_Texture*& out_texture);
	System::ErrorCode LoadTextTexture(std::string text, SDL_Texture*& out_texture);
	void ReleaseSdlTexture(SDL_Texture* texture);
	Coord GetTextureSize(SDL_Texture* texture);
	/// <summary>
	/// this is set by <see cref="System::ProcessInput"/>
	/// </summary>
	/// <param name="sdlk"></param>
	/// <param name="pressed"></param>
	/// <returns></returns>
	System::ErrorCode SetPressed(int sdlk, bool pressed);
	std::function<void(int, int)> OnMouseMove;
	void RegisterMouseDown(int button, size_t owner, SdlEventDelegate eventDeletage);
	void RegisterMouseUp(int button, size_t owner, SdlEventDelegate eventDeletage);
	void UnregisterMouseDown(int button, size_t owner);
	void UnregisterMouseUp(int button, size_t owner);
private:
	System::ErrorCode InitSDL_Surface();
	System::ErrorCode InitSDL_Renderer();
};

