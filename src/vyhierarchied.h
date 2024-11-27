#pragma once
#include <vector>
#include "vyobjectcommonbase.h"
#include "componentcontainer.h"

class VyHierarchedInterface {
public:
	virtual int GetChildCount() const = 0;
	virtual std::shared_ptr <VyHierarchedInterface> GetChild(int index) = 0;
	virtual std::shared_ptr<VyHierarchedInterface> GetParent() const = 0;
	virtual void SetParent(std::shared_ptr<VyHierarchedInterface> parent) = 0;
};

class VyHierarched : public VyObjectCommonBase, public VyComponentContainer {
public:
private:
	std::vector<std::shared_ptr<VyHierarchedInterface>> _children;
	std::shared_ptr<VyHierarchedInterface> _parent;
public:
	VyHierarched(std::string name) : VyObjectCommonBase(name), _children() {}
	virtual int GetChildCount() const { return (int)_children.size(); }
	virtual std::shared_ptr <VyHierarchedInterface> GetChild(int index) { return _children[index]; }
	virtual std::shared_ptr<VyHierarchedInterface> GetParent() const { return _parent; }
	virtual void SetParent(std::shared_ptr<VyHierarchedInterface> parent) { _parent = parent; }
	virtual VyEventProcessor* AsEventProcessor() { return this; }
	virtual VyDrawable* AsDrawable() { return this; }
	virtual VyUpdatable* AsUpdatable() { return this; }
};
