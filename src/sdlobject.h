#pragma once
#include "sdleventprocessor.h"
#include <string>

class SdlObject : public SdlNamed {
private:
	std::string _name;
public:
	SdlObject(std::string name) : _name(name) {}
	virtual const std::string& GetName() const { return _name; }
	virtual void SetName(std::string name) { _name = name; }
};
