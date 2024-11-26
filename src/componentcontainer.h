#pragma once
#include <vector>
#include <memory>
#include <functional>
#include "vyobjectcommonbase.h"

class VyComponentContainerInterface : public SdlUpdatable, public VyDrawable, public VyEventProcessor {
public:
	virtual void Update() = 0;
	virtual void Draw(SDL_Renderer* g) = 0;
	virtual void AddComponent(std::shared_ptr<VyInterface> ptr) = 0;
	virtual int GetUpdateCount() const = 0;
	virtual int GetDrawCount() const = 0;
};

//template<typename Type>
//class ComponentContainer {
//private:
//	std::vector<std::weak_ptr<Type>> _list;
//public:
//	int GetComponentCount() const { return (int)_list.size(); }
//	std::weak_ptr<Type> Get(int index) { return _list[index]; }
//	void ForEach(std::function<void(std::weak_ptr<Type>)> action) {
//		for (auto it = _list.begin(); it != _list.end(); it++) {
//			action(*it);
//		}
//	}
//	bool Add(std::weak_ptr<Type> ptr) {
//#if _DEBUG
//		for (auto it = _list.begin(); it != _list.end(); it++) {
//			if (*it == ptr) {
//				printf("duplicate component being added");
//				return false;
//			}
//		}
//#endif
//		_list.push_back(std::weak_ptr<Type>(shared_ptr<Type>(item));
//		return true;
//	}
//	void Clear() { _list.clear(); }
//	// TODO remove, get(index), get(type), getAll(type)
//};

class VyComponentContainer : public VyComponentContainerInterface {
private:
	//ComponentContainer<SdlUpdatable> _update;
	//ComponentContainer<VyDrawable> _drawable;
	std::vector<SdlUpdatable*> _updatable;
	std::vector<VyDrawable*> _drawable;
	std::vector<VyEventProcessor*> _eventProcessors;
	std::vector<std::shared_ptr<VyInterface>> _list;
public:
	VyComponentContainer() {}
	~VyComponentContainer() {
		_updatable.clear();
		_drawable.clear();
		_eventProcessors.clear();
	}
	void Update() {
		for (auto i : _updatable) {
			i->Update();
		}
	}
	void Draw(SDL_Renderer* g) {
		for (auto i : _drawable) {
			i->Draw(g);
		}
	}
	void HandleEvent(const SDL_Event& e) {
		for (auto i : _eventProcessors) {
			i->HandleEvent(e);
		}
	}
	void AddComponent(std::shared_ptr<VyInterface> ptr) {
		_list.push_back(ptr);
		SdlUpdatable* updatable = ptr->AsUpdatable();
		if (updatable) {
			_updatable.push_back(updatable);
		}
		VyDrawable* drawable = ptr->AsDrawable();
		if (drawable) {
			_drawable.push_back(drawable);
		}
		VyEventProcessor* eventable = ptr->AsEventProcessor();
		if (eventable) {
			_eventProcessors.push_back(eventable);
		}
	}
	virtual int GetUpdateCount() const { return (int)_updatable.size(); }
	virtual int GetDrawCount() const { return (int)_drawable.size(); }
};
