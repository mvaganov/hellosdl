#pragma once
#include "rect.h"
#include "vyengine.h"
#include "sdlhelper.h"
#include "sdleventprocessor.h"
#include <functional>
#include <algorithm>

// TODO register self with VyEngine, and mark as needing navigation recalculation
class SelectableRect : public Rect, public VyEventProcessor {
protected:
	bool _selected;
	/// <summary>
	/// if false, don't navigate to this element
	/// </summary>
	bool _navigatable;
	bool _active;
	SelectableRect* _next[(int)Rect::Dir::Count];
public:
	VyEngine::EventDelegateKeyedList OnKeyEvent;
	VyEngine::EventKeyedList OnSelected;
	VyEngine::EventKeyedList OnUnselected;

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

	SelectableRect(SDL_Rect rect) : Rect(rect), _selected(false), _navigatable(true), _active(true), _next() {
		memset(_next, NULL, sizeof(_next));
		OnKeyEvent[(size_t)this] = [this](SDL_Event e) { HandleNavigationKey(e); };
		VyEngine::GetInstance()->RegisterProcessor(this);
	}

	virtual void ProcessInput(const SDL_Event& e) {
		switch (e.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			VyEngine::ProcessDelegates(OnKeyEvent, e);
			break;
		}
	}

	void NotifySelect() {
		VyEngine::ProcessDelegates(OnSelected);
	}

	void NotifyUnselect() {
		VyEngine::ProcessDelegates(OnUnselected);
	}

	void HandleNavigationKey(SDL_Event keyPress) {
		if (!_active || !_selected || keyPress.type != SDL_KEYDOWN) {
			return;
		}
		//printf("SelectableRect::HandleNavigationKey\n");
		switch (keyPress.key.keysym.sym) {
		case SDLK_UP:   Navigate(Rect::Dir::Up);   break;
		case SDLK_LEFT: Navigate(Rect::Dir::Left); break;
		case SDLK_DOWN: Navigate(Rect::Dir::Down); break;
		case SDLK_RIGHT:Navigate(Rect::Dir::Right);break;
		}
	}

	void Navigate(Rect::Dir dir) {
		//printf("navigate %d\n", dir);
		SelectableRect* next = _next[(int)dir];
		if (next == NULL) {
			return;
		}
		VyEngine::GetInstance()->Queue([this,next]() {
			this->SetSelected(false);
			next->SetSelected(true);
		}, string_format(__FILE__ ":%d", __LINE__));
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
			[](Coord delta) -> bool { return delta.y < 0 && abs(delta.x) <= abs(delta.y); }, // up
			[](Coord delta) -> bool { return delta.x < 0 && abs(delta.x) >= abs(delta.y); }, // left
			[](Coord delta) -> bool { return delta.y > 0 && abs(delta.x) <= abs(delta.y); }, // down
			[](Coord delta) -> bool { return delta.x > 0 && abs(delta.x) >= abs(delta.y); }, // right
		};

		for (int dir = 0; dir < (int)Rect::Dir::Count; ++dir) {
			for (int selfId = 0; selfId < centers.size(); ++selfId) {
				int bestCandidate = GetBestNavigationCandidate(rects, centers, selfId, CandidateVetting[dir]);
				if (bestCandidate >= 0) {
					rects[selfId]->SetNavigation((Rect::Dir)dir, rects[bestCandidate]);
				}
			}
		}
	}

	static int GetBestNavigationCandidate(const std::vector<SelectableRect*>& rects, const std::vector<Coord>& centers, int selfId, std::function<bool(Coord delta)> initialCandidateVettingFunction) {
		Coord selfCenter = centers[selfId];
		int bestCandidate = -1;
		float bestCandidateDistance = 0;
		float distance;
		for (int candidateId = 0; candidateId < centers.size(); ++candidateId) {
			if (selfId == candidateId || !rects[candidateId]->_navigatable) {
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
