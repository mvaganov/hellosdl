#pragma once
#include <SDL.h>
#include <vector>
#include "rect.h"
#include "sdlengine.h"
#include "sdlobject.h"
#include "componentcontainer.h"
#include "sdlhierarchied.h"

class SdlGameObject : public SdlNamed, public SdlComponentContainerInterface, public SdlHierarchedInterface {
public:
private:
	SdlComponentContainer _container;
	SdlHierarched _hierarchy;
public:
	SdlGameObject(std::string name) : _hierarchy(name) {}
	virtual const std::string& GetName() const { return _hierarchy.GetName(); }
	virtual void SetName(std::string name) { _hierarchy.SetName(name); }
	virtual int GetChildCount() const { return _hierarchy.GetChildCount(); }
	virtual std::shared_ptr <SdlHierarchedInterface> GetChild(int index) { return _hierarchy.GetChild(index); }
	virtual std::shared_ptr<SdlHierarchedInterface> GetParent() const { return _hierarchy.GetParent(); }
	virtual void SetParent(std::shared_ptr<SdlHierarchedInterface> parent) { _hierarchy.SetParent(parent); }
	virtual void Update() { _container.Update(); }
	virtual void Draw(SDL_Renderer* g) { _container.Draw(g); }
	void AddComponent(std::shared_ptr<SdlNamed> ptr) { _container.AddComponent(ptr); }
	virtual int GetUpdateCount() const { return _container.GetUpdateCount(); }
	virtual int GetDrawCount() const { return _container.GetDrawCount(); }
};
