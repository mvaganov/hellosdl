#pragma once
#include <vector>
#include "sdlobject.h"
#include "componentcontainer.h"

class SdlHierarchedInterface {
public:
	virtual int GetChildCount() const = 0;
	virtual std::shared_ptr <SdlHierarchedInterface> GetChild(int index) = 0;
	virtual std::shared_ptr<SdlHierarchedInterface> GetParent() const = 0;
	virtual void SetParent(std::shared_ptr<SdlHierarchedInterface> parent) = 0;
};

class SdlHierarched : public SdlObject, public SdlComponentContainer {
public:
private:
	std::vector<std::shared_ptr<SdlHierarchedInterface>> _children;
	std::shared_ptr<SdlHierarchedInterface> _parent;
public:
	SdlHierarched(std::string name) : SdlObject(name), _children() {}
	virtual int GetChildCount() const { return (int)_children.size(); }
	virtual std::shared_ptr <SdlHierarchedInterface> GetChild(int index) { return _children[index]; }
	virtual std::shared_ptr<SdlHierarchedInterface> GetParent() const { return _parent; }
	virtual void SetParent(std::shared_ptr<SdlHierarchedInterface> parent) { _parent = parent; }
};
