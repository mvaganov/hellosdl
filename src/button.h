#pragma once
#include "rect.h"
#include "sdlengine.h"
#include "sdlhelper.h"

class Button : public Rect {
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
			: normal(normal), hover(hover), clicked(clicked), selected(selected)
		{}
		Colors() : Colors(0xff888888, 0xffbbbbbb, 0x88888888, 0xffaaaaaa) {}
	};
	Button::State state;
	Colors colors;

	void Draw(SDL_Renderer* g) {
		long oldColor;
		SDL_GetRenderDrawColor(g, &oldColor);
		long color = colors.normal;
		switch (state) {
		case State::Normal: color = colors.normal; break;
		case State::Hovered: color = colors.hover; break;
		case State::Clicked: color = colors.clicked; break;
		case State::Selected: color = colors.selected; break;
		case State::HoveredSelected: color = colors.hoveredSelected; break;
		}
		SDL_SetRenderDrawColor(g, color);
		RenderFillRect(g);
		SDL_SetRenderDrawColor(g, oldColor);
	}

	void Update(System engine) {
		if (state == State::Normal && IsContains(engine.MousePosition)) {
			state = State::Hovered;
		}
		bool isPressed;
		if (state == State::Hovered
		&& engine.IsPressed(SDL_MOUSEMOTION, isPressed) == System::ErrorCode::Success
		&& isPressed) {
			state = State::Selected;
		}
		switch (state) {
		case State::Normal: 
			
			break;
		case State::Hovered: color = colors.hover; break;
		case State::Clicked: color = colors.clicked; break;
		case State::Selected: color = colors.selected; break;
		}

		engine.MousePosition
	}
};