#pragma once
#include "rect.h"
#include "sdlengine.h"
#include "sdlhelper.h"
#include <functional>
#include <algorithm>

class SelectableRect : public Rect {
protected:
	bool _selected;
	SelectableRect* _next[4];
public:
	SdlEngine::EventDelegateKeyedList OnKeyEvent;
	SdlEngine::EventKeyedList OnSelected;
	SdlEngine::EventKeyedList OnUnselected;

	bool IsSelected() {
		return _selected;
	}

	bool SetSelectedNoNotify(bool selected) {
		_selected = selected;
	}

	bool SetSelected(bool selected) {
		if (selected != _selected) {
			if (selected) {
				NotifySelect();
			} else {
				NotifyUnselect();
			}
		}
		_selected = selected;
	}

	SelectableRect(SDL_Rect rect) : Rect(rect), _selected(false), _next() {
		memset(_next, NULL, sizeof(_next));
	}

	void ProcessInput(const SDL_Event& e) {
		switch (e.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			SdlEngine::ProcessDelegates(OnKeyEvent, e);
			break;
		}
	}

	void NotifySelect() {
		SdlEngine::ProcessDelegates(OnSelected);
	}

	void NotifyUnselect() {
		SdlEngine::ProcessDelegates(OnSelected);
	}

	void Navigate(Rect::Dir dir) {
		SelectableRect* next = _next[(int)dir];
		if (next == NULL) {
			return;
		}
		SetSelected(false);
		next->SetSelected(true);
	}

	void SetNavigation(Rect::Dir dir, SelectableRect* nextRect) {
		_next[(int)dir] = nextRect;
	}

	void ClearNavigation() {
		for (int i = 0; i < (int)Rect::Dir::Count; ++i) {
			_next[i] = NULL;
		}
	}

	struct CenterRect {
		int pos;
		int rect;
	};

	static void SetupNavigation(const std::vector<SelectableRect*>& rects) {
		std::vector<CenterRect> horizontal, vertical;
		std::vector<Coord> centers;
		horizontal.reserve(rects.size());
		vertical.reserve(rects.size());
		centers.reserve(rects.size());
		for (int i = 0; i < rects.size(); ++i) {
			rects[i]->ClearNavigation();
			Coord center = rects[i]->GetCenter();
			centers.push_back(center);
			horizontal.push_back({ center.x, i });
			vertical.push_back({ center.y, i });
		}
		std::sort(horizontal.begin(), horizontal.end(), [](CenterRect a, CenterRect b) { return a.pos < b.pos; });
		std::sort(vertical.begin(), vertical.end(), [](CenterRect a, CenterRect b) { return a.pos < b.pos; });

		for (int i = 0; i < horizontal.size(); ++i) {
			Coord c = centers[horizontal[i].rect];
			printf("%d, %d  ", c.x, c.y);
		}
		putchar('\n');
		for (int i = 0; i < vertical.size(); ++i) {
			Coord c = centers[vertical[i].rect];
			printf("%d, %d  ", c.x, c.y);
		}
		putchar('\n');

		std::function<float(int self, int other)> goingThisWay[4] = {
			// up
			[&centers](int self, int other) -> float {
				Coord delta = centers[other] - centers[self];
				if (delta.y < 0 && abs(delta.y) >= abs(delta.x)) {
					return delta.Magnitude();
				}
				return -1;
			},
			// left
			[&centers](int self, int other) -> float {
				Coord delta = centers[other] - centers[self];
				if (delta.x < 0 && abs(delta.x) >= abs(delta.y)) {
					return delta.Magnitude();
				}
				return -1;
			},
			// down
			[&centers](int self, int other) -> float {
				Coord delta = centers[other] - centers[self];
				if (delta.y > 0 && abs(delta.y) >= abs(delta.x)) {
					return delta.Magnitude();
				}
				return -1;
			},
			// right
			[&centers](int self, int other) -> float {
				Coord delta = centers[other] - centers[self];
				if (delta.x > 0 && abs(delta.x) >= abs(delta.y)) {
					return delta.Magnitude();
				}
				return -1;
			},
		};
		int dir;
		// go right
		// TODO make the rest like this
		dir = (int)Rect::Dir::Right;
		for (int i = 0; i < (int)horizontal.size(); ++i) {
			int self = horizontal[i].rect;
			float bestDist = 0, dist;
			int best = -1;
			for (int o = i + 1; o < horizontal.size(); ++o) {
				// TODO make this a function?
				int other = horizontal[o].rect;
				dist = goingThisWay[dir](self, other);
				if (dist > 0 && (best < 0 || dist < bestDist)) {
					bestDist = dist;
					best = other;
					if (o < horizontal.size() - 1) {
						int next = horizontal[o + 1].rect;
						if (rects[next]->y != rects[other]->y) {
							break;
						}
					}
				}
			}
			if (best >= 0) {
				rects[self]->SetNavigation((Rect::Dir)dir, rects[best]);
			}
		}
		// go left
		dir = (int)Rect::Dir::Left;
		for (int i = (int)horizontal.size() - 1; i >= 0; --i) {
			int self = horizontal[i].rect;
			for (int o = i - 1; o >= 0; --o) {
				int other = horizontal[o].rect;
				if (goingThisWay[dir](self, other)) {
					rects[self]->SetNavigation((Rect::Dir)dir, rects[other]);
					break;
				}
			}
		}
		// go down
		dir = (int)Rect::Dir::Down;
		for (int i = 0; i < (int)vertical.size(); ++i) {
			int self = vertical[i].rect;
			for (int o = i + 1; o < vertical.size(); ++o) {
				int other = vertical[o].rect;
				if (goingThisWay[dir](self, other)) {
					rects[self]->SetNavigation((Rect::Dir)dir, rects[other]);
					break;
				}
			}
		}
		// go up
		dir = (int)Rect::Dir::Up;
		for (int i = (int)vertical.size() - 1; i >= 0; --i) {
			int self = vertical[i].rect;
			for (int o = i - 1; o >= 0; --o) {
				int other = vertical[o].rect;
				if (goingThisWay[dir](self, other)) {
					rects[self]->SetNavigation((Rect::Dir)dir, rects[other]);
					break;
				}
			}
		}
	}
};
