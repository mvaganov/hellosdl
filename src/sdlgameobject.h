#pragma once
#include <SDL.h>
#include <vector>
#include "rect.h"
#include "vyengine.h"
#include "vyobjectcommonbase.h"
#include "componentcontainer.h"
#include "sdlhierarchied.h"

class SdlGameObject : public VyInterface, public VyComponentContainerInterface, public VyHierarchedInterface {
public:
private:
	VyComponentContainer _container;
	VyHierarched _hierarchy;
public:
	SdlGameObject(std::string name) : _hierarchy(name) {}
	virtual const std::string& GetName() const { return _hierarchy.GetName(); }
	virtual void SetName(std::string name) { _hierarchy.SetName(name); }
	virtual int GetChildCount() const { return _hierarchy.GetChildCount(); }
	virtual std::shared_ptr <VyHierarchedInterface> GetChild(int index) { return _hierarchy.GetChild(index); }
	virtual std::shared_ptr<VyHierarchedInterface> GetParent() const { return _hierarchy.GetParent(); }
	virtual void SetParent(std::shared_ptr<VyHierarchedInterface> parent) { _hierarchy.SetParent(parent); }
	virtual void Update() {
		_container.Update();
		_hierarchy.Update();
	}
	virtual void Draw(SDL_Renderer* g) {
		_container.Draw(g);
		_hierarchy.Draw(g);
	}
	virtual void HandleEvent(const SDL_Event& e) {
		_container.HandleEvent(e);
		_hierarchy.HandleEvent(e);
	}
	void AddComponent(std::shared_ptr<VyInterface> ptr) { _container.AddComponent(ptr); }
	virtual int GetUpdateCount() const { return _container.GetUpdateCount(); }
	virtual int GetDrawCount() const { return _container.GetDrawCount(); }
	virtual VyEventProcessor* AsEventProcessor() { return this; }
	virtual VyDrawable* AsDrawable() { return this; }
	virtual SdlUpdatable* AsUpdatable() { return this; }
};
