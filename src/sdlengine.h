#pragma once

#include <SDL.h>
#include <string>
#include <map>
#include <functional>
#include <vector>

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
	typedef std::vector<SdlEventDelegate> SdlEventDelegateList;
private:

	static System* _instance;
	std::string _errorMessage;
	SDL_Window* _window = NULL;
	SDL_Surface* _screenSurface = NULL;
	SDL_Renderer* _gRenderer = NULL;
	int _width, _height;
	Renderer _rendererKind;
	std::map<int, SdlEventDelegateList> _keyBindDown;
	std::map<int, SdlEventDelegateList> _keyBindUp;
	int _isPressedKeyMask[8];
	int _isPressedKeyMaskScancode[16];
	bool _running;
	bool _initialized;
	std::vector<SDL_Surface*> _managedSurfaces;
	std::vector<SDL_Texture*> _managedTextures;
public:
	System(int width, int height);
	~System();
	void FailFast();
	ErrorCode Init(std::string windowName, Renderer renderer);
	ErrorCode Release();
	bool IsRunning();
	SDL_Surface* GetScreenSurface();
	SDL_Renderer* GetRenderer();
	void ClearGraphics();
	void Render();
	void ProcessInput();
	System::ErrorCode IsPressed(int sdlk, bool& out_pressed);
	System::ErrorCode LoadSdlSurfaceBasic(std::string path, SDL_Surface*& out_surface);
	System::ErrorCode LoadSdlSurface(std::string path, SDL_Surface*& out_surface);
	System::ErrorCode LoadSdlTexture(std::string path, SDL_Texture*& out_texture);
	/// <summary>
	/// this is set by <see cref="System::ProcessInput"/>
	/// </summary>
	/// <param name="sdlk"></param>
	/// <param name="pressed"></param>
	/// <returns></returns>
	System::ErrorCode SetPressed(int sdlk, bool pressed);
private:
	System::ErrorCode InitSDL_Surface();
	System::ErrorCode InitSDL_Renderer();
};

