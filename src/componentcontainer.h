#pragma once
#include <vector>
#include <memory>
#include <functional>
#include "sdlobject.h"

class SdlComponentContainerInterface : public SdlUpdatable, public SdlDrawable {
public:
	virtual void Update() = 0;
	virtual void Draw(SDL_Renderer* g) = 0;
	virtual void AddComponent(std::shared_ptr<SdlNamed> ptr) = 0;
	virtual int GetUpdateCount() const = 0;
	virtual int GetDrawCount() const = 0;
};

template<typename Type>
class ComponentContainer {
private:
	std::vector<std::shared_ptr<Type>> _list;
public:
	int GetComponentCount() const { return (int)_list.size(); }
	std::shared_ptr<Type> Get(int index) { return _list[index]; }
	void ForEach(std::function<void(std::shared_ptr<Type>)> action) {
		const int size = (int)_list.size();
		for (int i = 0; i < size; ++i) {
			action(_list[i]);
		}
	}
	bool Add(std::shared_ptr<SdlNamed> sharedPtr) {
		Type* item = dynamic_cast<Type*>(sharedPtr.get());
		if (item) {
			_list.push_back(std::shared_ptr<Type>(item));
			return true;
		}
		return false;
	}
	void Clear() { _list.clear(); }
	// TODO remove, get(index), get(type), getAll(type)
};

class SdlComponentContainer : public SdlComponentContainerInterface {
private:
	ComponentContainer<SdlUpdatable> _update;
	ComponentContainer<SdlDrawable> _drawable;
public:
	SdlComponentContainer() {}
	~SdlComponentContainer() {
		_update.Clear();
		_drawable.Clear();
	}
	void Update() {
		_update.ForEach([](std::shared_ptr<SdlUpdatable> ptr) {
			ptr.get()->Update();
		});
	}
	void Draw(SDL_Renderer* g) {
		_drawable.ForEach([g](std::shared_ptr<SdlDrawable> ptr) {
			ptr.get()->Draw(g);
		});
	}
	void AddComponent(std::shared_ptr<SdlNamed> ptr) {
		_update.Add(ptr);
		_drawable.Add(ptr);
	}
	virtual int GetUpdateCount() const { return _update.GetComponentCount(); }
	virtual int GetDrawCount() const { return _drawable.GetComponentCount(); }
};
