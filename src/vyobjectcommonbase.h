#pragma once
#include "sdleventprocessor.h"
#include <string>

class VyObjectCommonBase : public VyInterface {
private:
	std::string _name;
public:
	VyObjectCommonBase(std::string name) : _name(name) {}
	virtual const std::string& GetName() const { return _name; }
	virtual void SetName(std::string name) { _name = name; }
	virtual VyEventProcessor* AsEventProcessor() { return nullptr; }
	virtual VyDrawable* AsDrawable() { return nullptr; }
	virtual VyUpdatable* AsUpdatable() { return nullptr; }
};
