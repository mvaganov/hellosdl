/*This source code copyrighted by Lazy Foo' Productions 2004-2024
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include "sdlengine.h"
#include <stdio.h>
#include <chrono>
//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef std::chrono::high_resolution_clock Clock;
typedef Clock::time_point TimePoint;
typedef std::chrono::duration<double> HiResDuration;


int main( int argc, char* args[] )
{	System sdl(SCREEN_WIDTH, SCREEN_HEIGHT);
	System::ErrorCode err = sdl.Init("sdl", System::Renderer::SDL_Renderer);
	sdl.FailFast();
	//SDL_Surface* img; sdl.LoadSdlSurface("img/helloworld.bmp", img);
	SDL_Texture* tex; sdl.LoadSdlTexture("img/helloworld.png", tex);
	sdl.FailFast();
	SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	SDL_Renderer* g = sdl.GetRenderer();
	while (sdl.IsRunning()) {
		TimePoint t0 = Clock::now();
		sdl.ClearGraphics();
		SDL_Rect stretchRect;
		stretchRect.x = 0;
		stretchRect.y = 0;
		stretchRect.w = SCREEN_WIDTH/8;
		stretchRect.h = SCREEN_HEIGHT;
		//SDL_BlitScaled(img, NULL, sdl.GetScreenSurface(), &stretchRect);
		SDL_RenderCopy(g, tex, NULL, NULL);
		SDL_SetRenderDrawColor(g, 0xFF, 0x00, 0x00, 0xFF);
		SDL_RenderFillRect(g, &fillRect);
		SDL_SetRenderDrawColor(g, 0x00, 0xFF, 0x00, 0xFF);
		SDL_RenderDrawRect(g, &fillRect);
		sdl.Render();
		sdl.ProcessInput();
		TimePoint t1 = Clock::now();
		HiResDuration time_span = std::chrono::duration_cast<HiResDuration>(t1 - t0);
		double secondsDuration = time_span.count();
		int maxFps = (int)(1 / secondsDuration);
		printf("%d fps   \r", maxFps);
		SDL_Delay(100);
	}
	sdl.Release();
	return 0;
}
