#pragma once
#include "rect.h"
#include "sdlengine.h"
#include "sdlhelper.h"
#include <functional>

class Button : public Rect {
public:
	typedef std::function<void()> ButtonEvent;
	enum class State { Normal, Hovered, Clicked, Selected, HoveredSelected };
	class Colors {
	public:
		long normal;
		long hover;
		long clicked;
		long selected;
		long hoveredSelected;
		Colors(long normal, long hover, long clicked, long selected, long hoveredSelected)
			: normal(normal), hover(hover), clicked(clicked), selected(selected), hoveredSelected(hoveredSelected)
		{}
		Colors() : Colors(0xff888888, 0xffbbbbbb, 0xff8888ff, 0xffaaaaaa, 0xffcccccc) {}
	};
	ButtonEvent onPress;
	ButtonEvent onRelease;
	Button::State state;
	Colors colors;
	int color;
	bool selected;
	bool held;
	System* engine;

	Button(System* engine) : Button({ 0, 0, 10, 10 }, engine) {}

	Button(SDL_Rect rect, System* engine) : Rect(rect), state(State::Normal), colors(),
	selected(false), held(false), color(0), engine(engine) {
		onPress = Nothing;
		onRelease = Nothing;
		engine->RegisterMouseDown(SDL_MOUSE_MAINCLICK, (size_t)this, [&](SDL_Event e) { HandlePress(e); });
		engine->RegisterMouseUp(SDL_MOUSE_MAINCLICK, (size_t)this, [&](SDL_Event e) { HandlePress(e); });
	}

	~Button() {
		engine->UnregisterMouseDown(SDL_MOUSE_MAINCLICK, (size_t)this);
		engine->UnregisterMouseUp(SDL_MOUSE_MAINCLICK, (size_t)this);
	}

	void HandlePress(SDL_Event e) {
		switch (e.button.state) {
		case SDL_PRESSED: {
			bool mouseOver = IsContains(engine->MousePosition);
			selected = mouseOver;
			if (mouseOver) {
				held = true;
				state = State::Clicked;
				onPress();
			}
			}break;
		case SDL_RELEASED:
			if (held) {
				held = false;
				onRelease();
			}
			break;
		}
	}

	static void Nothing() {}

	void UpdateColor() {
		switch (state) {
		case State::Normal: color = colors.normal; break;
		case State::Hovered: color = colors.hover; break;
		case State::Clicked: color = colors.clicked; break;
		case State::Selected: color = colors.selected; break;
		case State::HoveredSelected: color = colors.hoveredSelected; break;
		}
	}

	void Draw(SDL_Renderer* g) {
		long oldColor;
		SDL_GetRenderDrawColor(g, &oldColor);
		UpdateColor();
		SDL_SetRenderDrawColor(g, color);
		RenderFillRect(g);
		SDL_SetRenderDrawColor(g, oldColor);
	}

	void Update(System* engine) {
		bool mouseOver = IsContains(engine->MousePosition);
		//printf("%d, %d,     %d %d\n", engine->MousePosition.x, engine->MousePosition.y, mouseOver, isPressed);
		if (mouseOver) {
			bool isPressed = false;
			engine->IsPressed(SDL_MOUSE_MAINCLICK, isPressed);
			if (isPressed) {
				state = State::Clicked;
			} else if (selected) {
				state = State::HoveredSelected;
			} else {
				state = State::Hovered;
			}
		} else {
			if (selected) {
				state = State::Selected;
			} else {
				state = State::Normal;
			}
		}
		UpdateColor();
	}
};
