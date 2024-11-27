#pragma once
#include "rect.h"
#include "vyengine.h"
#include "sdlhelper.h"
#include "selectablerect.h"
#include <functional>
#include "helper.h"

class Button : public VyInterface, public SelectableRect, public VyDrawable, public VyUpdatable {
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
	VyEngine::TriggeredEvent onPress;
	VyEngine::TriggeredEvent onRelease;
private:
	Button::State _buttonState;
	int color;
	bool held;
public:
	Colors Colors;

	Button() : Button({ 0, 0, 10, 10 }) {}

	Button(SDL_Rect rect) : SelectableRect(rect), _buttonState(State::Normal), Colors(), held(false), color(0) {
		onPress = Nothing;
		onRelease = Nothing;
		Register();
	}

	~Button() {
		Unregister();
	}

	virtual VyEventProcessor* AsEventProcessor() { return this; }
	virtual VyDrawable* AsDrawable() { return this; }
	virtual VyUpdatable* AsUpdatable() { return this; }

	void Register() {
		VyEngine* sdl = VyEngine::GetInstance();
		sdl->RegisterMouseDown(SDL_MOUSE_MAINCLICK, (size_t)this, [&](SDL_Event e) { HandleEvent(e); });
		sdl->RegisterMouseUp(SDL_MOUSE_MAINCLICK, (size_t)this, [&](SDL_Event e) { HandleEvent(e); });
		sdl->RegisterDrawable(this);
		sdl->RegisterUpdatable(this);
	}

	void Unregister() {
		VyEngine* sdl = VyEngine::GetInstance();
		sdl->UnregisterMouseDown(SDL_MOUSE_MAINCLICK, (size_t)this);
		sdl->UnregisterMouseUp(SDL_MOUSE_MAINCLICK, (size_t)this);
		sdl->UnregisterDrawable(this);
		sdl->UnregisterUpdatable(this);
	}

	virtual void HandleEvent(const SDL_Event& e) {
		switch (e.button.state) {
		case SDL_PRESSED: {
			bool mouseOver = IsContains(VyEngine::GetInstance()->MousePosition);
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

	virtual void Draw(SDL_Renderer* g) {
		if (!_active) {
			// TODO when deactivated, remove it from the list instead.
			return;
		}
		long oldColor;
		SDL_GetRenderDrawColor(g, &oldColor);
		UpdateColor();
		SDL_SetRenderDrawColor(g, color);
		RenderFillRect(g);
		SDL_SetRenderDrawColor(g, 0xff00ff00);
		DrawNavigation(g);
		SDL_SetRenderDrawColor(g, oldColor);
	}

	virtual void Update() {
		if (!_active) {
			// TODO when deactivated, remove it from the list instead.
			return;
		}
		VyEngine* engine = VyEngine::GetInstance();
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

	// TODO rename to something... like ConstantName.. or something. I dunno. I am sleepy.
	const std::string _NAME = std::string(nameof(SdlButton));

	virtual const std::string& GetName() const { return _NAME; }
	virtual void SetName(std::string name) {  }
};
