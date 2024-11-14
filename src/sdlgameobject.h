#pragma once
#include <SDL.h>
#include <vector>
#include "rect.h"
#include "sdlengine.h"

class SdlGameObject {
public:
private:
	std::vector<std::shared_ptr<SdlGameObject>> _children;
	std::shared_ptr<SdlGameObject> _parent;
	std::vector<std::shared_ptr<SdlUpdatable>>  _update;
public:
	std::vector<std::shared_ptr<SdlGameObject>>& GetChildren() {
		return _children;
	}
};
