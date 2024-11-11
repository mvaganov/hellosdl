#include "sdlengine.h"
#include <stdio.h>
#include <chrono>
#include "button.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef std::chrono::high_resolution_clock Clock;
typedef Clock::time_point TimePoint;
typedef std::chrono::duration<double> HiResDuration;


int main( int argc, char* args[] )
{	SdlEngine sdl(SCREEN_WIDTH, SCREEN_HEIGHT);
	sdl.RegisterKeyDown(27, (size_t)&sdl, [&sdl](SDL_Event e) {
		SDL_Event quitEvent;
		quitEvent.type = SDL_QUIT;
		sdl.ProcessEvent(quitEvent);
	});
	SdlEngine::ErrorCode err = sdl.Init("sdl", SdlEngine::Renderer::SDL_Renderer);
	sdl.FailFast();
	SDL_Texture* tex;
	SDL_Texture* word;
	sdl.LoadSdlTexture("img/helloworld.png", tex);
	sdl.FailFast();
	SDL_Renderer* g = sdl.GetRenderer();
	Rect fillRect(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	SDL_SetRenderDrawColor(g, 0xFF008800);
	sdl.SetFont("arial", 24);
	SdlEngine::ErrorCode wordErr = sdl.CreateText("these are words!", word);

	Button buttons[] = {
		Button({ 10, 20, 30, 40 }),
		Button({ 100, 20, 30, 40 }),
		Button({ 50, 30, 30, 40 }),
		Button({ 20, 200, 30, 40 }),
		Button({ 300, 300, 30, 40 }),
		Button({ 0, 0, 30, 40 }),
		Button({ 10, 150, 30, 40 }),
		Button({ 50, 150, 30, 40 }),
		Button({ 90, 150, 30, 40 }),
		Button({ 130, 150, 30, 40 }),
	};
	const int buttonsCount = sizeof(buttons) / sizeof(buttons[0]);
	std::vector<SelectableRect*> buttonRefs;
	buttonRefs.reserve(buttonsCount);
	for (int b = 0; b < buttonsCount; ++b) {
		Button* btn = &buttons[b];
		buttonRefs.push_back(btn);
		btn->onPress = [b]() { printf("pressed %d!\n", b); };
		btn->onRelease = [b]() { printf("released %d!\n", b); };
	}

	Rect wordArea(buttons[0].GetPosition(), sdl.GetTextureSize(word));

	SelectableRect::SetupNavigation(buttonRefs);

	while (sdl.IsRunning()) {
		TimePoint t0 = Clock::now();
		sdl.ClearGraphics();
		SDL_RenderCopy(g, tex, NULL, NULL);
		long color = fillRect.IsContains(sdl.MousePosition) ? 0x880000FF : 0xFF0000FF;
		SDL_SetRenderDrawColor(g, color);
		SDL_RenderFillRect(g, &fillRect);
		SDL_SetRenderDrawColor(g, 0xFF00FF00);
		SDL_RenderDrawRect(g, &fillRect);
		SDL_SetRenderDrawColor(g, 0x8800FF00);
		SDL_FillCircle(g, 200, 50, 50);
		SDL_DrawCircle(g, 200, 50, 52);
		SDL_RenderCopy(g, word, NULL, &wordArea);
		sdl.Render();
		sdl.FailFast();
		sdl.ProcessInput();
		sdl.FailFast();
		sdl.Update();
		sdl.FailFast();
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
