#pragma once
#include "rect.h"
#include "sdlengine.h"
#include "sdlhelper.h"
#include <functional>
#include <algorithm>

// TODO register self with SdlEngine, and mark as needing navigation recalculation
// TODO navigatable flag, which allows this to be skipped for navigation calculations
class SelectableRect : public Rect {
protected:
	bool _selected;
	SelectableRect* _next[(int)Rect::Dir::Count];
public:
	SdlEngine::EventDelegateKeyedList OnKeyEvent;
	SdlEngine::EventKeyedList OnSelected;
	SdlEngine::EventKeyedList OnUnselected;

	bool IsSelected() {
		return _selected;
	}

	void SetSelectedNoNotify(bool selected) {
		_selected = selected;
	}

	void SetSelected(bool selected) {
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
		OnKeyEvent[(size_t)this] = [this](SDL_Event e) { HandleNavigationKey(e); };
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
		SdlEngine::ProcessDelegates(OnUnselected);
	}

	void HandleNavigationKey(SDL_Event keyPress) {
		if (keyPress.type != SDL_KEYDOWN) {
			return;
		}
		switch (keyPress.key.keysym.sym) {
		case SDLK_UP: Navigate(Rect::Dir::Up); break;
		case SDLK_LEFT: Navigate(Rect::Dir::Left); break;
		case SDLK_DOWN: Navigate(Rect::Dir::Down); break;
		case SDLK_RIGHT: Navigate(Rect::Dir::Right); break;
		}
	}

	void Navigate(Rect::Dir dir) {
		printf("navigate %d\n", dir);
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
		std::vector<Coord> centers;
		centers.reserve(rects.size());
		for (int i = 0; i < rects.size(); ++i) {
			rects[i]->ClearNavigation();
			Coord center = rects[i]->GetCenter();
			centers.push_back(center);
		}

		std::function<bool(Coord delta)> CandidateVetting[(int)Rect::Dir::Count] = {
			// up
			[](Coord delta) -> bool {
				return delta.y < 0 && abs(delta.x) <= abs(delta.y);
			},
			// left
			[](Coord delta) -> bool {
				return delta.x < 0 && abs(delta.x) >= abs(delta.y);
			},
			// down
			[](Coord delta) -> bool {
				return delta.y > 0 && abs(delta.x) <= abs(delta.y);
			},
			// right
			[](Coord delta) -> bool {
				return delta.x > 0 && abs(delta.x) >= abs(delta.y);
			},
		};

		for (int dir = 0; dir < (int)Rect::Dir::Count; ++dir) {
			for (int selfId = 0; selfId < centers.size(); ++selfId) {
				int bestCandidate = GetBestNavigationCandidate(centers, selfId, CandidateVetting[dir]);
				if (bestCandidate >= 0) {
					rects[selfId]->SetNavigation((Rect::Dir)dir, rects[bestCandidate]);
				}
			}
		}
	}

	static int GetBestNavigationCandidate(const std::vector<Coord>& centers, int selfId, std::function<bool(Coord delta)> initialCandidateVettingFunction) {
		Coord selfCenter = centers[selfId];
		int bestCandidate = -1;
		float bestCandidateDistance = 0;
		float distance;
		for (int candidateId = 0; candidateId < centers.size(); ++candidateId) {
			if (selfId == candidateId) {
				continue;
			}
			Coord delta = centers[candidateId]  - selfCenter;
			if (initialCandidateVettingFunction(delta)) {
				distance = delta.MagnitudeSq();
				if (bestCandidateDistance == 0 || distance < bestCandidateDistance) {
					bestCandidateDistance = distance;
					bestCandidate = candidateId;
				}
			}
		}
		return bestCandidate;
	}

	void DrawNavigation(SDL_Renderer* g) {
		Coord center = GetCenter();
		Coord other;
		for (int i = 0; i < (int)Rect::Dir::Count; ++i) {
			SelectableRect* next = _next[i];
			if (next == NULL) {
				continue;
			}
			other = next->GetCenter();
			Coord delta = other - center;
			other = center + delta / 2;
			int c = 0xff000000 | Rect::DirColor[i];
			SDL_SetRenderDrawColor(g, c);
			SDL_RenderDrawLine(g, center.x, center.y, other.x, other.y);
		}
	}

};
