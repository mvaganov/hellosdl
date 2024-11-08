#pragma once
#include "rect.h"
#include "sdlengine.h"
#include "sdlhelper.h"
#include "selectablerect.h"
#include <functional>

class Button : public SelectableRect {
public:
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
	SdlEngine::TriggeredEvent onPress;
	SdlEngine::TriggeredEvent onRelease;
private:
	Button::State _buttonState;
	int color;
	bool held;
	SdlEngine* engine;
public:
	Colors Colors;

	Button(SdlEngine* engine) : Button({ 0, 0, 10, 10 }, engine) {}

	Button(SDL_Rect rect, SdlEngine* engine) : SelectableRect(rect), _buttonState(State::Normal), Colors(),
	held(false), color(0), engine(engine) {
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
			_selected = mouseOver;
			if (mouseOver) {
				held = true;
				_buttonState = State::Clicked;
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
		switch (_buttonState) {
		case State::Normal: color = Colors.normal; break;
		case State::Hovered: color = Colors.hover; break;
		case State::Clicked: color = Colors.clicked; break;
		case State::Selected: color = Colors.selected; break;
		case State::HoveredSelected: color = Colors.hoveredSelected; break;
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

	void Update(SdlEngine* engine) {
		bool mouseOver = IsContains(engine->MousePosition);
		//printf("%d, %d,     %d %d\n", engine->MousePosition.x, engine->MousePosition.y, mouseOver, isPressed);
		if (mouseOver) {
			bool isPressed = false;
			engine->IsPressed(SDL_MOUSE_MAINCLICK, isPressed);
			if (isPressed) {
				_buttonState = State::Clicked;
			} else if (_selected) {
				_buttonState = State::HoveredSelected;
			} else {
				_buttonState = State::Hovered;
			}
		} else {
			if (_selected) {
				_buttonState = State::Selected;
			} else {
				_buttonState = State::Normal;
			}
		}
		UpdateColor();
	}
};
