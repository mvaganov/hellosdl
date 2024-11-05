/*This source code copyrighted by Lazy Foo' Productions 2004-2024
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include "sdlengine.h"
#include <stdio.h>
#include <chrono>
#include "button.h"
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
	SDL_Texture* tex;
	SDL_Texture* word;
	sdl.LoadSdlTexture("img/helloworld.png", tex);
	sdl.FailFast();
	//SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	SDL_Renderer* g = sdl.GetRenderer();
	Rect fillRect(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	Button button({ 10, 20, 30, 40 }, &sdl);
	button.onPress = []() { printf("pressed!\n"); };
	button.onRelease = []() { printf("released!\n"); };
	SDL_SetRenderDrawColor(g, 0xFF008800);
	sdl.SetFont("arial", 24);
	System::ErrorCode wordErr = sdl.CreateText("these are words!", word);
	Rect wordArea(button.GetPosition(), sdl.GetTextureSize(word));
	while (sdl.IsRunning()) {
		TimePoint t0 = Clock::now();
		sdl.ClearGraphics();
		//SDL_Rect stretchRect;
		//stretchRect.x = 0;
		//stretchRect.y = 0;
		//stretchRect.w = SCREEN_WIDTH/8;
		//stretchRect.h = SCREEN_HEIGHT;
		//SDL_BlitScaled(img, NULL, sdl.GetScreenSurface(), &stretchRect);
		SDL_RenderCopy(g, tex, NULL, NULL);
		long color = fillRect.IsContains(sdl.MousePosition) ? 0x880000FF : 0xFF0000FF;
		SDL_SetRenderDrawColor(g, color);
		SDL_RenderFillRect(g, &fillRect);
		SDL_SetRenderDrawColor(g, 0xFF00FF00);
		SDL_RenderDrawRect(g, &fillRect);
		SDL_SetRenderDrawColor(g, 0x8800FF00);
		SDL_FillCircle(g, 200, 50, 50);
		SDL_DrawCircle(g, 200, 50, 52);
		button.Draw(g);
		SDL_RenderCopy(g, word, NULL, &wordArea);
		sdl.Render();
		sdl.ProcessInput();
		button.Update(&sdl);
		TimePoint t1 = Clock::now();
		HiResDuration time_span = std::chrono::duration_cast<HiResDuration>(t1 - t0);
		double secondsDuration = time_span.count();
		int maxFps = (int)(1 / secondsDuration);
		printf("%d fps   \r", maxFps);
		SDL_Delay(10);
	}
	sdl.Release();
	return 0;
}
